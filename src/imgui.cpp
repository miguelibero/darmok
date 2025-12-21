#include "detail/imgui.hpp"
#include "detail/asset.hpp"
#include "detail/input.hpp"
#include "detail/platform.hpp"
#include <darmok/imgui.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/mesh.hpp>
#include <darmok/math.hpp>
#include <darmok/texture.hpp>
#include <darmok/program_core.hpp>
#include <bx/allocator.h>
#include "generated/shaders/imgui/imgui.program.h"

namespace darmok
{
	struct ImguiUtils
	{
		static glm::vec2 convert(const ImVec2& v) noexcept
		{
			return { v.x, v.y };
		}

		static ImVec2 convert(const glm::vec2& v) noexcept
		{
			return { v.x, v.y };
		}

		static glm::vec4 convert(const ImVec4& v) noexcept
		{
			return { v.x, v.y, v.z, v.w };
		}

		static ImVec4 convert(const glm::vec4& v) noexcept
		{
			return { v.x, v.y, v.z, v.w };
		}

		static uint16_t convertUint16(float v) noexcept
		{
			v = bx::max(v, 0.0f);
			v = bx::min(v, 65535.0f);
			return static_cast<uint16_t>(v);
		}
	};

	ImguiRenderPass::ImguiRenderPass(IImguiRenderer& renderer, ImGuiContext* imgui) noexcept
		: _renderer{ renderer }
		, _imgui{ imgui }
		, _lodEnabledUniform{ "u_imageLodEnabled", bgfx::UniformType::Vec4 }
		, _textureUniform{ "s_texColor", bgfx::UniformType::Sampler }
	{
	}

	expected<void, std::string> ImguiRenderPass::init() noexcept
	{
		auto fontsResult = updateFonts();
		if (!fontsResult)
		{
			return unexpected{ std::move(fontsResult).error() };
		}
		auto progResult = Program::loadStaticMem(imgui_program);
		if (!progResult)
		{
			return unexpected{ std::move(progResult).error() };
		}
		_program = std::make_unique<Program>(std::move(progResult).value());
		return {};
	}

	expected<void, std::string> ImguiRenderPass::updateFonts() noexcept
	{
		uint8_t* data;
		int width, height, bytesPerPixel = 0;
		ImGuiIO& iio = ImGui::GetIO();
		iio.Fonts->GetTexDataAsRGBA32(&data, &width, &height, &bytesPerPixel);

		Texture::Config config;
		config.set_format(Texture::Definition::BGRA8);
		config.set_type(Texture::Definition::Texture2D);
		*config.mutable_size() = convert<protobuf::Uvec2>(glm::uvec2{ width, height });
		DataView dataView{ data, static_cast<size_t>(width * height * bytesPerPixel) };

		auto texResult = Texture::load(dataView, config);
		if(!texResult)
		{
			return unexpected{ std::move(texResult).error() };
		}
		_fontsTexture = std::make_unique<Texture>(std::move(texResult).value());
		return {};
	}

	bgfx::ViewId ImguiRenderPass::renderReset(bgfx::ViewId viewId) noexcept
	{
		bgfx::setViewName(viewId, "Imgui");
		bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
		_viewId = viewId;
		return ++viewId;
	}

	expected<void, std::string> ImguiRenderPass::render() noexcept
	{
		ImGui::SetCurrentContext(_imgui);
		beginFrame();
		auto result = _renderer.imguiRender();
		if (!result)
		{
			return result;
		}
		auto encoder = bgfx::begin();
		auto frameResult = endFrame(*encoder);
		bgfx::end(encoder);
		ImGui::SetCurrentContext(nullptr);
		return frameResult;
	}

	void ImguiRenderPass::beginFrame() const noexcept
	{
		ImGui::NewFrame();
	}

	expected<void, std::string> ImguiRenderPass::endFrame(bgfx::Encoder& encoder) const noexcept
	{
		ImGui::Render();
		return render(encoder, ImGui::GetDrawData());
	}

	ImguiTextureData::ImguiTextureData(const bgfx::TextureHandle& handle) noexcept
		: handle{ handle }
		, alphaBlend{ false }
		, mip{ 0 }
	{
	}

	ImguiTextureData::ImguiTextureData(const ImTextureID& id) noexcept
	{
		union { ImTextureID id; ImguiTextureData data; } v = { id };
		*this = v.data;
	}

	ImguiTextureData::operator ImTextureID() const noexcept
	{
		union { ImguiTextureData data; ImTextureID id; } v = { *this };
		return v.id;
	}
	
	expected<void, std::string> ImguiRenderPass::render(bgfx::Encoder& encoder, ImDrawData* drawData) const noexcept
	{
		if (!_viewId)
		{
			return unexpected<std::string>{"missing view id"};
		}
		auto clipPos = ImguiUtils::convert(drawData->DisplayPos); // (0,0) unless using multi-viewports
		auto size = ImguiUtils::convert(drawData->DisplaySize);
		auto clipScale = ImguiUtils::convert(drawData->FramebufferScale);  // (1,1) unless using retina display which are often (2,2)
		auto clipSize = size / clipScale;

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		if (clipSize.x <= 0 || clipSize.y <= 0)
		{
			return {};
		}

		auto viewId = _viewId.value();
		{
			auto ortho = Math::ortho(clipPos.x, clipPos.x + clipSize.x, clipPos.y + clipSize.y, clipPos.y);
			bgfx::setViewTransform(viewId, nullptr, glm::value_ptr(ortho));
			bgfx::setViewRect(viewId, 0, 0,
				ImguiUtils::convertUint16(clipSize.x),
				ImguiUtils::convertUint16(clipSize.y)
			);
		}

		auto& layout = _program->getVertexLayout();
		std::vector<std::string> errors;

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			const ImDrawList* drawList = drawData->CmdLists[ii];

			auto numVertices = static_cast<uint32_t>(drawList->VtxBuffer.size());
			auto numIndices = static_cast<uint32_t>(drawList->IdxBuffer.size());

			const DataView vertData{ &drawList->VtxBuffer.front(), numVertices * sizeof(ImDrawVert) };
			const DataView idxData{ &drawList->IdxBuffer.front(), numIndices * sizeof(ImDrawIdx) };
			MeshConfig config{ .type = Mesh::Definition::Transient };
			auto meshResult = Mesh::load(layout, vertData, idxData, config);
			if(!meshResult)
			{
				errors.push_back(std::move(meshResult).error());
				break;
			}
			auto mesh = std::move(meshResult).value();

			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_MSAA
						;

					auto tex = _fontsTexture->getHandle();
					auto program = _program->getHandle();

					if (cmd->TextureId != 0)
					{
						const ImguiTextureData texData{ cmd->TextureId };
						state |= 0 != (texData.alphaBlend)
							? BGFX_STATE_BLEND_ALPHA
							: BGFX_STATE_NONE
							;
						tex = texData.handle;
						auto mipEnabled = 0 != texData.mip;
						if (mipEnabled)
						{
							program = _program->getHandle({ "LOD_ENABLED" });
							const float lodEnabled[4] = { float(texData.mip), mipEnabled ? 1.0F : 0.0F, 0.0F, 0.0F };
							bgfx::setUniform(_lodEnabledUniform, lodEnabled);
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_ALPHA;
					}

					auto clipRect = ImguiUtils::convert(cmd->ClipRect);
					if (clipRect.x < clipSize.x
						&& clipRect.y < clipSize.y
						&& clipRect.z >= 0.0f
						&& clipRect.w >= 0.0f)
					{
						encoder.setScissor(
							ImguiUtils::convertUint16(clipRect.x),
							ImguiUtils::convertUint16(clipRect.y),
							ImguiUtils::convertUint16(clipRect.z - clipRect.x),
							ImguiUtils::convertUint16(clipRect.w - clipRect.y)
						);

						encoder.setState(state);
						encoder.setTexture(0, _textureUniform, tex);
						const MeshRenderConfig renderConfig
						{
							.startVertex = cmd->VtxOffset,
							.startIndex = cmd->IdxOffset,
							.numIndices = cmd->ElemCount
						};

						auto renderResult = mesh.render(encoder, renderConfig);
						if (!renderResult)
						{
							errors.push_back(std::move(renderResult).error());
						}
						encoder.submit(viewId, program);
					}
				}
			}
		}

		return StringUtils::joinExpectedErrors(errors);
	}

	const ImguiAppComponentImpl::KeyboardMap& ImguiAppComponentImpl::getKeyboardMap() noexcept
	{
		static KeyboardMap map
		{
			{ Keyboard::Definition::KeyEsc, ImGuiKey_Escape},
			{ Keyboard::Definition::KeyReturn, ImGuiKey_Enter},
			{ Keyboard::Definition::KeyTab, ImGuiKey_Tab},
			{ Keyboard::Definition::KeySpace, ImGuiKey_Space},
			{ Keyboard::Definition::KeyBackspace, ImGuiKey_Backspace},
			{ Keyboard::Definition::KeyUp, ImGuiKey_UpArrow},
			{ Keyboard::Definition::KeyDown, ImGuiKey_DownArrow},
			{ Keyboard::Definition::KeyLeft, ImGuiKey_LeftArrow},
			{ Keyboard::Definition::KeyRight, ImGuiKey_RightArrow},
			{ Keyboard::Definition::KeyInsert, ImGuiKey_Insert},
			{ Keyboard::Definition::KeyDelete, ImGuiKey_Delete},
			{ Keyboard::Definition::KeyHome, ImGuiKey_Home},
			{ Keyboard::Definition::KeyEnd, ImGuiKey_End},
			{ Keyboard::Definition::KeyPageUp, ImGuiKey_PageUp},
			{ Keyboard::Definition::KeyPageDown, ImGuiKey_PageDown},
			{ Keyboard::Definition::KeyPrint, ImGuiKey_PrintScreen},
			{ Keyboard::Definition::KeyPlus, ImGuiKey_Equal},
			{ Keyboard::Definition::KeyMinus, ImGuiKey_Minus},
			{ Keyboard::Definition::KeyLeftBracket, ImGuiKey_LeftBracket},
			{ Keyboard::Definition::KeyRightBracket, ImGuiKey_RightBracket},
			{ Keyboard::Definition::KeySemicolon, ImGuiKey_Semicolon},
			{ Keyboard::Definition::KeyQuote, ImGuiKey_Apostrophe},
			{ Keyboard::Definition::KeyComma, ImGuiKey_Comma},
			{ Keyboard::Definition::KeyPeriod, ImGuiKey_Period},
			{ Keyboard::Definition::KeySlash, ImGuiKey_Slash},
			{ Keyboard::Definition::KeyBackslash, ImGuiKey_Backslash},
			{ Keyboard::Definition::KeyGraveAccent, ImGuiKey_GraveAccent},
		};
		static bool first = true;

		auto addRange = [](KeyboardKey start, KeyboardKey end, ImGuiKey imguiStart) {
			auto j = toUnderlying(imguiStart);
			for (auto i = toUnderlying(start); i < toUnderlying(end); i++)
			{
				map[(KeyboardKey)i] = (ImGuiKey)j++;
			}
		};

		if (first)
		{
			addRange(Keyboard::Definition::KeyF1, Keyboard::Definition::KeyF12, ImGuiKey_F1);
			addRange(Keyboard::Definition::KeyNumPad0, Keyboard::Definition::KeyNumPad9, ImGuiKey_Keypad9);
			addRange(Keyboard::Definition::Key0, Keyboard::Definition::Key9, ImGuiKey_0);
			addRange(Keyboard::Definition::KeyA, Keyboard::Definition::KeyZ, ImGuiKey_A);
			first = false;
		}
		return map;
	}

	const ImguiAppComponentImpl::GamepadMap& ImguiAppComponentImpl::getGamepadMap() noexcept
	{
		static const GamepadMap map
		{
			{ Gamepad::Definition::ButtonStart, ImGuiKey_GamepadStart },
			{ Gamepad::Definition::ButtonSelect, ImGuiKey_GamepadBack },
			{ Gamepad::Definition::ButtonY, ImGuiKey_GamepadFaceUp },
			{ Gamepad::Definition::ButtonA, ImGuiKey_GamepadFaceDown },
			{ Gamepad::Definition::ButtonX, ImGuiKey_GamepadFaceLeft },
			{ Gamepad::Definition::ButtonB, ImGuiKey_GamepadFaceRight },
			{ Gamepad::Definition::ButtonPadUp, ImGuiKey_GamepadDpadUp },
			{ Gamepad::Definition::ButtonPadDown, ImGuiKey_GamepadDpadDown },
			{ Gamepad::Definition::ButtonPadLeft, ImGuiKey_GamepadDpadLeft },
			{ Gamepad::Definition::ButtonPadRight, ImGuiKey_GamepadDpadRight },
			{ Gamepad::Definition::ButtonLeftBumper, ImGuiKey_GamepadL1 },
			{ Gamepad::Definition::ButtonRightBumper, ImGuiKey_GamepadR1 },
			{ Gamepad::Definition::ButtonLeftThumb, ImGuiKey_GamepadL3 },
			{ Gamepad::Definition::ButtonRightThumb, ImGuiKey_GamepadR3 },
			{ Gamepad::Definition::ButtonGuide, ImGuiKey_None },
		};

		return map;
	}

	void* ImguiAppComponentImpl::memAlloc(size_t size, void* userData) noexcept
	{
		if (userData == nullptr)
		{
			return std::malloc(size);
		}
		auto alloc = static_cast<bx::AllocatorI*>(userData);
		return bx::alloc(alloc, size);
	}

	void ImguiAppComponentImpl::memFree(void* ptr, void* userData) noexcept
	{
		if (userData == nullptr)
		{
			return std::free(ptr);
		}
		auto alloc = static_cast<bx::AllocatorI*>(userData);
		bx::free(alloc, ptr);
	}

	inline bool ImguiAppComponentImpl::checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices) noexcept
	{
		return numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, layout)
			&& (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices))
			;
	}	

	ImguiAppComponentImpl::ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize)
		: _renderer{ renderer }
		, _inputEnabled{ true }
		, _imgui{ nullptr }
	{
		IMGUI_CHECKVERSION();
	}

	ImguiAppComponentImpl::~ImguiAppComponentImpl() noexcept = default;

	expected<void, std::string> ImguiAppComponentImpl::init(App& app) noexcept
	{
		_app = app;
		ImGui::SetAllocatorFunctions(memAlloc, memFree, &app.getAssets().getAllocator());

		_imgui = ImGui::CreateContext();
		ImGui::SetCurrentContext(_imgui);

		auto& imio = ImGui::GetIO();

		imio.DeltaTime = 1.0f / 60.0f;
		imio.IniFilename = NULL;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&style);

		style.FrameRounding = 4.0f;
		style.WindowBorderSize = 0.0f;

		imio.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		imio.ConfigFlags |= 0
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_NavEnableKeyboard
			;

		auto result = _renderer.imguiSetup();
		if (!result)
		{
			return result;
		}
		_renderPass.emplace(_renderer, _imgui);
		result = _renderPass->init();
		if (!result)
		{
			return result;
		}

		ImGui::SetCurrentContext(nullptr);

		return result;
	}

	expected<bgfx::ViewId, std::string> ImguiAppComponentImpl::renderReset(bgfx::ViewId viewId) noexcept
	{
		if (_renderPass)
		{
			return _renderPass->renderReset(viewId);
		}
		return viewId;
	}

	expected<void, std::string> ImguiAppComponentImpl::render() noexcept
	{
		if (_renderPass)
		{
			return _renderPass->render();
		}
		return {};
	}

	expected<void, std::string> ImguiAppComponentImpl::shutdown() noexcept
	{
		_app.reset();
		_renderPass.reset();

		if (_imgui != nullptr)
		{
			ImGui::DestroyContext(_imgui);
			_imgui = nullptr;
		}
		return {};
	}

	expected<void, std::string> ImguiAppComponentImpl::update(float dt) noexcept
	{
		ImGui::SetCurrentContext(_imgui);
		updateInput(dt);
		ImGui::SetCurrentContext(nullptr);
		return {};
	}

	ImGuiContext* ImguiAppComponentImpl::getContext() noexcept
	{
		return _imgui;
	}

	bool ImguiAppComponentImpl::getInputEnabled() const noexcept
	{
		return _inputEnabled;
	}

	void ImguiAppComponentImpl::setInputEnabled(bool enabled) noexcept
	{
		_inputEnabled = enabled;
	}

	expected<void, std::string> ImguiAppComponentImpl::updateFonts() noexcept
	{
		if (_renderPass)
		{
			return _renderPass->updateFonts();
		}
		return {};
	}

	void ImguiAppComponentImpl::updateInput(float dt) noexcept
	{
		if (!_inputEnabled || !_app)
		{
			return;
		}
		auto& input = _app->getInput();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;

		for (auto& chr : input.getImpl().getKeyboard().getUpdateChars())
		{
			io.AddInputCharacter(chr);
		}

		auto& win = _app->getWindow();
		io.DisplaySize = ImguiUtils::convert(win.getPixelSize());
		io.DisplayFramebufferScale = ImguiUtils::convert(win.getFramebufferScale());

		auto& mouse = input.getMouse();
		auto pos = mouse.getPosition();
		io.AddMousePosEvent(pos.x, pos.y);

		io.AddMouseButtonEvent(ImGuiMouseButton_Left, mouse.getButton(Mouse::Definition::ButtonLeft));
		io.AddMouseButtonEvent(ImGuiMouseButton_Right, mouse.getButton(Mouse::Definition::ButtonRight));
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, mouse.getButton(Mouse::Definition::ButtonMiddle));
		
		auto& scroll = mouse.getScroll();
		io.AddMouseWheelEvent(scroll.x, scroll.y);

		auto& keyb = input.getKeyboard();
		io.AddKeyEvent(ImGuiMod_Shift, keyb.getModifier(Keyboard::Definition::ModifierShift));
		io.AddKeyEvent(ImGuiMod_Ctrl, keyb.getModifier(Keyboard::Definition::ModifierCtrl));
		io.AddKeyEvent(ImGuiMod_Alt, keyb.getModifier(Keyboard::Definition::ModifierAlt));
		io.AddKeyEvent(ImGuiMod_Super, keyb.getModifier(Keyboard::Definition::ModifierMeta));
		for (auto& elm : getKeyboardMap())
		{
			io.AddKeyEvent(elm.second, keyb.getKey(elm.first));
		}
		for (auto& gamepadNum : input.getImpl().getGamepadNums())
		{
			auto& gamepad = input.getGamepad(gamepadNum);
			for (auto& elm : getGamepadMap())
			{
				io.AddKeyEvent(elm.second, gamepad.getButton(elm.first));
			}
		}
	}

	ImguiAppComponent::ImguiAppComponent(IImguiRenderer& renderer, float fontSize) noexcept
		: _impl{ std::make_unique<ImguiAppComponentImpl>(renderer, fontSize) }
	{
	}

	ImguiAppComponent::~ImguiAppComponent() = default;

	expected<void, std::string> ImguiAppComponent::init(App& app) noexcept
	{
		return _impl->init(app);
	}

	expected<void, std::string> ImguiAppComponent::shutdown() noexcept
	{
		return _impl->shutdown();
	}

	expected<bgfx::ViewId, std::string> ImguiAppComponent::renderReset(bgfx::ViewId viewId) noexcept
	{
		return _impl->renderReset(viewId);
	}

	expected<void, std::string> ImguiAppComponent::render() noexcept
	{
		return _impl->render();
	}

	expected<void, std::string> ImguiAppComponent::update(float dt) noexcept
	{
		return _impl->update(dt);
	}

	ImGuiContext* ImguiAppComponent::getContext() noexcept
	{
		return _impl->getContext();
	}

	bool ImguiAppComponent::getInputEnabled() const noexcept
	{
		return _impl->getInputEnabled();
	}

	ImguiAppComponent& ImguiAppComponent::setInputEnabled(bool enabled) noexcept
	{
		_impl->setInputEnabled(enabled);
		return *this;
	}

	expected<void, std::string> ImguiAppComponent::updateFonts() noexcept
	{
		return _impl->updateFonts();
	}
}