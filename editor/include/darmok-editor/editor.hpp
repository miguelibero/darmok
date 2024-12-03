#pragma once

#include <bx/bx.h>
#include <entt/entt.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class ObjectEditorContainer;
    class EditorAppDelegate;

    class BX_NO_VTABLE IObjectEditor
    {
    public:
        virtual ~IObjectEditor() = default;
        virtual void init(EditorAppDelegate& app, ObjectEditorContainer& container) { }
        virtual void shutdown() {}
        virtual entt::type_info getObjectType() const = 0;
        virtual bool tryRender(entt::meta_any& any) { return false; }
    };

    template<typename T>
    class BX_NO_VTABLE ITypeObjectEditor : public IObjectEditor
    {
    public:
        virtual bool render(T& obj) = 0;

        entt::type_info getObjectType() const override
        {
            return entt::type_id<T>();
        }

        bool tryRender(entt::meta_any& any) override
        {
            if (auto ptr = any.try_cast<T>())
            {
                return render(*ptr);
            }
            return false;
        }
    };

    class ObjectEditorContainer final
    {
    public:

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

        bool render(entt::meta_any& obj) const;
        void add(std::unique_ptr<IObjectEditor>&& editor);
        void init(EditorAppDelegate& app);
        void shutdown();

    private:
        std::unordered_map<entt::id_type, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
        OptionalRef<EditorAppDelegate> _app;
    };
}