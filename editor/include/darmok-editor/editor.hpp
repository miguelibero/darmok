#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>

#include <bx/bx.h>
#include <imgui.h>
#include <google/protobuf/any.pb.h>

namespace darmok::editor
{
    class ObjectEditorContainer;
    class EditorApp;

    class BX_NO_VTABLE IObjectEditor
    {
    public:
        using Message = google::protobuf::Message;
        using RenderResult = expected<bool, std::string>;
        virtual ~IObjectEditor() = default;
        virtual expected<void, std::string> init(EditorApp& app, ObjectEditorContainer& container) noexcept { return {}; }
        virtual expected<void, std::string> shutdown() noexcept { return {}; }
        virtual std::string getTitle() const noexcept = 0;
        virtual std::string getObjectTypeUrl() const noexcept = 0;
        virtual bool canRender(const Message& msg) const noexcept = 0;
        virtual RenderResult render(Message& msg) noexcept { return false; }
    };

    class EditorProject;

    class BaseObjectEditor : public IObjectEditor
    {
    public:
        using Any = google::protobuf::Any;

        expected<void, std::string> init(EditorApp& app, ObjectEditorContainer& container) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;

    protected:
        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
		EditorApp& getApp() noexcept;
        const EditorApp& getApp() const noexcept;
        Scene& getScene() noexcept;
        const Scene& getScene() const noexcept;
        SceneDefinitionWrapper& getSceneDefinition() noexcept;
        const SceneDefinitionWrapper& getSceneDefinition() const noexcept;
		IComponentLoadContext& getComponentLoadContext() noexcept;

        expected<void, std::string> reloadAsset(const std::filesystem::path& path) noexcept;
        expected<void, std::string> removeAsset(const std::filesystem::path& path) noexcept;
        RenderResult renderChild(google::protobuf::Message& msg, bool withTitle = false) noexcept;
        EntityId getEntityId(const Any& anyComp) const noexcept;
        Entity getEntity(EntityId entityId) const noexcept;
        std::optional<std::filesystem::path> getAssetPath(const Any& anyAsset) const noexcept;
    };

    template<typename T>
    class ObjectEditor : public BaseObjectEditor
    {
    public:
        using Object = T;
		using Any = BaseObjectEditor::Any;
    protected:
        virtual RenderResult renderType(T& obj) noexcept { return false; };
		virtual RenderResult beforeRenderAny(Any& any, T& obj) noexcept { return false; }
        virtual RenderResult afterRenderAny(Any& any, T& obj, bool changed) noexcept { return false; }

    public:

        std::string getTitle() const noexcept override
        {
            return T::descriptor()->name();
        }

        std::string getObjectTypeUrl() const noexcept override
        {
            return protobuf::getTypeUrl<T>();
        }

        bool canRender(const Message& msg) const noexcept override
        {
            return protobuf::getTypeUrl(msg) == getObjectTypeUrl();
        }

        RenderResult render(Message& msg) noexcept override
        {
            if (!protobuf::isAny(msg))
            {
				return renderType(static_cast<T&>(msg));
            }

            T obj;
            auto& any = static_cast<Any&>(msg);
            if (!any.UnpackTo(&obj))
            {
                return unexpected<std::string>{"failed to unpack any"};
            }

            auto changed = false;
			auto result = beforeRenderAny(any, obj);
            if(!result)
            {
                return unexpected{ std::move(result).error() };
			}
            changed |= *result;
            result = renderType(obj);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            changed |= *result;
			std::optional<Any> oldAny;
            if (changed)
            {
				oldAny.emplace().CopyFrom(any);
                if (!any.PackFrom(obj))
                {
                    return unexpected<std::string>{"failed to pack any"};
                }
            }
            result = afterRenderAny(any, obj, changed);
            if (!result)
            {
                if(oldAny)
                {
                    any.CopyFrom(*oldAny);
				}
                return unexpected{ std::move(result).error() };
            }
            changed |= *result;
            return changed;
        }
    };

    template<typename T, typename Def = T::Definition>
        requires std::is_base_of_v<ISceneComponent, T>
    class BX_NO_VTABLE SceneComponentObjectEditor : public ObjectEditor<Def>
    {
    public:
        using Definition = Def;
        using RenderResult = ObjectEditor<Definition>::RenderResult;
        using Message = ObjectEditor<Definition>::Message;
        using Any = ObjectEditor<Definition>::Any;
    protected:

        RenderResult afterRenderAny(Any& any, Definition& def, bool changed) noexcept override
        {
            auto& scene = BaseObjectEditor::getScene();
            auto& sceneDef = BaseObjectEditor::getSceneDefinition();

            if (ImGui::Button("Remove Component"))
            {
                if (!sceneDef.template removeSceneComponent<Definition>())
                {
                    return unexpected{ "failed to remove scene definition component" };
                }
                if (!scene.template removeSceneComponent<T>())
                {
                    return unexpected{ "failed to remove scene component" };
                }
                changed = true;
            }
            else if (changed)
            {
                auto compResult = scene.template getOrAddSceneComponent<T>();
                if (!compResult)
                {
					return unexpected{ std::move(compResult).error() };
                }
				auto& comp = compResult.value().get();
                auto loadResult = SceneArchive::loadComponent(comp, def, BaseObjectEditor::getComponentLoadContext());
                if (!loadResult)
                {
                    return unexpected{ std::move(loadResult).error() };
                }
            }
            return ObjectEditor<Definition>::afterRenderAny(any, def, changed);
        }
    };

    template<typename T, typename Def = T::Definition>
    class BX_NO_VTABLE EntityComponentObjectEditor : public ObjectEditor<Def>
    {
    public:
        using Definition = Def;
        using RenderResult = ObjectEditor<Definition>::RenderResult;
        using Message = ObjectEditor<Definition>::Message;
        using Any = ObjectEditor<Definition>::Any;
    protected:
        std::optional<EntityId> _entityId;


        RenderResult beforeRenderAny(Any& any, Definition& def) noexcept override
        {
            _entityId = BaseObjectEditor::getEntityId(any);
            return ObjectEditor<Definition>::beforeRenderAny(any, def);
        }

        RenderResult afterRenderAny(Any& any, Definition& def, bool changed) noexcept override
        {
            if (!_entityId)
            {
                return unexpected{ "missing entity" };
            }
            auto& scene = BaseObjectEditor::getScene();
            auto& sceneDef = BaseObjectEditor::getSceneDefinition();
            auto entityId = *_entityId;
            auto entity = BaseObjectEditor::getEntity(entityId);

            if (ImGui::Button("Remove Component"))
            {
                if (!sceneDef.template removeComponent<Definition>(entityId))
                {
                    return unexpected{ "failed to remove entity component definition" };
                }
                if (!scene.template removeComponent<T>(entity))
                {
                    return unexpected{ "failed to remove entity component" };
                }
                changed = true;
            }
            else if (changed)
            {
                auto& comp = scene.template getOrAddComponent<T>(entity);
                auto result = SceneArchive::loadComponent(comp, def, BaseObjectEditor::getComponentLoadContext());
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
            }
            return ObjectEditor<Definition>::afterRenderAny(any, def, changed);
        }
    };

    template<typename T, typename Def = T::Definition>
        requires std::is_base_of_v<ICameraComponent, T>
    class BX_NO_VTABLE CameraComponentObjectEditor : public ObjectEditor<Def>
    {
    public:
        using Definition = Def;
        using RenderResult = ObjectEditor<Definition>::RenderResult;
        using Message = ObjectEditor<Definition>::Message;
        using Any = ObjectEditor<Definition>::Any;

    protected:
        std::optional<EntityId> _entityId;

        RenderResult beforeRenderAny(Any& any, Definition& def) noexcept override
        {
            _entityId = BaseObjectEditor::getEntityId(any);
            return ObjectEditor<Definition>::beforeRenderAny(any, def);
        }

        RenderResult afterRenderAny(Any& any, Definition& def, bool changed) noexcept override
        {
            if (!_entityId)
            {
                return unexpected{ "missing entity" };
            }
            auto& scene = BaseObjectEditor::getScene();
            auto entityId = *_entityId;
            auto entity = BaseObjectEditor::getEntity(entityId);
            auto optCam = scene.getComponent<Camera>(entity);
            if (!optCam)
            {
                return unexpected{ "missing camera component on entity" };
            }
			auto& cam = *optCam;
            auto& sceneDef = BaseObjectEditor::getSceneDefinition();

            if (ImGui::Button("Remove Component"))
            {
                if (auto camDef = sceneDef.getComponent<Camera::Definition>(entityId))
                {
                    CameraDefinitionWrapper camWrap{ *camDef };
                    if (!camWrap.removeComponent<Definition>())
                    {
                        return unexpected{ "failed to remove camera component definition" };
                    }
                }
                if (!cam.removeComponent<T>())
                {
                    return unexpected{ "failed to remove camera component" };
                }
                changed = true;
            }
            else if (changed)
            {
                auto compResult = cam.getOrAddComponent<T>();
                if (!compResult)
                {
                    return unexpected{ std::move(compResult).error() };
                }
                auto loadResult = SceneArchive::loadComponent(compResult.value().get(), def, BaseObjectEditor::getComponentLoadContext());
                if (!loadResult)
                {
                    return unexpected{ std::move(loadResult).error() };
                }
            }

            return ObjectEditor<Definition>::afterRenderAny(any, def, changed);
        }
    };

    template<typename T>
        requires std::is_base_of_v<google::protobuf::Message, T>
    class BX_NO_VTABLE AssetObjectEditor : public ObjectEditor<T>
    {
    protected:
        std::optional<std::filesystem::path> _path;
        using RenderResult = ObjectEditor<T>::RenderResult;
        using Message = ObjectEditor<T>::Message;
        using Any = ObjectEditor<T>::Any;

        RenderResult beforeRenderAny(Any& any, T& def) noexcept override
        {
            _path = BaseObjectEditor::getAssetPath(any);
            return ObjectEditor<T>::beforeRenderAny(any, def);
        }

        RenderResult afterRenderAny(Any& any, T& def, bool changed) noexcept override
        {
            if (!_path)
            {
                return unexpected{ "missing path" };
            }
            auto& path = *_path;
            if (ImGui::Button("Remove Asset"))
            {
                auto& sceneDef = BaseObjectEditor::getSceneDefinition();
                if (!sceneDef.removeAsset(path))
                {
                    return unexpected{ "failed to remove scene definition asset" };
                }
                changed = true;
            }
            else if (changed)
            {
                auto result = BaseObjectEditor::reloadAsset(path);
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
            }
            return ObjectEditor<T>::afterRenderAny(any, def, changed);
        }
    };

    class ObjectEditorContainer final
    {
    public:
        ObjectEditorContainer();

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> add(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = add(std::move(ptr));
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            return std::ref(ref);
        }

        using RenderResult = expected<bool, std::string>;

        RenderResult render(google::protobuf::Message& obj, bool withTitle = false) const noexcept;
        expected<void, std::string> add(std::unique_ptr<IObjectEditor> editor) noexcept;
        expected<void, std::string> init(EditorApp& app) noexcept;
        expected<void, std::string> shutdown() noexcept;

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<std::string, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}