#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

#include <memory>
#include <string>


namespace darmok
{
    class App;
    class Scene;
    class Camera;
    class Mesh;
    class FrameBuffer;
}

namespace darmok::editor
{
    enum class SceneViewMouseMode
    {
        None,
        Look,
        Drag
    };

    enum class SceneViewTransformMode
    {
        Grab,
        Translate,
        Rotate,
        Scale
    };

    class BX_NO_VTABLE ISceneGizmo
    {
    public:
        virtual ~ISceneGizmo() = default;
        virtual expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept { return {}; }
        virtual expected<void, std::string> shutdown() noexcept { return {}; }
        virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
        virtual expected<void, std::string> render(bgfx::Encoder& encoder, Entity entity) noexcept { return {}; }
    };

    class TransformGizmo final : public ISceneGizmo
    {
    public:
        using Mode = SceneViewTransformMode;

        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> render(bgfx::Encoder& encoder, Entity entity) noexcept override;

        Mode getMode() const noexcept;
        void setMode(Mode mode) noexcept;

    private:
        Mode _mode;
        OptionalRef<App> _app;
        std::unique_ptr<Mesh> _transMesh;
        std::unique_ptr<Mesh> _rotMesh;
        std::unique_ptr<Mesh> _scaleMesh;
    };

    class EditorSceneView final
    {
    public:
        using TransformMode = SceneViewTransformMode;
        using MouseMode = SceneViewMouseMode;

        EditorSceneView(App& app) noexcept;
        expected<void, std::string> init(std::shared_ptr<Scene> scene, Camera& cam) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRender() noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        EditorSceneView& selectEntity(Entity entity) noexcept;

        MouseMode getMouseMode() const noexcept;
        static const std::string& getWindowName() noexcept;

        TransformMode getTransformMode() const noexcept;
        EditorSceneView& setTransformMode(TransformMode mode) noexcept;

    private:
        static const std::string _windowName;
        App& _app;
        std::shared_ptr<Scene> _scene;
        OptionalRef<Camera> _cam;
        MouseMode _mouseMode;
        bool _focused;
        std::shared_ptr<FrameBuffer> _sceneBuffer;
        Entity _selectedEntity;
        std::vector<std::unique_ptr<ISceneGizmo>> _gizmos;
        OptionalRef<TransformGizmo> _transformGizmo;

        expected<void, std::string> updateSize(const glm::uvec2& size) noexcept;
        expected<void, std::string> updateGizmos(float deltaTime) noexcept;
        expected<void, std::string> renderGizmos(bgfx::Encoder& encoder) noexcept;
        void updateCamera(float deltaTime) noexcept;
        expected<void, std::string> addGizmo(std::unique_ptr<ISceneGizmo> gizmo) noexcept;

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> addGizmo(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = addGizmo(std::move(ptr));
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            return std::ref(ref);
        }
    };
}