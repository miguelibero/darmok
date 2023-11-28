#include "imgui.hpp"
#include "bgfx.hpp"
#include <darmok/input.hpp>
#include <darmok/app.hpp>

#include <dear-imgui/imgui.h>
#include <dear-imgui/imgui_internal.h>
#include <bx/timer.h>
#include <bx/allocator.h>
#include <bgfx/embedded_shader.h>

// shaders (using the ones in bgfx examples)
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
	static const bgfx::EmbeddedShader s_embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
		BGFX_EMBEDDED_SHADER(vs_imgui_image),
		BGFX_EMBEDDED_SHADER(fs_imgui_image),

		BGFX_EMBEDDED_SHADER_END()
	};

	struct FontRangeMerge
	{
		const void* data;
		size_t      size;
		ImWchar     ranges[3];
	};

	static FontRangeMerge s_fontRangeMerge[] =
	{
		{ s_iconsKenneyTtf,      sizeof(s_iconsKenneyTtf),      { ICON_MIN_KI, ICON_MAX_KI, 0 } },
		{ s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), { ICON_MIN_FA, ICON_MAX_FA, 0 } },
	};

	static void* memAlloc(size_t size, void* userData);
	static void memFree(void* ptr, void* userData);

	struct ImguiContext
	{
		void render(ImDrawData* _drawData)
		{
			// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
			int fb_width = (int)(_drawData->DisplaySize.x * _drawData->FramebufferScale.x);
			int fb_height = (int)(_drawData->DisplaySize.y * _drawData->FramebufferScale.y);
			if (fb_width <= 0 || fb_height <= 0)
				return;

			bgfx::setViewName(_viewId, "ImGui");
			bgfx::setViewMode(_viewId, bgfx::ViewMode::Sequential);

			const bgfx::Caps* caps = bgfx::getCaps();
			{
				float ortho[16];
				float x = _drawData->DisplayPos.x;
				float y = _drawData->DisplayPos.y;
				float width = _drawData->DisplaySize.x;
				float height = _drawData->DisplaySize.y;

				bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
				bgfx::setViewTransform(_viewId, NULL, ortho);
				bgfx::setViewRect(_viewId, 0, 0, uint16_t(width), uint16_t(height));
			}

			const ImVec2 clipPos = _drawData->DisplayPos;       // (0,0) unless using multi-viewports
			const ImVec2 clipScale = _drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

			// Render command lists
			for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
			{
				bgfx::TransientVertexBuffer tvb;
				bgfx::TransientIndexBuffer tib;

				const ImDrawList* drawList = _drawData->CmdLists[ii];
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

						bgfx::TextureHandle th = _texture;
						bgfx::ProgramHandle program = _program;

						if (NULL != cmd->TextureId)
						{
							union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
							state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
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
							encoder->submit(_viewId, program);
						}
					}
				}

				bgfx::end(encoder);
			}
		}

		void create(float fontSize)
		{
			IMGUI_CHECKVERSION();

			_viewId = 255;
			_lastScroll = 0;
			_last = bx::getHPCounter();

			ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

			_imgui = ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2(1280.0f, 720.0f);
			io.DeltaTime = 1.0f / 60.0f;
			io.IniFilename = NULL;

			setupStyle(true);

			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

			for (int32_t i = 0; i < (int32_t)Key::Count; ++i)
			{
				_keyMap[i] = ImGuiKey_None;
			}

			_keyMap[to_underlying(Key::Esc)] = ImGuiKey_Escape;
			_keyMap[to_underlying(Key::Return)] = ImGuiKey_Enter;
			_keyMap[to_underlying(Key::Tab)] = ImGuiKey_Tab;
			_keyMap[to_underlying(Key::Space)] = ImGuiKey_Space;
			_keyMap[to_underlying(Key::Backspace)] = ImGuiKey_Backspace;
			_keyMap[to_underlying(Key::Up)] = ImGuiKey_UpArrow;
			_keyMap[to_underlying(Key::Down)] = ImGuiKey_DownArrow;
			_keyMap[to_underlying(Key::Left)] = ImGuiKey_LeftArrow;
			_keyMap[to_underlying(Key::Right)] = ImGuiKey_RightArrow;
			_keyMap[to_underlying(Key::Insert)] = ImGuiKey_Insert;
			_keyMap[to_underlying(Key::Delete)] = ImGuiKey_Delete;
			_keyMap[to_underlying(Key::Home)] = ImGuiKey_Home;
			_keyMap[to_underlying(Key::End)] = ImGuiKey_End;
			_keyMap[to_underlying(Key::PageUp)] = ImGuiKey_PageUp;
			_keyMap[to_underlying(Key::PageDown)] = ImGuiKey_PageDown;
			_keyMap[to_underlying(Key::Print)] = ImGuiKey_PrintScreen;
			_keyMap[to_underlying(Key::Plus)] = ImGuiKey_Equal;
			_keyMap[to_underlying(Key::Minus)] = ImGuiKey_Minus;
			_keyMap[to_underlying(Key::LeftBracket)] = ImGuiKey_LeftBracket;
			_keyMap[to_underlying(Key::RightBracket)] = ImGuiKey_RightBracket;
			_keyMap[to_underlying(Key::Semicolon)] = ImGuiKey_Semicolon;
			_keyMap[to_underlying(Key::Quote)] = ImGuiKey_Apostrophe;
			_keyMap[to_underlying(Key::Comma)] = ImGuiKey_Comma;
			_keyMap[to_underlying(Key::Period)] = ImGuiKey_Period;
			_keyMap[to_underlying(Key::Slash)] = ImGuiKey_Slash;
			_keyMap[to_underlying(Key::Backslash)] = ImGuiKey_Backslash;
			_keyMap[to_underlying(Key::Tilde)] = ImGuiKey_GraveAccent;
			_keyMap[to_underlying(Key::F1)] = ImGuiKey_F1;
			_keyMap[to_underlying(Key::F2)] = ImGuiKey_F2;
			_keyMap[to_underlying(Key::F3)] = ImGuiKey_F3;
			_keyMap[to_underlying(Key::F4)] = ImGuiKey_F4;
			_keyMap[to_underlying(Key::F5)] = ImGuiKey_F5;
			_keyMap[to_underlying(Key::F6)] = ImGuiKey_F6;
			_keyMap[to_underlying(Key::F7)] = ImGuiKey_F7;
			_keyMap[to_underlying(Key::F8)] = ImGuiKey_F8;
			_keyMap[to_underlying(Key::F9)] = ImGuiKey_F9;
			_keyMap[to_underlying(Key::F10)] = ImGuiKey_F10;
			_keyMap[to_underlying(Key::F11)] = ImGuiKey_F11;
			_keyMap[to_underlying(Key::F12)] = ImGuiKey_F12;
			_keyMap[to_underlying(Key::NumPad0)] = ImGuiKey_Keypad0;
			_keyMap[to_underlying(Key::NumPad1)] = ImGuiKey_Keypad1;
			_keyMap[to_underlying(Key::NumPad2)] = ImGuiKey_Keypad2;
			_keyMap[to_underlying(Key::NumPad3)] = ImGuiKey_Keypad3;
			_keyMap[to_underlying(Key::NumPad4)] = ImGuiKey_Keypad4;
			_keyMap[to_underlying(Key::NumPad5)] = ImGuiKey_Keypad5;
			_keyMap[to_underlying(Key::NumPad6)] = ImGuiKey_Keypad6;
			_keyMap[to_underlying(Key::NumPad7)] = ImGuiKey_Keypad7;
			_keyMap[to_underlying(Key::NumPad8)] = ImGuiKey_Keypad8;
			_keyMap[to_underlying(Key::NumPad9)] = ImGuiKey_Keypad9;
			_keyMap[to_underlying(Key::Key0)] = ImGuiKey_0;
			_keyMap[to_underlying(Key::Key1)] = ImGuiKey_1;
			_keyMap[to_underlying(Key::Key2)] = ImGuiKey_2;
			_keyMap[to_underlying(Key::Key3)] = ImGuiKey_3;
			_keyMap[to_underlying(Key::Key4)] = ImGuiKey_4;
			_keyMap[to_underlying(Key::Key5)] = ImGuiKey_5;
			_keyMap[to_underlying(Key::Key6)] = ImGuiKey_6;
			_keyMap[to_underlying(Key::Key7)] = ImGuiKey_7;
			_keyMap[to_underlying(Key::Key8)] = ImGuiKey_8;
			_keyMap[to_underlying(Key::Key9)] = ImGuiKey_9;
			_keyMap[to_underlying(Key::KeyA)] = ImGuiKey_A;
			_keyMap[to_underlying(Key::KeyB)] = ImGuiKey_B;
			_keyMap[to_underlying(Key::KeyC)] = ImGuiKey_C;
			_keyMap[to_underlying(Key::KeyD)] = ImGuiKey_D;
			_keyMap[to_underlying(Key::KeyE)] = ImGuiKey_E;
			_keyMap[to_underlying(Key::KeyF)] = ImGuiKey_F;
			_keyMap[to_underlying(Key::KeyG)] = ImGuiKey_G;
			_keyMap[to_underlying(Key::KeyH)] = ImGuiKey_H;
			_keyMap[to_underlying(Key::KeyI)] = ImGuiKey_I;
			_keyMap[to_underlying(Key::KeyJ)] = ImGuiKey_J;
			_keyMap[to_underlying(Key::KeyK)] = ImGuiKey_K;
			_keyMap[to_underlying(Key::KeyL)] = ImGuiKey_L;
			_keyMap[to_underlying(Key::KeyM)] = ImGuiKey_M;
			_keyMap[to_underlying(Key::KeyN)] = ImGuiKey_N;
			_keyMap[to_underlying(Key::KeyO)] = ImGuiKey_O;
			_keyMap[to_underlying(Key::KeyP)] = ImGuiKey_P;
			_keyMap[to_underlying(Key::KeyQ)] = ImGuiKey_Q;
			_keyMap[to_underlying(Key::KeyR)] = ImGuiKey_R;
			_keyMap[to_underlying(Key::KeyS)] = ImGuiKey_S;
			_keyMap[to_underlying(Key::KeyT)] = ImGuiKey_T;
			_keyMap[to_underlying(Key::KeyU)] = ImGuiKey_U;
			_keyMap[to_underlying(Key::KeyV)] = ImGuiKey_V;
			_keyMap[to_underlying(Key::KeyW)] = ImGuiKey_W;
			_keyMap[to_underlying(Key::KeyX)] = ImGuiKey_X;
			_keyMap[to_underlying(Key::KeyY)] = ImGuiKey_Y;
			_keyMap[to_underlying(Key::KeyZ)] = ImGuiKey_Z;

			io.ConfigFlags |= 0
				| ImGuiConfigFlags_NavEnableGamepad
				| ImGuiConfigFlags_NavEnableKeyboard
				;

			_keyMap[to_underlying(Key::GamepadStart)] = ImGuiKey_GamepadStart;
			_keyMap[to_underlying(Key::GamepadBack)] = ImGuiKey_GamepadBack;
			_keyMap[to_underlying(Key::GamepadY)] = ImGuiKey_GamepadFaceUp;
			_keyMap[to_underlying(Key::GamepadA)] = ImGuiKey_GamepadFaceDown;
			_keyMap[to_underlying(Key::GamepadX)] = ImGuiKey_GamepadFaceLeft;
			_keyMap[to_underlying(Key::GamepadB)] = ImGuiKey_GamepadFaceRight;
			_keyMap[to_underlying(Key::GamepadUp)] = ImGuiKey_GamepadDpadUp;
			_keyMap[to_underlying(Key::GamepadDown)] = ImGuiKey_GamepadDpadDown;
			_keyMap[to_underlying(Key::GamepadLeft)] = ImGuiKey_GamepadDpadLeft;
			_keyMap[to_underlying(Key::GamepadRight)] = ImGuiKey_GamepadDpadRight;
			_keyMap[to_underlying(Key::GamepadShoulderL)] = ImGuiKey_GamepadL1;
			_keyMap[to_underlying(Key::GamepadShoulderR)] = ImGuiKey_GamepadR1;
			_keyMap[to_underlying(Key::GamepadThumbL)] = ImGuiKey_GamepadL3;
			_keyMap[to_underlying(Key::GamepadThumbR)] = ImGuiKey_GamepadR3;

			bgfx::RendererType::Enum type = bgfx::getRendererType();
			_program = bgfx::createProgram(
				bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
				, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
				, true
			);

			_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
			_imageProgram = bgfx::createProgram(
				bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
				, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
				, true
			);

			_layout
				.begin()
				.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();

			_uniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

			uint8_t* data;
			int32_t width;
			int32_t height;
			{
				ImFontConfig config;
				config.FontDataOwnedByAtlas = false;
				config.MergeMode = false;
				//			config.MergeGlyphCenterV = true;

				const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
				_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf), fontSize, &config, ranges);
				_font[ImGui::Font::Mono] = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), fontSize - 3.0f, &config, ranges);

				config.MergeMode = true;
				config.DstFont = _font[ImGui::Font::Regular];

				for (uint32_t i = 0; i< BX_COUNTOF(s_fontRangeMerge); ++i)
				{
					const FontRangeMerge& frm = s_fontRangeMerge[i];

					io.Fonts->AddFontFromMemoryTTF((void*)frm.data
						, (int)frm.size
						, fontSize - 3.0f
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

			ImGui::InitDockContext();
		}

		void destroy()
		{
			ImGui::ShutdownDockContext();
			ImGui::DestroyContext(_imgui);

			bgfx::destroy(_uniform);
			bgfx::destroy(_texture);

			bgfx::destroy(_imageLodEnabled);
			bgfx::destroy(_imageProgram);
			bgfx::destroy(_program);
		}

		void setupStyle(bool _dark)
		{
			// Doug Binks' darl color scheme
			// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
			ImGuiStyle& style = ImGui::GetStyle();
			if (_dark)
			{
				ImGui::StyleColorsDark(&style);
			}
			else
			{
				ImGui::StyleColorsLight(&style);
			}

			style.FrameRounding = 4.0f;
			style.WindowBorderSize = 0.0f;
		}

		void beginFrame(WindowHandle handle = Window::DefaultHandle, Utf8Char inputChar = {}, bgfx::ViewId viewId = 255)
		{
			_viewId = viewId;

			ImGuiIO& io = ImGui::GetIO();
			if (inputChar.len >= 0)
			{
				io.AddInputCharacter(inputChar.data);
			}

			auto& win = Window::get(handle);
			auto& size = win.getSize();
			io.DisplaySize = ImVec2((float)size.width, (float)size.height);

			const int64_t now = bx::getHPCounter();
			const int64_t frameTime = now - _last;
			_last = now;
			const double freq = double(bx::getHPFrequency());
			io.DeltaTime = float(frameTime / freq);

			auto& buttons = inputGetMouseButtons();
			auto& pos = inputGetAbsoluteMouse();
			io.AddMousePosEvent((float)pos.x, (float)pos.y);
			io.AddMouseButtonEvent(ImGuiMouseButton_Left, buttons[to_underlying(MouseButton::Left)]);
			io.AddMouseButtonEvent(ImGuiMouseButton_Right, buttons[to_underlying(MouseButton::Right)]);
			io.AddMouseButtonEvent(ImGuiMouseButton_Middle, buttons[to_underlying(MouseButton::Middle)]);
			io.AddMouseWheelEvent(0.0f, (float)(pos.z - _lastScroll));
			_lastScroll = pos.z;

			uint8_t modifiers = inputGetModifiersState();
			io.AddKeyEvent(ImGuiMod_Shift, 0 != (modifiers & KeyModifiers::Shift));
			io.AddKeyEvent(ImGuiMod_Ctrl, 0 != (modifiers & KeyModifiers::Ctrl));
			io.AddKeyEvent(ImGuiMod_Alt, 0 != (modifiers & KeyModifiers::Alt));
			io.AddKeyEvent(ImGuiMod_Super, 0 != (modifiers & KeyModifiers::Meta));
			for (int32_t i = 0; i < (int32_t)Key::Count; ++i)
			{
				io.AddKeyEvent(_keyMap[i], inputGetKeyState(Key(i)));
				io.SetKeyEventNativeData(_keyMap[i], 0, 0, i);
			}

			ImGui::NewFrame();

			ImGuizmo::BeginFrame();
		}

		void endFrame()
		{
			ImGui::Render();
			render(ImGui::GetDrawData());
		}

		ImGuiContext* _imgui;
		bgfx::VertexLayout  _layout;
		bgfx::ProgramHandle _program;
		bgfx::ProgramHandle _imageProgram;
		bgfx::TextureHandle _texture;
		bgfx::UniformHandle _uniform;
		bgfx::UniformHandle _imageLodEnabled;
		std::array<ImFont*, ImGui::Font::Count> _font;
		int64_t _last;
		int32_t _lastScroll;
		bgfx::ViewId _viewId;
		std::array<ImGuiKey, to_underlying(Key::Count)> _keyMap;
	};

	static ImguiContext s_ctx;

	static void* memAlloc(size_t size, void* userData)
	{
		BX_UNUSED(userData);
		return bx::alloc(&getAllocator(), size);
	}

	static void memFree(void* ptr, void* userData)
	{
		BX_UNUSED(userData);
		bx::free(&getAllocator(), ptr);
	}

	void imguiCreate(float fontSize)
	{
		s_ctx.create(fontSize);
	}

	void imguiDestroy()
	{
		s_ctx.destroy();
	}

	void imguiBeginFrame(WindowHandle window, Utf8Char inputChar, bgfx::ViewId viewId)
	{
		s_ctx.beginFrame(window, inputChar, viewId);
	}

	void imguiEndFrame()
	{
		s_ctx.endFrame();
	}
}

namespace ImGui
{
	void PushFont(Font::Enum font)
	{
		PushFont(darmok::s_ctx._font[font]);
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
#define STBTT_malloc(size, userData) darmok::memAlloc(size, userData)
#define STBTT_free(ptr, userData) darmok::memFree(ptr, userData)
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
BX_PRAGMA_DIAGNOSTIC_POP();