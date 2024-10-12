#include "imgui.hpp"
#include "asset.hpp"
#include "input.hpp"
#include "platform.hpp"
#include <darmok/imgui.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/mesh.hpp>
#include <darmok/math.hpp>
#include <darmok/program_core.hpp>
#include <bx/allocator.h>
#include "generated/imgui/imgui.program.h"

namespace darmok
{
	struct ImguiUtils
	{
		static glm::vec2 convert(const ImVec2& v) noexcept
		{
			return glm::vec2(v.x, v.y);
		}

		static ImVec2 convert(const glm::vec2& v) noexcept
		{
			return ImVec2(v.x, v.y);
		}

		static glm::vec4 convert(const ImVec4& v) noexcept
		{
			return glm::vec4(v.x, v.y, v.z, v.w);
		}

		static ImVec4 convert(const glm::vec4& v) noexcept
		{
			return ImVec4(v.x, v.y, v.z, v.w);
		}

		static uint16_t convertUint16(float v) noexcept
		{
			v = bx::max(v, 0.0f);
			v = bx::min(v, 65535.0f);
			return uint16_t(v);
		}
	};

	ImguiRenderPass::ImguiRenderPass(IImguiRenderer& renderer, ImGuiContext* imgui) noexcept
		: _renderer(renderer)
		, _imgui(imgui)
		, _textureUniform{ bgfx::kInvalidHandle }
		, _lodEnabledUniform{ bgfx::kInvalidHandle }
		, _viewId(0)
		, _fontsTexture{ bgfx::kInvalidHandle }
	{
		ProgramDefinition progDef;
		progDef.loadStaticMem(imgui_program);
		_program = std::make_unique<Program>(progDef);
		
		_lodEnabledUniform = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		_textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		uint8_t* data;
		int32_t width;
		int32_t height;
		{
			ImFontConfig config;
			config.FontDataOwnedByAtlas = false;
			config.MergeMode = false;
			// config.MergeGlyphCenterV = true;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

		_fontsTexture = bgfx::createTexture2D(
			(uint16_t)width
			, (uint16_t)height
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(data, width * height * 4)
		);
	}

	ImguiRenderPass::~ImguiRenderPass() noexcept
	{
		if (isValid(_textureUniform))
		{
			bgfx::destroy(_textureUniform);
		}
		if (isValid(_lodEnabledUniform))
		{
			bgfx::destroy(_lodEnabledUniform);
		}
		if (isValid(_fontsTexture))
		{
			bgfx::destroy(_fontsTexture);
		}
	}

	void ImguiRenderPass::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName("Imgui");
	}

	void ImguiRenderPass::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
		_viewId = viewId;
	}

	void ImguiRenderPass::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		ImGui::SetCurrentContext(_imgui);
		beginFrame();
		_renderer.imguiRender();
		endFrame(context.getEncoder());
		ImGui::SetCurrentContext(nullptr);
	}

	void ImguiRenderPass::beginFrame() const noexcept
	{
		ImGui::NewFrame();
	}

	bool ImguiRenderPass::endFrame(bgfx::Encoder& encoder) const noexcept
	{
		ImGui::Render();
		return render(encoder, ImGui::GetDrawData());
	}

	const uint8_t ImguiRenderPass::_imguiAlphaBlendFlags = 0x01;

	bool ImguiRenderPass::render(bgfx::Encoder& encoder, ImDrawData* drawData) const noexcept
	{
		auto clipPos = ImguiUtils::convert(drawData->DisplayPos); // (0,0) unless using multi-viewports
		auto size = ImguiUtils::convert(drawData->DisplaySize);
		auto clipScale = ImguiUtils::convert(drawData->FramebufferScale);  // (1,1) unless using retina display which are often (2,2)
		auto clipSize = size * clipScale;

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		if (clipSize.x <= 0 || clipSize.y <= 0)
		{
			return false;
		}

		{
			auto ortho = Math::ortho(clipPos.x, clipPos.x + clipSize.x, clipPos.y + clipSize.y, clipPos.y);
			bgfx::setViewTransform(_viewId, nullptr, glm::value_ptr(ortho));
			bgfx::setViewRect(_viewId, 0, 0,
				ImguiUtils::convertUint16(clipSize.x),
				ImguiUtils::convertUint16(clipSize.y)
			);
		}

		auto& layout = _program->getVertexLayout();

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			const ImDrawList* drawList = drawData->CmdLists[ii];

			auto numVertices = (uint32_t)drawList->VtxBuffer.size();
			auto numIndices = (uint32_t)drawList->IdxBuffer.size();

			DataView vertData(&drawList->VtxBuffer.front(), numVertices * sizeof(ImDrawVert));
			DataView idxData(&drawList->IdxBuffer.front(), numIndices * sizeof(ImDrawIdx));
			std::optional<TransientMesh> mesh;
			try
			{
				mesh.emplace(layout, vertData, idxData);
			}
			catch (...)
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

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

					auto th = _fontsTexture;
					auto program = _program->getHandle();

					if (NULL != cmd->TextureId)
					{
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (_imguiAlphaBlendFlags & texture.s.flags)
							? BGFX_STATE_BLEND_ALPHA
							: BGFX_STATE_NONE
							;
						th = texture.s.handle;
						auto mipEnabled = 0 != texture.s.mip;
						if (mipEnabled)
						{
							program = _program->getHandle({ "LOD_ENABLED" });
							const float lodEnabled[4] = { float(texture.s.mip), mipEnabled ? 1.0F : 0.0F, 0.0F, 0.0F };
							bgfx::setUniform(_lodEnabledUniform, lodEnabled);
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_ALPHA;
					}

					// Project scissor/clipping rectangles into framebuffer space
					auto clipRect = ImguiUtils::convert(cmd->ClipRect);
					clipRect = (clipRect - glm::vec4(clipPos, clipPos)) * glm::vec4(clipScale, clipScale);

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
						encoder.setTexture(0, _textureUniform, th);
						mesh->render(encoder);
						encoder.submit(_viewId, program);
					}
				}
			}
		}

		return true;
	}

	const ImguiAppComponentImpl::KeyboardMap& ImguiAppComponentImpl::getKeyboardMap() noexcept
	{
		static KeyboardMap map
		{
			{ KeyboardKey::Esc, ImGuiKey_Escape},
			{ KeyboardKey::Return, ImGuiKey_Enter},
			{ KeyboardKey::Tab, ImGuiKey_Tab},
			{ KeyboardKey::Space, ImGuiKey_Space},
			{ KeyboardKey::Backspace, ImGuiKey_Backspace},
			{ KeyboardKey::Up, ImGuiKey_UpArrow},
			{ KeyboardKey::Down, ImGuiKey_DownArrow},
			{ KeyboardKey::Left, ImGuiKey_LeftArrow},
			{ KeyboardKey::Right, ImGuiKey_RightArrow},
			{ KeyboardKey::Insert, ImGuiKey_Insert},
			{ KeyboardKey::Delete, ImGuiKey_Delete},
			{ KeyboardKey::Home, ImGuiKey_Home},
			{ KeyboardKey::End, ImGuiKey_End},
			{ KeyboardKey::PageUp, ImGuiKey_PageUp},
			{ KeyboardKey::PageDown, ImGuiKey_PageDown},
			{ KeyboardKey::Print, ImGuiKey_PrintScreen},
			{ KeyboardKey::Plus, ImGuiKey_Equal},
			{ KeyboardKey::Minus, ImGuiKey_Minus},
			{ KeyboardKey::LeftBracket, ImGuiKey_LeftBracket},
			{ KeyboardKey::RightBracket, ImGuiKey_RightBracket},
			{ KeyboardKey::Semicolon, ImGuiKey_Semicolon},
			{ KeyboardKey::Quote, ImGuiKey_Apostrophe},
			{ KeyboardKey::Comma, ImGuiKey_Comma},
			{ KeyboardKey::Period, ImGuiKey_Period},
			{ KeyboardKey::Slash, ImGuiKey_Slash},
			{ KeyboardKey::Backslash, ImGuiKey_Backslash},
			{ KeyboardKey::GraveAccent, ImGuiKey_GraveAccent},
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
			addRange(KeyboardKey::F1, KeyboardKey::F12, ImGuiKey_F1);
			addRange(KeyboardKey::NumPad0, KeyboardKey::NumPad9, ImGuiKey_Keypad9);
			addRange(KeyboardKey::Key0, KeyboardKey::Key9, ImGuiKey_0);
			addRange(KeyboardKey::KeyA, KeyboardKey::KeyZ, ImGuiKey_A);
			first = false;
		}
		return map;
	}


	const ImguiAppComponentImpl::GamepadMap& ImguiAppComponentImpl::getGamepadMap() noexcept
	{
		static GamepadMap map
		{
			{ GamepadButton::Start, ImGuiKey_GamepadStart },
			{ GamepadButton::Back, ImGuiKey_GamepadBack },
			{ GamepadButton::Y, ImGuiKey_GamepadFaceUp },
			{ GamepadButton::A, ImGuiKey_GamepadFaceDown },
			{ GamepadButton::X, ImGuiKey_GamepadFaceLeft },
			{ GamepadButton::B, ImGuiKey_GamepadFaceRight },
			{ GamepadButton::Up, ImGuiKey_GamepadDpadUp },
			{ GamepadButton::Down, ImGuiKey_GamepadDpadDown },
			{ GamepadButton::Left, ImGuiKey_GamepadDpadLeft },
			{ GamepadButton::Right, ImGuiKey_GamepadDpadRight },
			{ GamepadButton::ShoulderL, ImGuiKey_GamepadL1 },
			{ GamepadButton::ShoulderR, ImGuiKey_GamepadR1 },
			{ GamepadButton::ThumbL, ImGuiKey_GamepadL3 },
			{ GamepadButton::ThumbR, ImGuiKey_GamepadR3 },
			{ GamepadButton::Guide, ImGuiKey_None },
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
		: _renderer(renderer)
		, _inputEnabled(true)
		, _imgui(nullptr)
	{
		IMGUI_CHECKVERSION();
	}

	ImguiAppComponentImpl::~ImguiAppComponentImpl() noexcept
	{
		shutdown();
	}

	void ImguiAppComponentImpl::init(App& app)
	{
		_app = app;
		ImGui::SetAllocatorFunctions(memAlloc, memFree, &app.getAssets().getAllocator());

		_imgui = ImGui::CreateContext();
		ImGui::SetCurrentContext(_imgui);

		auto& io = ImGui::GetIO();

		io.DeltaTime = 1.0f / 60.0f;
		io.IniFilename = NULL;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&style);

		style.FrameRounding = 4.0f;
		style.WindowBorderSize = 0.0f;

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		io.ConfigFlags |= 0
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_NavEnableKeyboard
			;

		_renderPass.emplace(_renderer, _imgui);
		app.getRenderGraph().addPass(_renderPass.value());

		ImGui::SetCurrentContext(nullptr);
	}

	void ImguiAppComponentImpl::renderReset() noexcept
	{
		if (_app && _renderPass)
		{
			_app->getRenderGraph().addPass(_renderPass.value());
		}
	}

	void ImguiAppComponentImpl::shutdown() noexcept
	{
		if (_app && _renderPass)
		{
			_app->getRenderGraph().removePass(_renderPass.value());
		}
		_app.reset();
		_renderPass.reset();

		if (_imgui != nullptr)
		{
			ImGui::DestroyContext(_imgui);
			_imgui = nullptr;
		}
	}

	void ImguiAppComponentImpl::update(float dt) noexcept
	{
		ImGui::SetCurrentContext(_imgui);
		updateInput(dt);
		ImGui::SetCurrentContext(nullptr);
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

	void ImguiAppComponentImpl::updateInput(float dt) noexcept
	{
		if (!_inputEnabled || !_app)
		{
			return;
		}
		auto& input = _app->getInput();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;

		for (auto& inputChar : input.getImpl().getKeyboard().getUpdateChars())
		{
			io.AddInputCharacter(inputChar.code);
		}

		auto& win = _app->getWindow();

		auto& pixelSize = win.getPixelSize();
		io.DisplaySize = ImguiUtils::convert(pixelSize);
		io.DisplayFramebufferScale = ImguiUtils::convert(win.getFramebufferScale());

		auto& mouse = input.getMouse();
		auto& buttons = mouse.getButtons();
		auto pos = win.windowToScreenPoint(mouse.getPosition());
		io.AddMousePosEvent(pos.x, pos.y);

		io.AddMouseButtonEvent(ImGuiMouseButton_Left, buttons[toUnderlying(MouseButton::Left)]);
		io.AddMouseButtonEvent(ImGuiMouseButton_Right, buttons[toUnderlying(MouseButton::Right)]);
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, buttons[toUnderlying(MouseButton::Middle)]);
		
		auto& scroll = mouse.getScroll();
		io.AddMouseWheelEvent(scroll.x, scroll.y);

		auto& kb = input.getKeyboard();
		auto& modifiers = kb.getModifiers();
		io.AddKeyEvent(ImGuiMod_Shift, modifiers.contains(KeyboardModifier::Shift));
		io.AddKeyEvent(ImGuiMod_Ctrl, modifiers.contains(KeyboardModifier::Ctrl));
		io.AddKeyEvent(ImGuiMod_Alt, modifiers.contains(KeyboardModifier::Alt));
		io.AddKeyEvent(ImGuiMod_Super, modifiers.contains(KeyboardModifier::Meta));
		for (auto& elm : getKeyboardMap())
		{
			io.AddKeyEvent(elm.second, kb.getKey(elm.first));
		}
		for (auto& gamepad : input.getGamepads())
		{
			for (auto& elm : getGamepadMap())
			{
				io.AddKeyEvent(elm.second, gamepad.getButton(elm.first));
			}
		}
	}

	ImguiAppComponent::ImguiAppComponent(IImguiRenderer& renderer, float fontSize) noexcept
		: _impl(std::make_unique<ImguiAppComponentImpl>(renderer, fontSize))
	{
	}

	ImguiAppComponent::~ImguiAppComponent()
	{
		// empty for the impl forward declaration
	}

	void ImguiAppComponent::init(App& app)
	{
		_impl->init(app);
	}

	void ImguiAppComponent::shutdown() noexcept
	{
		_impl->shutdown();
	}

	void ImguiAppComponent::renderReset() noexcept
	{
		_impl->renderReset();
	}

	void ImguiAppComponent::update(float dt) noexcept
	{
		_impl->update(dt);
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
}