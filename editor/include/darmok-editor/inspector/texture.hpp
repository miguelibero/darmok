#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/texture.hpp>

#include <memory>
#include <unordered_map>

namespace darmok::editor
{
    class TextureInspectorEditor final : public AssetObjectEditor<Texture::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Texture::Source& def) noexcept override;
    private:
        std::unordered_map<std::size_t, std::shared_ptr<Texture>> _textures;
        static const glm::vec2 _maxPreviewSize;
    };
}