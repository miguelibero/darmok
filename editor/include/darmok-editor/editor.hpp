#pragma once

#include <bx/bx.h>
#include <entt/entt.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class ObjectEditorContainer;
    class EditorApp;

    class BX_NO_VTABLE IObjectEditor
    {
    public:
        virtual ~IObjectEditor() = default;
        virtual void init(EditorApp& app, ObjectEditorContainer& container) {}
        virtual void shutdown() {}
        virtual entt::type_info getObjectType() const = 0;
        virtual bool canRender(entt::meta_any& any) const = 0;
        virtual bool render(entt::meta_any& any) { return false; }
    };

    template<typename T>
    class BX_NO_VTABLE ITypeObjectEditor : public IObjectEditor
    {
    public:
        virtual bool renderType(T& obj) = 0;

        entt::type_info getObjectType() const override
        {
            return entt::type_id<T>();
        }

        bool canRender(entt::meta_any& any) const override
        {
            return any.type().info() == getObjectType();
        }

        bool render(entt::meta_any& any) override
        {
            if (auto ptr = any.try_cast<T>())
            {
                return renderType(*ptr);
            }
            return false;
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

        template<typename Itr>
        void render(Itr begin, Itr end)
        {
            for (auto itr = begin; itr != end; ++itr)
            {
                render(*itr);
            }
        }

        template<typename T>
        bool renderType(T& obj)
        {
            auto any = entt::forward_as_meta(obj);
            return render(any);
        }

        bool render(entt::meta_any& obj) const;
        void add(std::unique_ptr<IObjectEditor>&& editor);
        void init(EditorApp& app);
        void shutdown();

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<entt::id_type, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}