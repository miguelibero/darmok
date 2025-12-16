#pragma once

#include <optional>
#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>
#include <darmok/window_fwd.hpp>
#include <darmok/collection.hpp>
#include <darmok/protobuf/freelook.pb.h>
#include <glm/gtc/quaternion.hpp>

namespace darmok
{
    class Window;
    class Camera;
    class IComponentLoadContext;

    class DARMOK_EXPORT BX_NO_VTABLE IFreelookListener
    {
    public:
        virtual ~IFreelookListener() = default;
        virtual entt::id_type getFreelookListenerType() const noexcept { return 0; };
        virtual expected<void, std::string> onFreelookEnable(bool enabled) noexcept = 0;
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeFreelookListener : public IFreelookListener
    {
    public:
        entt::id_type getFreelookListenerType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFreelookListenerFilter
    {
    public:
        virtual ~IFreelookListenerFilter() = default;
        virtual bool operator()(const IFreelookListener& listener) const = 0;
    };

    class DARMOK_EXPORT FreelookController final : public ITypeSceneComponent<FreelookController>, public ITypeInputEventListener<FreelookController>
    {
    public:
        using Definition = protobuf::FreelookController;
        FreelookController(OptionalRef<Camera> cam = nullptr, const Definition& def = createDefinition()) noexcept;
        
        expected<void, std::string> init(Scene& scene, App& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> onInputEvent(const std::string& tag) noexcept override;
        expected<void, std::string> setEnabled(bool enabled) noexcept;
        bool isEnabled() const noexcept;
        FreelookController& addListener(IFreelookListener& listener) noexcept;
        FreelookController& addListener(std::unique_ptr<IFreelookListener> listener) noexcept;
        bool removeListener(const IFreelookListener& listener) noexcept;
        size_t removeListeners(const IFreelookListenerFilter& filter) noexcept;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Input> _input;
        OptionalRef<Window> _win;
        OptionalRef<Camera> _cam;
        WindowCursorMode _winCursorMode;
        glm::quat _rot;
        glm::vec3 _pos;
        glm::vec3 _scale;
        bool _enabled;
        Definition _def;
        OwnRefCollection<IFreelookListener> _listeners;

        expected<void, std::string> doInit() noexcept;
    };
}