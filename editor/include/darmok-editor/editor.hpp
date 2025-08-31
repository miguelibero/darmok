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
        virtual void init(EditorApp& app, ObjectEditorContainer& container) {}
        virtual void shutdown() {}
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

        void init(EditorApp& app, ObjectEditorContainer& container) override;
		void shutdown() override;
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
        RenderResult renderChild(google::protobuf::Message& msg) noexcept;
        std::optional<Entity> getEntity(const Any& anyComp) const noexcept;
        std::optional<std::filesystem::path> getAssetPath(const Any& anyAsset) const noexcept;
    };

    template<typename T>
    class ObjectEditor : public BaseObjectEditor
    {
    public:
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

    template<typename T>
    class BX_NO_VTABLE ComponentObjectEditor : public ObjectEditor<typename T::Definition>
    {
    protected:
        std::optional<Entity> _entity;
		using Definition = typename T::Definition;
		using RenderResult = ObjectEditor<Definition>::RenderResult;
        using Message = ObjectEditor<Definition>::Message;
        using Any = ObjectEditor<Definition>::Any;

        RenderResult beforeRenderAny(Any& any, Definition& def) noexcept override
        {
            _entity = BaseObjectEditor::getEntity(any);
            return ObjectEditor<Definition>::beforeRenderAny(any, def);
        }

        RenderResult afterRenderAny(Any& any, Definition& def, bool changed) noexcept override
        {
            if (_entity)
            {
                auto& scene = BaseObjectEditor::getScene();
                auto& sceneDef = BaseObjectEditor::getSceneDefinition();
                auto entity = *_entity;
                if (ImGui::Button("Remove Component"))
                {
                    if (!sceneDef.removeComponent<Definition>(entity))
                    {
                        return unexpected{ "failed to remove scene definition component" };
                    }
                    if (!scene.removeComponent<T>(entity))
                    {
                        return unexpected{ "failed to remove scene component" };
                    }
                    changed = true;
                }
                else if (changed)
                {
                    auto& comp = scene.getOrAddComponent<T>(entity);
                    auto result = comp.load(def, BaseObjectEditor::getComponentLoadContext());
                    if (!result)
                    {
                        return unexpected{ std::move(result).error() };
                    }
                }
            }
            return ObjectEditor<Definition>::afterRenderAny(any, def, changed);
        }
    };

    template<typename T>
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
            if (_path)
            {
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
            }
            return ObjectEditor<T>::afterRenderAny(any, def, changed);
        }
    };

    class ObjectEditorContainer final
    {
    public:
        ObjectEditorContainer();

        template<typename T, typename... A>
        T& add(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            add(std::move(ptr));
            return ref;
        }

        using RenderResult = expected<bool, std::string>;

        RenderResult render(google::protobuf::Message& obj, bool withTitle = false) const noexcept;
        void add(std::unique_ptr<IObjectEditor>&& editor) noexcept;
        void init(EditorApp& app);
        void shutdown();

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<std::string, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}