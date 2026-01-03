#include <darmok-editor/player_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <imgui.h>
namespace darmok::editor
{
    EditorPlayerView::EditorPlayerView(EditorApp& app) noexcept
        : _app{ app }
        , _camIndex{ 0 }
    {
    }

    expected<void, std::string> EditorPlayerView::init(std::shared_ptr<Scene> scene) noexcept
    {
        _scene = scene;
        return {};
    }

    expected<void, std::string> EditorPlayerView::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        _camIndex = 0;
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
                _sceneBuffer.reset();
            }
            auto sizeResult = updateSize(ImguiUtils::getAvailableContentRegion());
            if (!sizeResult)
            {
                return sizeResult;
            }
            if (_sceneBuffer)
            {
                ImguiUtils::drawBuffer(*_sceneBuffer);
            }
        }
        ImGui::End();
        return {};
    }

    expected<void, std::string> EditorPlayerView::update(float deltaTime) noexcept
    {
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
        if (_sceneBuffer && size == _sceneBuffer->getSize())
        {
            return {};
        }
        if (size.x <= 0.F || size.y <= 0.F)
        {
            _sceneBuffer.reset();
            _cam->setEnabled(false);
        }
        else
        {
            auto fbResult = FrameBuffer::load(size);
            if (!fbResult)
            {
                return unexpected{ std::move(fbResult).error() };
            }
            _sceneBuffer = std::make_shared<FrameBuffer>(std::move(fbResult).value());
            auto result = _cam->getRenderChain().setOutput(_sceneBuffer);
            if (!result)
            {
                return result;
            }
            _cam->setEnabled(true);
            _cam->setBaseViewport(Viewport{ size });
        }
        _app.requestRenderReset();
        return {};
    }
}