#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    MeshDefinitionFromSourceLoader::MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader) noexcept
		: FromDefinitionLoader(srcLoader)

	{
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(std::shared_ptr<Mesh::Source> src) noexcept
	{
		if (!src)
		{
			return unexpected<std::string>{ "Mesh source is null" };
		}
        bgfx::VertexLayout vertexLayout;

        if (src->has_vertex_layout())
        {
            vertexLayout = ConstVertexLayoutWrapper{src->vertex_layout()}.getBgfx();
        }
        else
        {
            auto layoutResult = StandardProgramLoader::loadDefaultVertexLayout();
            if(!layoutResult)
            {
                return unexpected{layoutResult.error()};
            }
            vertexLayout = std::move(layoutResult).value();
        }

		MeshConfig config{ .index32 = src->index32() };
		auto def = MeshData{*src}.createSharedDefinition(vertexLayout, config);
		return def;
	}
}
