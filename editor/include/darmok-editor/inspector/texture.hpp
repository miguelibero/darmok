#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/texture.hpp>

#include <memory>

namespace darmok::editor
{
    class TextureDefinitionInspectorEditor final : public ITypeObjectEditor<Texture::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Texture::Source& def) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        std::shared_ptr<Texture> _tex;
        static const std::string _imageFilter;
        static const glm::vec2 _maxPreviewSize;
    };
}