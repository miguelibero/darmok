#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/material.hpp>

#include <unordered_map>
#include <memory>

namespace darmok
{
    class AssetPack;
}

namespace darmok::editor
{
    class MaterialTextureEditor final
    {
    public:
        MaterialTextureEditor(std::string_view label, Material::TextureDefinition::Type type);
        bool render(Material::Definition& mat, AssetPack& assets) noexcept;
    private:
		std::string _label;
		Material::TextureDefinition::Type _type;
        std::shared_ptr<Texture> _tex;
    };

    class MaterialInspectorEditor final : public ITypeObjectEditor<Material::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Material::Definition& mat) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
		std::unordered_map<Material::TextureDefinition::Type, MaterialTextureEditor> _textureEditors;

        bool renderTextureInput(Material::Definition& mat, Material::TextureDefinition::Type type) noexcept;
    };
}