#include <darmok/sprite.hpp>
#include <darmok/utils.hpp>
#include <darmok/vertex.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    static std::shared_ptr<Material> createSpriteMaterial(const std::shared_ptr<Texture>& texture, const ProgramDefinition& progDef) noexcept
    {
        auto mat = std::make_shared<Material>(progDef);
        mat->setTexture(MaterialTextureType::Diffuse, texture);
        return mat;
    }

    std::shared_ptr<Mesh> SpriteUtils::fromAtlas(const TextureAtlas& atlas, const ProgramDefinition& progDef, const TextureAtlasElement& element, float scale, const Color& color)
    {
        auto material = createSpriteMaterial(atlas.texture, progDef);
        auto size = std::min(element.positions.size(), element.texCoords.size());
        VertexDataWriter writer(material->getVertexLayout(), size);

        glm::vec2 atlasSize(atlas.size);

        uint32_t i = 0;
        for (auto& pos : element.positions)
        {
            auto v = scale * glm::vec2(pos.x, element.originalSize.y - pos.y);
            writer.set(bgfx::Attrib::Position, i++, v);
        }
        i = 0;
        for (auto& texCoord : element.texCoords)
        {
            auto v = glm::vec2(texCoord) / atlasSize;
            writer.set(bgfx::Attrib::TexCoord0, i++, v);
        }
        writer.set(bgfx::Attrib::Color0, color);
        return std::make_shared<Mesh>(material, writer.finish(), Data::copy(element.indices));
    }

    std::vector<AnimationFrame> SpriteUtils::fromAtlas(const TextureAtlas& atlas, const ProgramDefinition& progDef, std::string_view namePrefix, float frameDuration, float scale, const Color& color)
    {
        std::vector<AnimationFrame> frames;
        for (auto& elm : atlas.elements)
        {
            if (elm.name.starts_with(namePrefix))
            {
                auto mesh = fromAtlas(atlas, progDef, elm, scale, color);
                if (mesh)
                {
                    frames.push_back({ { mesh }, frameDuration });
                }
            }
        }
        return frames;
    }

    static const std::vector<glm::vec2> _textureSpritePositions = {
        { 1, 1 },
        { 1, 0 },
        { 0, 0 },
        { 0, 1 },
    };

    static const std::vector<glm::vec2> _textureSpriteTexCoords = {
        { 1, 0 },
        { 1, 1 },
        { 0, 1 },
        { 0, 0 },
    };

    static const std::vector<VertexIndex> _textureSpriteIndices = {
        0, 1, 2, 2, 3, 0
    };

    std::shared_ptr<Mesh> SpriteUtils::fromTexture(const std::shared_ptr<Texture>& texture, const ProgramDefinition& progDef, float scale, const Color& color)
    {
        auto material = createSpriteMaterial(texture, progDef);
        VertexDataWriter writer(material->getVertexLayout(), _textureSpritePositions.size());
        auto size = glm::vec2(texture->getImage()->getSize()) * scale;
        auto i = 0;
        for (auto& pos : _textureSpritePositions)
        {
            auto v = pos * size;
            writer.set(bgfx::Attrib::Position, i++, v);
        }
        writer.set(bgfx::Attrib::TexCoord0, _textureSpriteTexCoords);
        writer.set(bgfx::Attrib::Color0, color);
        return std::make_shared<Mesh>(material, writer.finish(), Data::copy(_textureSpriteIndices));
    }


}