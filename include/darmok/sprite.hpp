#pragma once

#include <darmok/color.hpp>
#include <darmok/anim.hpp>
#include <memory>
#include <vector>
#include <string_view>

namespace darmok
{
    class Texture;
    class TextureAtlas;
    class TextureAtlasElement;
    class Mesh;
    struct ProgramDefinition;


    class SpriteUtils final
    {
    public:
        static std::shared_ptr<Mesh> fromAtlas(const TextureAtlas& atlas, const ProgramDefinition& progDef, const TextureAtlasElement& element, float scale = 1.f, const Color& color = Colors::white);
        static std::vector<AnimationFrame> fromAtlas(const TextureAtlas& atlas, const ProgramDefinition& progDef, std::string_view namePrefix, float frameDuration = 1.f/30.f, float scale = 1.f, const Color& color = Colors::white);
        static std::shared_ptr<Mesh> fromTexture(const std::shared_ptr<Texture>& texture, const ProgramDefinition& progDef, float scale = 1.f, const Color& color = Colors::white);
    };
}

