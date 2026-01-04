#include <darmok-editor/player_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <imgui.h>
namespace darmok::editor
{
    EditorPlayerView::EditorPlayerView(EditorApp& editorApp, App& app) noexcept
        : _editorApp{ editorApp }
        , _app{ app }
        , _camIndex{ 0 }
    {
    }

    expected<void, std::string> EditorPlayerView::init(std::shared_ptr<Scene> scene) noexcept
    {
        _scene = scene;
        _playing = false;
        return {};
    }

    expected<void, std::string> EditorPlayerView::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        _camIndex = 0;
        _playing = false;
        return {};
    }

    expected<void, std::string> EditorPlayerView::render() noexcept
    {
        if (ImGui::Begin(_windowName.c_str()))
        {
            std::vector<std::string> camNames;
            std::vector<Entity> camEntities;
            for (auto entity : _scene->getComponents<Camera>())
            {
                camNames.push_back(_scene->getComponent<Camera>(entity)->getName());
                camEntities.push_back(entity);
            }
            auto changed = ImguiUtils::drawListCombo("Cameras", _camIndex, camNames);
            if (!camEntities.empty() && (changed || !_cam))
            {
                _cam = _scene->getComponent<Camera>(camEntities.at(_camIndex));
            }
            auto sizeResult = updateSize(ImguiUtils::getAvailableContentRegion());
            if (!sizeResult)
            {
                return sizeResult;
            }
            if (_cam && _cam->getRenderOutput())
            {
                ImguiUtils::drawBuffer(*_cam->getRenderOutput());
            }
        }
        ImGui::End();

        ImGuiIO& io = ImGui::GetIO();

        expected<void, std::string> result;

        bool ctrl = io.KeyCtrl || io.KeySuper;
        bool shift = io.KeyShift;
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_P, false))
        {
            if (isPlaying())
            {
                if (shift)
                {
                    pause();
                }
                else
                {
                    result = stop();
                }
            }
            else
            {
                result = play();
            }
        }


        return result;
    }

    expected<void, std::string> EditorPlayerView::update(float deltaTime) noexcept
    {
        _playbackChange.reset();
        return {};
    }

    const std::string EditorPlayerView::_windowName{"Player"};

    const std::string& EditorPlayerView::getWindowName() noexcept
    {
        return _windowName;
    }

    expected<void, std::string> EditorPlayerView::updateSize(const glm::uvec2& size) noexcept
    {
        if (!_cam)
        {
            return {};
        }
        auto result = _cam->setRenderOutputSize(size);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        if (result.value())
        {
            _app.requestRenderReset();
        }
        return {};
    }

    expected<void, std::string> EditorPlayerView::play() noexcept
    {
        if (!_scene)
        {
            return unexpected{ "missing scene" };
        }
        auto& scene = *_scene;
        for (auto comp : scene.getSceneComponents())
        {
            auto result = comp.get().shutdown();
            if (!result)
            {
                return result;
            }
        }
        for (auto comp : scene.getSceneComponents())
        {
            auto result = comp.get().init(scene, _app);
            if (!result)
            {
                return result;
            }
        }
        scene.setPaused(false);
        _playing = true;
        _playbackChange = true;
        return {};
    }

    expected<void, std::string> EditorPlayerView::stop() noexcept
    {
        if (!_scene)
        {
            return unexpected{ "missing scene" };
        }
        _scene->setPaused(true);
        _playing = false;
        _playbackChange = false;
        return _editorApp.getProject().updateScene();
    }

    std::optional<bool> EditorPlayerView::getPlaybackChange() const noexcept
    {
        return _playbackChange;
    }

    void EditorPlayerView::pause() noexcept
    {
        if (_scene)
        {
            _scene->setPaused(!_scene->isPaused());
        }
    }

    bool EditorPlayerView::isPlaying() const noexcept
    {
        return _playing;
    }

    bool EditorPlayerView::isPaused() const noexcept
    {
        return !_scene || _scene->isPaused();
    }
}