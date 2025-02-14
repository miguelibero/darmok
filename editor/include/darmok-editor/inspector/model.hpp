#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/model.hpp>

#include <variant>

namespace darmok::editor
{
    class ModelInspectorEditor final : public ITypeObjectEditor<Model>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Model& model) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;
        static const std::string _fileFilter;
    };
}
