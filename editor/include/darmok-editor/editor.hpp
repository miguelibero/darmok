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
        virtual std::string getTitle() const noexcept = 0;
        virtual std::string getObjectTypeUrl() const = 0;
        virtual bool canRender(const google::protobuf::Message& msg) const = 0;
        virtual bool render(google::protobuf::Message& msg) { return false; }
    };

    template<typename T>
    class BX_NO_VTABLE ITypeObjectEditor : public IObjectEditor
    {
    public:
        virtual bool renderType(T& obj) = 0;

        std::string getObjectTypeUrl() const override
        {
            return protobuf::getTypeUrl<T>();
        }

        bool canRender(const google::protobuf::Message& msg) const override
        {
            auto typeUrl = getObjectTypeUrl();
            if (!protobuf::isAny(msg))
            {
                return protobuf::getTypeUrl(msg) == typeUrl;
            }
            return static_cast<const google::protobuf::Any&>(msg).type_url() == typeUrl;
        }

        bool render(google::protobuf::Message& msg) override
        {
            if (!protobuf::isAny(msg))
            {
				return renderType(static_cast<T&>(msg));
            }

            T obj;
            auto& any = static_cast<google::protobuf::Any&>(msg);
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
        bool render(Itr begin, Itr end)
        {
            int i = 0;
			bool changed = false;
            for (auto itr = begin; itr != end; ++itr)
            {
                auto& elm = *itr;
                ImGui::PushID(i);
                if (render(elm, true))
                {
                    changed = true;
                }
                ImGui::PopID();
                ++i;
            }
            return changed;
        }

        bool render(google::protobuf::Message& obj, bool withTitle = false) const;
        void add(std::unique_ptr<IObjectEditor>&& editor);
        void init(EditorApp& app);
        void shutdown();

    private:
        OptionalRef<EditorApp> _app;
        std::unordered_map<std::string, std::vector<std::unique_ptr<IObjectEditor>>> _editors;
    };
}