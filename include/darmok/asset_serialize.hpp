#pragma once

#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/serialize.hpp>

#include <cereal/cereal.hpp>

namespace std
{
	template<typename Archive>
	void save(Archive& archive, const std::shared_ptr<darmok::Program>& prog)
	{
		auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
		auto def = assets.getProgramLoader().getDefinition(prog);
		archive(CEREAL_NVP_("definition", def));
	}

	template<typename Archive>
	void load(Archive& archive, std::shared_ptr<darmok::Program>& prog)
	{
		std::shared_ptr<darmok::ProgramDefinition> def;
		archive(CEREAL_NVP_("definition", def));
		auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
		prog = assets.getProgramLoader().loadResource(def);
	}

    template<typename Archive>
    void save(Archive& archive, const std::shared_ptr<darmok::Texture>& tex)
    {
        auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
        auto def = assets.getTextureLoader().getDefinition(tex);
        archive(CEREAL_NVP_("definition", def));
    }

    template<typename Archive>
    void load(Archive& archive, std::shared_ptr<darmok::Texture>& tex)
    {
        std::shared_ptr<darmok::TextureDefinition> def;
        archive(CEREAL_NVP_("definition", def));
        auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
        tex = assets.getTextureLoader().loadResource(def);
    }

    template<typename Archive>
    void save(Archive& archive, const std::shared_ptr<darmok::IMesh>& mesh)
    {
        auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
        auto def = assets.getMeshLoader().getDefinition(mesh);
        archive(CEREAL_NVP_("definition", def));
    }

    template<typename Archive>
    void load(Archive& archive, std::shared_ptr<darmok::IMesh>& mesh)
    {
        std::shared_ptr<darmok::MeshDefinition> def;
        archive(CEREAL_NVP_("definition", def));
        auto& assets = darmok::SerializeContextStack<darmok::AssetContext>::get();
        mesh = assets.getMeshLoader().loadResource(def);
    }
}