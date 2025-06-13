#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/protobuf.hpp>
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
        virtual ~IObjectEditor() = default;
        virtual void init(EditorApp& app, ObjectEditorContainer& container) {}
        virtual void shutdown() {}
        virtual std::string getObjectTypeUrl() const = 0;
        virtual bool canRender(google::protobuf::Any& any) const = 0;
        virtual bool render(google::protobuf::Any& any) { return false; }
    };

    template<typename T>
    class BX_NO_VTABLE ITypeObjectEditor : public IObjectEditor
    {
    public:
        virtual bool renderType(T& obj) = 0;

        std::string getObjectTypUrl() const override
        {
            return protobuf::getTypeUrl<T>();
        }

        bool canRender(google::protobuf::Any& any) const override
        {
            return any.type_url() == getObjectTypUrl();
        }

        bool render(google::protobuf::Any& any) override
        {
            T obj;
            if (any.UnpackTo(&obj))
            {
                if (renderType(obj))
                {
                    any.PackFrom(obj);
                    return true;
                }
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
            int i = 0;
            for (auto itr = begin; itr != end; ++itr)
            {
                auto& elm = *itr;
                ImGui::PushID(i);
                render(elm);
                ImGui::PopID();
                ++i;
            }
        }

        template<typename T>
        bool renderType(T& obj)
        {
            auto any = entt::forward_as_meta(obj);
            return render(any);
        }

        bool render(google::protobuf::Any& obj) const;
        void add(std::unique_ptr<IObjectEditor>&& editor);
        void init(EditorApp& app);
        void shutdown();

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<std::string, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}