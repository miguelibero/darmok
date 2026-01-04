#pragma once
#include <darmok/expected.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_chain.hpp>
#include <memory>
#include <string>

namespace darmok
{
    class App;
    class Scene;
    class Camera;
}

namespace darmok::editor
{
    class EditorApp;     

    class EditorPlayerView final
    {
    public:
        EditorPlayerView(EditorApp& editorApp, App& app) noexcept;
        expected<void, std::string> init(std::shared_ptr<Scene> scene) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRender() noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        static const std::string& getWindowName() noexcept;

        expected<void, std::string> play() noexcept;
        expected<void, std::string> stop() noexcept;
        void pause() noexcept;
        bool isPlaying() const noexcept;
        bool isPaused() const noexcept;

    private:
        EditorApp& _editorApp;
        App& _app;
        size_t _camIndex;
        std::shared_ptr<Scene> _scene;
        OptionalRef<Camera> _cam;
        bool _playing;

        static const std::string _windowName;

        expected<void, std::string> updateSize(const glm::uvec2& size) noexcept;
    };
}
