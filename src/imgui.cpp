#include "imgui.hpp"
#include "asset.hpp"
#include "input.hpp"
#include "platform.hpp"
#include <darmok/imgui.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <bx/allocator.h>
#include "generated/imgui/shaders/basic.vertex.h"
#include "generated/imgui/shaders/basic.fragment.h"
#include "generated/imgui/shaders/lod.vertex.h"
#include "generated/imgui/shaders/lod.fragment.h"
#include "generated/shaders/gui.layout.h"

namespace darmok
{
	const bgfx::EmbeddedShader ImguiAppComponentImpl::_embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(imgui_basic_vertex),
		BGFX_EMBEDDED_SHADER(imgui_basic_fragment),
		BGFX_EMBEDDED_SHADER(imgui_lod_vertex),
		BGFX_EMBEDDED_SHADER(imgui_lod_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	const uint8_t ImguiAppComponentImpl::_imguiAlphaBlendFlags = 0x01;

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
			{ KeyboardKey::Tilde, ImGuiKey_GraveAccent},
		};
		static bool first = true;

		auto addRange = [](KeyboardKey start, KeyboardKey end, ImGuiKey imguiStart) {
			auto j = to_underlying(imguiStart);
			for (auto i = to_underlying(start); i < to_underlying(end); i++)
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
			{ GamepadButton::None, ImGuiKey_None },
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

	bool ImguiAppComponentImpl::render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData) const noexcept
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
		{
			return false;
		}

		bgfx::setViewName(viewId, "ImGui");
		bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = drawData->DisplayPos.x;
			float y = drawData->DisplayPos.y;
			float width = drawData->DisplaySize.x;
			float height = drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(viewId, NULL, ortho);
			bgfx::setViewRect(viewId, 0, 0, uint16_t(width), uint16_t(height));
		}

		const ImVec2 clipPos = drawData->DisplayPos;       // (0,0) unless using multi-viewports
		const ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		auto& layout = _basicProgram->getVertexLayout();

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

			if (!checkAvailTransientBuffers(numVertices, layout, numIndices))
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, layout);
			bgfx::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));

			bgfx::Encoder* encoder = bgfx::begin();

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

					bgfx::TextureHandle th = texture;
					bgfx::ProgramHandle program = _basicProgram->getHandle();

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
							program = _lodProgram->getHandle();
							const float lodEnabled[4] = { float(texture.s.mip), mipEnabled ? 1.0F : 0.0F, 0.0F, 0.0F };
							bgfx::setUniform(_lodEnabledUniform, lodEnabled);
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_ALPHA;
					}

					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (cmd->ClipRect.x - clipPos.x) * clipScale.x;
					clipRect.y = (cmd->ClipRect.y - clipPos.y) * clipScale.y;
					clipRect.z = (cmd->ClipRect.z - clipPos.x) * clipScale.x;
					clipRect.w = (cmd->ClipRect.w - clipPos.y) * clipScale.y;

					if (clipRect.x < fb_width
						&& clipRect.y < fb_height
						&& clipRect.z >= 0.0f
						&& clipRect.w >= 0.0f)
					{
						const uint16_t xx = uint16_t(bx::max(clipRect.x, 0.0f));
						const uint16_t yy = uint16_t(bx::max(clipRect.y, 0.0f));
						encoder->setScissor(xx, yy
							, uint16_t(bx::min(clipRect.z, 65535.0f) - xx)
							, uint16_t(bx::min(clipRect.w, 65535.0f) - yy)
						);

						encoder->setState(state);
						encoder->setTexture(0, _textureUniform, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}

		return true;
	}

	ImguiAppComponentImpl::ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize)
		: _renderer(renderer)
		, _texture{ bgfx::kInvalidHandle }
		, _textureUniform{ bgfx::kInvalidHandle }
		, _lodEnabledUniform{ bgfx::kInvalidHandle }
	{
		IMGUI_CHECKVERSION();
	}

	void ImguiAppComponentImpl::init(App& app)
	{
		_app = app;
		ImGui::SetAllocatorFunctions(memAlloc, memFree, &app.getAssets().getAllocator());

		_basicProgram = std::make_unique<Program>("imgui_basic", _embeddedShaders, gui_layout);
		_lodProgram = std::make_unique<Program>("imgui_lod", _embeddedShaders, gui_layout);
		_lodEnabledUniform = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		_textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		_imgui = ImGui::CreateContext();
		ImGui::SetCurrentContext(_imgui);

		ImGuiIO& io = ImGui::GetIO();

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

		uint8_t* data;
		int32_t width;
		int32_t height;
		{
			ImFontConfig config;
			config.FontDataOwnedByAtlas = false;
			config.MergeMode = false;
			// config.MergeGlyphCenterV = true;
		}

		io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

		_texture = bgfx::createTexture2D(
			(uint16_t)width
			, (uint16_t)height
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(data, width * height * 4)
		);

		ImGui::SetCurrentContext(nullptr);
	}

	void ImguiAppComponentImpl::shutdown() noexcept
	{
		ImGui::DestroyContext(_imgui);
		bgfx::destroy(_textureUniform);
		bgfx::destroy(_lodEnabledUniform);
	}

	void ImguiAppComponentImpl::updateLogic(float dt) noexcept
	{
		ImGui::SetCurrentContext(_imgui);
		updateInput(dt);
		ImGui::SetCurrentContext(nullptr);
	}

	bool ImguiAppComponentImpl::render(bgfx::ViewId viewId) const noexcept
	{
		ImGui::SetCurrentContext(_imgui);

		beginFrame();
		_renderer.imguiRender();
		auto rendered = endFrame(viewId);

		ImGui::SetCurrentContext(nullptr);

		return rendered;
	}

	void ImguiAppComponentImpl::updateInput(float dt) noexcept
	{
		if (!_app.hasValue())
		{
			return;
		}
		auto& input = _app->getInput();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;

		for (auto& inputChar : input.getImpl().getKeyboard().getUpdateChars())
		{
			io.AddInputCharacter(inputChar.data);
		}

		auto& mouse = input.getMouse();
		auto& size = _app->getWindow().getSize();
		io.DisplaySize = ImVec2((float)size.x, (float)size.y);

		auto& buttons = mouse.getButtons();
		auto& pos = mouse.getPosition();
		io.AddMousePosEvent(pos.x, size.y - pos.y);
		io.AddMouseButtonEvent(ImGuiMouseButton_Left, buttons[to_underlying(MouseButton::Left)]);
		io.AddMouseButtonEvent(ImGuiMouseButton_Right, buttons[to_underlying(MouseButton::Right)]);
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, buttons[to_underlying(MouseButton::Middle)]);
		
		auto& scroll = mouse.getScrollDelta();
		io.AddMouseWheelEvent(scroll.x, scroll.y);

		auto& kb = input.getKeyboard();
		uint8_t modifiers = kb.getModifiers();
		io.AddKeyEvent(ImGuiMod_Shift, 0 != (modifiers & to_underlying(KeyboardModifierGroup::Shift)));
		io.AddKeyEvent(ImGuiMod_Ctrl, 0 != (modifiers & to_underlying(KeyboardModifierGroup::Ctrl)));
		io.AddKeyEvent(ImGuiMod_Alt, 0 != (modifiers & to_underlying(KeyboardModifierGroup::Alt)));
		io.AddKeyEvent(ImGuiMod_Super, 0 != (modifiers & to_underlying(KeyboardModifierGroup::Meta)));
		for (auto& elm : getKeyboardMap())
		{
			io.AddKeyEvent(elm.second, kb.getKey(elm.first));
			io.SetKeyEventNativeData(elm.second, 0, 0, (int)elm.first);
		}
		for (auto& gamepad : input.getGamepads())
		{
			for (auto& elm : getGamepadMap())
			{
				io.AddKeyEvent(elm.second, gamepad.getButton(elm.first));
				io.SetKeyEventNativeData(elm.second, 0, 0, (int)elm.first);
			}
		}
	}

	void ImguiAppComponentImpl::beginFrame() const noexcept
	{
		ImGui::NewFrame();
	}

	bool ImguiAppComponentImpl::endFrame(bgfx::ViewId viewId) const noexcept
	{
		ImGui::Render();
		return render(viewId, _texture, ImGui::GetDrawData());
	}

	ImguiAppComponent::ImguiAppComponent(IImguiRenderer& renderer, float fontSize) noexcept
		: _impl(std::make_unique<ImguiAppComponentImpl>(renderer, fontSize))
	{
	}

	void ImguiAppComponent::init(App& app)
	{
		_impl->init(app);
	}

	void ImguiAppComponent::shutdown() noexcept
	{
		_impl->shutdown();
	}

	void ImguiAppComponent::updateLogic(float dt) noexcept
	{
		_impl->updateLogic(dt);
	}

	bgfx::ViewId ImguiAppComponent::render(bgfx::ViewId viewId) const noexcept
	{
		if (_impl->render(viewId))
		{
			++viewId;
		}
		return viewId;
	}
}