#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/scene_fwd.hpp>

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
    protected:

        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;

        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
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
		virtual RenderResult beforeRenderAny(Any& any, Message& msg) noexcept { return false; }
        virtual RenderResult afterRenderAny(Any& any, Message& msg) noexcept { return false; }

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
			auto result = beforeRenderAny(any, msg);
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
            result = afterRenderAny(any, msg);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            changed |= *result;
            if (changed)
            {
                if (!any.PackFrom(obj))
                {
                    return unexpected<std::string>{"failed to pack any"};
                }
                return true;
            }
            return false;
        }
    };

    template<typename T>
    class BX_NO_VTABLE ComponentObjectEditor : public ObjectEditor<T>
    {
    protected:
        std::optional<Entity> _entity;
		using RenderResult = ObjectEditor<T>::RenderResult;
        using Message = ObjectEditor<T>::Message;
        using Any = ObjectEditor<T>::Any;

        RenderResult render(google::protobuf::Message& msg) noexcept override
        {
			return ObjectEditor<T>::render(msg);
        }

        RenderResult beforeRenderAny(Any& any, Message& msg) noexcept override
        {
            _entity = BaseObjectEditor::getEntity(any);
            return ObjectEditor<T>::beforeRenderAny(any, msg);
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

        RenderResult render(google::protobuf::Message& msg) noexcept override
        {
            _path.reset();
            return ObjectEditor<T>::render(msg);
        }

        RenderResult beforeRenderAny(Any& any, Message& msg) noexcept override
        {
            _path = BaseObjectEditor::getAssetPath(any);
            return ObjectEditor<T>::beforeRenderAny(any, msg);
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

        template<typename Itr>
        RenderResult render(Itr begin, Itr end) noexcept
        {
            int i = 0;
			bool changed = false;
            for (auto itr = begin; itr != end; ++itr)
            {
                auto& elm = *itr;
                ImGui::PushID(i);
                auto result = render(elm, true);
                ImGui::PopID();
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                if(result.value())
                {
                    changed = true;
                }
                ++i;
            }
            return changed;
        }

        RenderResult render(google::protobuf::Message& obj, bool withTitle = false) const noexcept;
        void add(std::unique_ptr<IObjectEditor>&& editor) noexcept;
        void init(EditorApp& app);
        void shutdown();

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<std::string, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}