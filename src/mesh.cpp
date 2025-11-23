#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
	MeshDefinitionFromSourceLoader::MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramSourceLoader& progLoader) noexcept
		: FromDefinitionLoader(srcLoader)
		, _progLoader{ progLoader }
	{
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(std::shared_ptr<Mesh::Source> src) noexcept
	{
		if (!src)
		{
			return unexpected<std::string>{ "Mesh source is null" };
		}

		auto progResult = Program::loadRefVarying(src->program(), _progLoader);
		if (!progResult)
		{
			return unexpected{ progResult.error() };
		}
		auto varying = progResult.value();
		auto layout = ConstVertexLayoutWrapper{ varying.vertex() }.getBgfx();
		MeshConfig config{ .index32 = src->index32() };

		auto def = MeshData{ *src }.createSharedDefinition(layout, config);
		return def;
	}
}