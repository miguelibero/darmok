#pragma once
#include <darmok/expected.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_chain.hpp>
#include <memory>
#include <string>

namespace darmok
{
    class Scene;
    class Camera;
}

namespace darmok::editor
{
    class EditorApp;        

    class EditorPlayerView final
    {
    public:
        EditorPlayerView(EditorApp& app) noexcept;
        expected<void, std::string> init(std::shared_ptr<Scene> scene) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRender() noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        static const std::string& getWindowName() noexcept;
    private:
        EditorApp& _app;
        size_t _camIndex;
        std::shared_ptr<Scene> _scene;
        OptionalRef<Camera> _cam;
        std::shared_ptr<FrameBuffer> _sceneBuffer;

        static const std::string _windowName;

        expected<void, std::string> updateSize(const glm::uvec2& size) noexcept;
    };
}
