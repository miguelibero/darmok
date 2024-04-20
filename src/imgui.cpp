#include "imgui.hpp"
#include "asset.hpp"
#include "input.hpp"
#include "platform.hpp"
#include <darmok/imgui.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <bx/allocator.h>

#include <dear-imgui/imgui.h>
#include <dear-imgui/imgui_internal.h>
#include <bgfx/embedded_shader.h>

// shaders (using the ones in bgfx examples)
#include <bgfx/embedded_shader.h>
#include "imgui/vs_ocornut_imgui.bin.h"
#include "imgui/fs_ocornut_imgui.bin.h"
#include "imgui/vs_imgui_image.bin.h"
#include "imgui/fs_imgui_image.bin.h"

// fonts (using the ones in bgfx examples)
#include "imgui/roboto_regular.ttf.h"
#include "imgui/robotomono_regular.ttf.h"
#include "imgui/icons_kenney.ttf.h"
#include "imgui/icons_font_awesome.ttf.h"
#include <iconfontheaders/icons_kenney.h>
#include <iconfontheaders/icons_font_awesome.h>

namespace darmok
{
	static const uint8_t _imguiAlphaBlendFlags = 0x01;

	static const bgfx::EmbeddedShader _imguiEmbeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(vs_imgui_image),
		BGFX_EMBEDDED_SHADER(fs_imgui_image),

		BGFX_EMBEDDED_SHADER_END()
	};

	struct ImguiFontRangeMerge final
	{
		const void* data;
		size_t      size;
		ImWchar     ranges[3];
	};

	static const std::vector<ImguiFontRangeMerge> _imguiFontRangeMerge =
	{
		{ s_iconsKenneyTtf,      sizeof(s_iconsKenneyTtf),      { ICON_MIN_KI, ICON_MAX_KI, 0 } },
		{ s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), { ICON_MIN_FA, ICON_MAX_FA, 0 } },
	};

	using ImguiKeyboardMap = std::array<ImGuiKey, to_underlying(KeyboardKey::Count)>;

	static ImguiKeyboardMap createImguiKeyboardMap()
	{
		ImguiKeyboardMap map;

		for (int32_t i = 0; i < (int32_t)KeyboardKey::Count; ++i)
		{
			map[i] = ImGuiKey_None;
		}

		map[to_underlying(KeyboardKey::Esc)] = ImGuiKey_Escape;
		map[to_underlying(KeyboardKey::Return)] = ImGuiKey_Enter;
		map[to_underlying(KeyboardKey::Tab)] = ImGuiKey_Tab;
		map[to_underlying(KeyboardKey::Space)] = ImGuiKey_Space;
		map[to_underlying(KeyboardKey::Backspace)] = ImGuiKey_Backspace;
		map[to_underlying(KeyboardKey::Up)] = ImGuiKey_UpArrow;
		map[to_underlying(KeyboardKey::Down)] = ImGuiKey_DownArrow;
		map[to_underlying(KeyboardKey::Left)] = ImGuiKey_LeftArrow;
		map[to_underlying(KeyboardKey::Right)] = ImGuiKey_RightArrow;
		map[to_underlying(KeyboardKey::Insert)] = ImGuiKey_Insert;
		map[to_underlying(KeyboardKey::Delete)] = ImGuiKey_Delete;
		map[to_underlying(KeyboardKey::Home)] = ImGuiKey_Home;
		map[to_underlying(KeyboardKey::End)] = ImGuiKey_End;
		map[to_underlying(KeyboardKey::PageUp)] = ImGuiKey_PageUp;
		map[to_underlying(KeyboardKey::PageDown)] = ImGuiKey_PageDown;
		map[to_underlying(KeyboardKey::Print)] = ImGuiKey_PrintScreen;
		map[to_underlying(KeyboardKey::Plus)] = ImGuiKey_Equal;
		map[to_underlying(KeyboardKey::Minus)] = ImGuiKey_Minus;
		map[to_underlying(KeyboardKey::LeftBracket)] = ImGuiKey_LeftBracket;
		map[to_underlying(KeyboardKey::RightBracket)] = ImGuiKey_RightBracket;
		map[to_underlying(KeyboardKey::Semicolon)] = ImGuiKey_Semicolon;
		map[to_underlying(KeyboardKey::Quote)] = ImGuiKey_Apostrophe;
		map[to_underlying(KeyboardKey::Comma)] = ImGuiKey_Comma;
		map[to_underlying(KeyboardKey::Period)] = ImGuiKey_Period;
		map[to_underlying(KeyboardKey::Slash)] = ImGuiKey_Slash;
		map[to_underlying(KeyboardKey::Backslash)] = ImGuiKey_Backslash;
		map[to_underlying(KeyboardKey::Tilde)] = ImGuiKey_GraveAccent;
		map[to_underlying(KeyboardKey::F1)] = ImGuiKey_F1;
		map[to_underlying(KeyboardKey::F2)] = ImGuiKey_F2;
		map[to_underlying(KeyboardKey::F3)] = ImGuiKey_F3;
		map[to_underlying(KeyboardKey::F4)] = ImGuiKey_F4;
		map[to_underlying(KeyboardKey::F5)] = ImGuiKey_F5;
		map[to_underlying(KeyboardKey::F6)] = ImGuiKey_F6;
		map[to_underlying(KeyboardKey::F7)] = ImGuiKey_F7;
		map[to_underlying(KeyboardKey::F8)] = ImGuiKey_F8;
		map[to_underlying(KeyboardKey::F9)] = ImGuiKey_F9;
		map[to_underlying(KeyboardKey::F10)] = ImGuiKey_F10;
		map[to_underlying(KeyboardKey::F11)] = ImGuiKey_F11;
		map[to_underlying(KeyboardKey::F12)] = ImGuiKey_F12;
		map[to_underlying(KeyboardKey::NumPad0)] = ImGuiKey_Keypad0;
		map[to_underlying(KeyboardKey::NumPad1)] = ImGuiKey_Keypad1;
		map[to_underlying(KeyboardKey::NumPad2)] = ImGuiKey_Keypad2;
		map[to_underlying(KeyboardKey::NumPad3)] = ImGuiKey_Keypad3;
		map[to_underlying(KeyboardKey::NumPad4)] = ImGuiKey_Keypad4;
		map[to_underlying(KeyboardKey::NumPad5)] = ImGuiKey_Keypad5;
		map[to_underlying(KeyboardKey::NumPad6)] = ImGuiKey_Keypad6;
		map[to_underlying(KeyboardKey::NumPad7)] = ImGuiKey_Keypad7;
		map[to_underlying(KeyboardKey::NumPad8)] = ImGuiKey_Keypad8;
		map[to_underlying(KeyboardKey::NumPad9)] = ImGuiKey_Keypad9;
		map[to_underlying(KeyboardKey::Key0)] = ImGuiKey_0;
		map[to_underlying(KeyboardKey::Key1)] = ImGuiKey_1;
		map[to_underlying(KeyboardKey::Key2)] = ImGuiKey_2;
		map[to_underlying(KeyboardKey::Key3)] = ImGuiKey_3;
		map[to_underlying(KeyboardKey::Key4)] = ImGuiKey_4;
		map[to_underlying(KeyboardKey::Key5)] = ImGuiKey_5;
		map[to_underlying(KeyboardKey::Key6)] = ImGuiKey_6;
		map[to_underlying(KeyboardKey::Key7)] = ImGuiKey_7;
		map[to_underlying(KeyboardKey::Key8)] = ImGuiKey_8;
		map[to_underlying(KeyboardKey::Key9)] = ImGuiKey_9;
		map[to_underlying(KeyboardKey::KeyA)] = ImGuiKey_A;
		map[to_underlying(KeyboardKey::KeyB)] = ImGuiKey_B;
		map[to_underlying(KeyboardKey::KeyC)] = ImGuiKey_C;
		map[to_underlying(KeyboardKey::KeyD)] = ImGuiKey_D;
		map[to_underlying(KeyboardKey::KeyE)] = ImGuiKey_E;
		map[to_underlying(KeyboardKey::KeyF)] = ImGuiKey_F;
		map[to_underlying(KeyboardKey::KeyG)] = ImGuiKey_G;
		map[to_underlying(KeyboardKey::KeyH)] = ImGuiKey_H;
		map[to_underlying(KeyboardKey::KeyI)] = ImGuiKey_I;
		map[to_underlying(KeyboardKey::KeyJ)] = ImGuiKey_J;
		map[to_underlying(KeyboardKey::KeyK)] = ImGuiKey_K;
		map[to_underlying(KeyboardKey::KeyL)] = ImGuiKey_L;
		map[to_underlying(KeyboardKey::KeyM)] = ImGuiKey_M;
		map[to_underlying(KeyboardKey::KeyN)] = ImGuiKey_N;
		map[to_underlying(KeyboardKey::KeyO)] = ImGuiKey_O;
		map[to_underlying(KeyboardKey::KeyP)] = ImGuiKey_P;
		map[to_underlying(KeyboardKey::KeyQ)] = ImGuiKey_Q;
		map[to_underlying(KeyboardKey::KeyR)] = ImGuiKey_R;
		map[to_underlying(KeyboardKey::KeyS)] = ImGuiKey_S;
		map[to_underlying(KeyboardKey::KeyT)] = ImGuiKey_T;
		map[to_underlying(KeyboardKey::KeyU)] = ImGuiKey_U;
		map[to_underlying(KeyboardKey::KeyV)] = ImGuiKey_V;
		map[to_underlying(KeyboardKey::KeyW)] = ImGuiKey_W;
		map[to_underlying(KeyboardKey::KeyX)] = ImGuiKey_X;
		map[to_underlying(KeyboardKey::KeyY)] = ImGuiKey_Y;
		map[to_underlying(KeyboardKey::KeyZ)] = ImGuiKey_Z;

		return map;
	}

	using ImguiGamepadMap = std::array<ImGuiKey, to_underlying(GamepadButton::Count)>;


	static ImguiGamepadMap createImguiGamepadMap()
	{
		ImguiGamepadMap map;
		map[to_underlying(GamepadButton::None)] = ImGuiKey_None;
		map[to_underlying(GamepadButton::Start)] = ImGuiKey_GamepadStart;
		map[to_underlying(GamepadButton::Back)] = ImGuiKey_GamepadBack;
		map[to_underlying(GamepadButton::Y)] = ImGuiKey_GamepadFaceUp;
		map[to_underlying(GamepadButton::A)] = ImGuiKey_GamepadFaceDown;
		map[to_underlying(GamepadButton::X)] = ImGuiKey_GamepadFaceLeft;
		map[to_underlying(GamepadButton::B)] = ImGuiKey_GamepadFaceRight;
		map[to_underlying(GamepadButton::Up)] = ImGuiKey_GamepadDpadUp;
		map[to_underlying(GamepadButton::Down)] = ImGuiKey_GamepadDpadDown;
		map[to_underlying(GamepadButton::Left)] = ImGuiKey_GamepadDpadLeft;
		map[to_underlying(GamepadButton::Right)] = ImGuiKey_GamepadDpadRight;
		map[to_underlying(GamepadButton::ShoulderL)] = ImGuiKey_GamepadL1;
		map[to_underlying(GamepadButton::ShoulderR)] = ImGuiKey_GamepadR1;
		map[to_underlying(GamepadButton::ThumbL)] = ImGuiKey_GamepadL3;
		map[to_underlying(GamepadButton::ThumbR)] = ImGuiKey_GamepadR3;
		map[to_underlying(GamepadButton::Guide)] = ImGuiKey_None;

		return map;
	}

	static const ImguiKeyboardMap _imguiKeyboardMap = createImguiKeyboardMap();
	static const ImguiGamepadMap _imguiGamepadMap = createImguiGamepadMap();

	void* imguiMemAlloc(size_t size, void* userData)
	{
		if (userData == nullptr)
		{
			return std::malloc(size);
		}
		auto alloc = static_cast<bx::AllocatorI*>(userData);
		return bx::alloc(alloc, size);
	}

	void imguiMemFree(void* ptr, void* userData)
	{
		if (userData == nullptr)
		{
			return std::free(ptr);
		}
		auto alloc = static_cast<bx::AllocatorI*>(userData);
		bx::free(alloc, ptr);
	}

	inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices)
	{
		return numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, layout)
			&& (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices))
			;
	}

	void ImguiAppComponentImpl::render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
		{
			return;
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

		// Render command lists
		for (int32_t ii = 0, num = drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices = (uint32_t)drawList->IdxBuffer.size();

			if (!checkAvailTransientBuffers(numVertices, _layout, numIndices))
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, _layout);
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
					bgfx::ProgramHandle program = _program;

					if (NULL != cmd->TextureId)
					{
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (_imguiAlphaBlendFlags & texture.s.flags)
							? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							: BGFX_STATE_NONE
							;
						th = texture.s.handle;
						if (0 != texture.s.mip)
						{
							const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
							bgfx::setUniform(_imageLodEnabled, lodEnabled);
							program = _imageProgram;
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
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
						encoder->setTexture(0, _uniform, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}

	ImguiAppComponentImpl::ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize)
		: _renderer(renderer)
		, _texture{ bgfx::kInvalidHandle }
		, _font{}
		, _fontSize(fontSize)
		, _program{ bgfx::kInvalidHandle }
		, _imageProgram{ bgfx::kInvalidHandle }
		, _uniform{ bgfx::kInvalidHandle }
		, _imageLodEnabled{ bgfx::kInvalidHandle }
	{
		IMGUI_CHECKVERSION();
	}

	void ImguiAppComponentImpl::init(App& app)
	{
		_app = app;
		ImGui::SetAllocatorFunctions(imguiMemAlloc, imguiMemFree, app.getAssets().getImpl().getAllocator());

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		_program = bgfx::createProgram(
			bgfx::createEmbeddedShader(_imguiEmbeddedShaders, type, "vs_ocornut_imgui")
			, bgfx::createEmbeddedShader(_imguiEmbeddedShaders, type, "fs_ocornut_imgui")
			, true
		);

		_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		_imageProgram = bgfx::createProgram(
			bgfx::createEmbeddedShader(_imguiEmbeddedShaders, type, "vs_imgui_image")
			, bgfx::createEmbeddedShader(_imguiEmbeddedShaders, type, "fs_imgui_image")
			, true
		);

		_layout
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();

		_uniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		ImGui::InitDockContext();

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

			const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
			_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf), _fontSize, &config, ranges);
			_font[ImGui::Font::Mono] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), _fontSize - 3.0f, &config, ranges);

			config.MergeMode = true;
			config.DstFont = _font[ImGui::Font::Regular];

			for(auto& frm : _imguiFontRangeMerge)
			{
				io.Fonts->AddFontFromMemoryTTF((void*)frm.data
					, (int)frm.size
					, _fontSize - 3.0f
					, &config
					, frm.ranges
				);
			}
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

	void ImguiAppComponentImpl::shutdown()
	{
		ImGui::DestroyContext(_imgui);
		ImGui::ShutdownDockContext();
		bgfx::destroy(_uniform);
		bgfx::destroy(_imageLodEnabled);
		bgfx::destroy(_imageProgram);
		bgfx::destroy(_program);
	}

	void ImguiAppComponentImpl::updateLogic(float dt)
	{
		ImGui::SetCurrentContext(_imgui);

		updateInput(dt);

		ImGui::SetCurrentContext(nullptr);
	}

	void ImguiAppComponentImpl::render(bgfx::ViewId viewId)
	{
		ImGui::SetCurrentContext(_imgui);

		beginFrame();
		_renderer.imguiRender();
		endFrame(viewId);

		ImGui::SetCurrentContext(nullptr);
	}

	void ImguiAppComponentImpl::updateInput(float dt)
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
		
		auto& scroll = mouse.getScroll();
		io.AddMouseWheelEvent(scroll.x, scroll.y);

		auto& kb = input.getKeyboard();
		uint8_t modifiers = kb.getModifiers();
		io.AddKeyEvent(ImGuiMod_Shift, 0 != (modifiers & KeyboardModifiers::Shift));
		io.AddKeyEvent(ImGuiMod_Ctrl, 0 != (modifiers & KeyboardModifiers::Ctrl));
		io.AddKeyEvent(ImGuiMod_Alt, 0 != (modifiers & KeyboardModifiers::Alt));
		io.AddKeyEvent(ImGuiMod_Super, 0 != (modifiers & KeyboardModifiers::Meta));
		for (int32_t i = 0; i < _imguiKeyboardMap.size(); ++i)
		{
			io.AddKeyEvent(_imguiKeyboardMap[i], kb.getKey(KeyboardKey(i)));
			io.SetKeyEventNativeData(_imguiKeyboardMap[i], 0, 0, i);
		}
		for (auto& gamepad : input.getGamepads())
		{
			for (int32_t i = 0; i < _imguiGamepadMap.size(); ++i)
			{
				io.AddKeyEvent(_imguiGamepadMap[i], gamepad.getButton(GamepadButton(i)));
				io.SetKeyEventNativeData(_imguiGamepadMap[i], 0, 0, i);
			}
		}
	}

	void ImguiAppComponentImpl::beginFrame()
	{
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImguiAppComponentImpl::endFrame(bgfx::ViewId viewId)
	{
		ImGui::Render();
		render(viewId, _texture, ImGui::GetDrawData());
	}

	ImguiAppComponent::ImguiAppComponent(IImguiRenderer& renderer, float fontSize) noexcept
		: _impl(std::make_unique<ImguiAppComponentImpl>(renderer, fontSize))
	{
	}

	void ImguiAppComponent::init(App& app)
	{
		_impl->init(app);
	}

	void ImguiAppComponent::shutdown()
	{
		_impl->shutdown();
	}

	void ImguiAppComponent::updateLogic(float dt)
	{
		_impl->updateLogic(dt);
	}

	bgfx::ViewId ImguiAppComponent::render(bgfx::ViewId viewId)
	{
		_impl->render(viewId);
		return ++viewId;
	}
}

namespace ImGui
{
	void PushFont(Font::Enum font)
	{
		// PushFont(darmok::s_ctx._font[font]);
	}

	void PushEnabled(bool enabled)
	{
		extern void PushItemFlag(int option, bool enabled);
		PushItemFlag(ImGuiItemFlags_Disabled, !enabled);
		PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (enabled ? 1.0f : 0.5f));
	}

	void PopEnabled()
	{
		extern void PopItemFlag();
		PopItemFlag();
		PopStyleVar();
	}

} // namespace ImGui

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: 'int rect_width_compare(const void*, const void*)' defined but not used
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
// #define STBTT_malloc(size, userData) darmok::imguiMemAlloc(size, userData)
// #define STBTT_free(ptr, userData) darmok::imguiMemFree(ptr, userData)
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
BX_PRAGMA_DIAGNOSTIC_POP();