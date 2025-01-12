

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/data.hpp>
#include <darmok/window.hpp>
#include <darmok/input.hpp>
#include <bgfx/bgfx.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtx/string_cast.hpp>

namespace
{
	using namespace darmok;

	struct VideoModeState final
	{
		void onWindowVideoMode(const VideoMode& mode)
		{
			_screenMode = (int)mode.screenMode;
			_monitor = mode.monitor;
			_resolution = -1;

			int i = 0;
			for (auto& elm : _info.modes)
			{
				if (mode.monitor == elm.monitor && mode.screenMode == elm.screenMode)
				{
					if (mode == elm)
					{
						_resolution = i;
						break;
					}
					i++;
				}
			}
		}

		void update(const VideoModeInfo& info)
		{
			_info = info;
			_videoModeNames.clear();
			for (auto& mode : info.modes)
			{
				_videoModeNames.push_back(mode.toShortString());
			}
		}

		void imguiRender(Window& win)
		{
			auto monitorCount = _info.monitors.size();
			if (monitorCount > 1)
			{
				ImGui::Combo("Monitor", &_monitor, getMonitorName, this, monitorCount);
			}
			if (!_screenModes.empty())
			{
				if (ImGui::Combo("Screen Mode", &_screenMode, &_screenModes.front(), _screenModes.size()))
				{
					_resolution = 0;
				}
			}
			auto resCount = getResolutionCount();
			if (resCount > 0)
			{
				ImGui::Combo("Resolution", &_resolution, getResolutionName, this, resCount);
			}
			if (ImGui::Button("Apply Video Mode"))
			{
				apply(win);
			}
		}

	private:

		VideoModeInfo _info;
		int _monitor = 0;
		int _resolution = 0;
		int _screenMode = 0;
		const std::vector<const char*> _screenModes = { "Windowed", "Fullscreen", "Windowed Fullscreen" };
		std::vector<std::string> _videoModeNames;

		bool apply(Window& win)
		{
			auto idx = getVideoModeIndex(_resolution);
			if (!idx)
			{
				return false;
			}

			win.requestVideoMode(_info.modes[idx.value()]);
			return true;
		}

		static bool getMonitorName(void* data, int idx, const char** outText)
		{
			if (data == nullptr)
			{
				return false;
			}
			auto self = (VideoModeState*)data;
			if (self->_info.monitors.size() <= idx)
			{
				return false;
			}
			*outText = self->_info.monitors[idx].name.c_str();
			return true;
		};

		bool isSelectableMode(const VideoMode& mode) const
		{
			if (mode.monitor >= 0 && mode.monitor != _monitor)
			{
				return false;
			}
			if (mode.screenMode != (WindowScreenMode)_screenMode)
			{
				return false;
			}
			return true;
		}

		int getResolutionCount()
		{
			int count = 0;
			for (auto& mode : _info.modes)
			{
				if (isSelectableMode(mode))
				{
					count++;
				}
			}
			return count;
		}

		std::optional<size_t> getVideoModeIndex(int idx) const
		{
			int j = 0;
			auto screenMode = (WindowScreenMode)_screenMode;
			for (size_t i = 0; i < _info.modes.size(); i++)
			{
				if (isSelectableMode(_info.modes[i]) && j++ == idx)
				{
					return i;
				}
			}
			return std::nullopt;
		}

		static bool getResolutionName(void* data, int idx, const char** outText)
		{
			if (data == nullptr)
			{
				return false;
			}
			auto self = (VideoModeState*)data;
			auto modeIdx = self->getVideoModeIndex(idx);
			if (!modeIdx)
			{
				return false;
			}
			*outText = self->_videoModeNames[modeIdx.value()].c_str();
			return true;
		};
	};

	class ImguiSampleAppDelegate final : public IAppDelegate, public IImguiRenderer, IWindowListener
	{
	public:
		ImguiSampleAppDelegate(App& app)
			: _app(app)
		{
		}

		void init() override
		{
			// Enable debug text.
			_app.setDebugFlag(BGFX_DEBUG_TEXT);
			auto& imgui = _app.addComponent<darmok::ImguiAppComponent>(*this);
			// the current imgui context is static, we have to do this if darmok is dynamically linked
			// to set the static in this part
			ImGui::SetCurrentContext(imgui.getContext());

			_videoModeState.update(_app.getWindow().getVideoModeInfo());
			_videoModeState.onWindowVideoMode(_app.getWindow().getVideoMode());
			_app.getWindow().addListener(*this);
		}

		void shutdown() override
		{
			_app.getWindow().removeListener(*this);
		}

		void imguiRender() override
		{
			ImGui::TextWrapped("lala");
			ImGui::InputText("text", &_text);

			ImGui::Spacing();

			_videoModeState.imguiRender(_app.getWindow());

			ImGui::Spacing();

			if (ImGui::Button("Reset max deltas"))
			{
				_mouseVelocityMax = glm::vec2(0);
				_mouseScrollMax = glm::vec2(0);
			}
		}

		void onWindowVideoMode(const VideoMode& mode) override
		{
			_videoModeState.onWindowVideoMode(mode);
		}

		void render() const override
		{
			const bgfx::Stats* stats = bgfx::getStats();

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

			bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
			bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

			bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
				, stats->width
				, stats->height
				, stats->textWidth
				, stats->textHeight
			);

			bgfx::dbgTextPrintf(0, 3, 0x0f, "Window: mode %s size %s"
				, _app.getWindow().getVideoMode().toString().c_str()
				, glm::to_string(_app.getWindow().getSize()).c_str()
			);
			bgfx::dbgTextPrintf(0, 4, 0x0f, "Mouse: position %s velocity %s velocity max %s"
				, glm::to_string(_app.getInput().getMouse().getPosition()).c_str()
				, glm::to_string(_mouseVelocity).c_str()
				, glm::to_string(_mouseVelocityMax).c_str()
			);
			bgfx::dbgTextPrintf(0, 5, 0x0f, "Mouse scroll: %s max %s"
				, glm::to_string(_mouseScroll).c_str()
				, glm::to_string(_mouseScrollMax).c_str()
			);
			bgfx::dbgTextPrintf(0, 6, 0x0f, "Gamepad Sticks: %s %s"
				, glm::to_string(_app.getInput().getGamepad()->getStick(GamepadStick::Left)).c_str()
				, glm::to_string(_app.getInput().getGamepad()->getStick(GamepadStick::Right)).c_str()
			);
		}

		void update(float deltaTime) override
		{
			_mouseVelocity = _app.getInput().getMouse().getVelocity();
			updateMaxVec(_mouseVelocity, _mouseVelocityMax);
			_mouseScroll = _app.getInput().getMouse().getScroll();
			updateMaxVec(_mouseScroll, _mouseScrollMax);
		}

	private:
		App& _app;
		std::string _text = "hello darmok!";
		VideoModeState _videoModeState;
		glm::vec2 _mouseVelocity = glm::vec2(0);
		glm::vec2 _mouseVelocityMax = glm::vec2(0);
		glm::vec2 _mouseScroll = glm::vec2(0);
		glm::vec2 _mouseScrollMax = glm::vec2(0);

		static void updateMaxVec(const glm::vec2& v, glm::vec2& max)
		{
			auto x = std::abs(v.x);
			if (max.x < x)
			{
				max.x = x;
			}
			auto y = std::abs(v.y);
			if (max.y < y)
			{
				max.y = y;
			}
		}
	};

}

DARMOK_RUN_APP(ImguiSampleAppDelegate);
