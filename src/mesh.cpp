#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/shape_serialize.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
	MeshDefinitionFromSourceLoader::MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramDefinitionLoader& progDefLoader, bx::AllocatorI& allocator) noexcept
		: FromDefinitionLoader(srcLoader)
		, _progDefLoader{ progDefLoader }
		, _allocator{ allocator }
	{
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(const std::shared_ptr<Mesh::Source>& src)
	{
		if (!src)
		{
			return unexpected<std::string>{ "Mesh source is null" };
		}

		auto progResult = Program::loadRef(_progDefLoader, src->program());
		if (!progResult)
		{
			return unexpected{ progResult.error() };
		}
		auto progDef = progResult.value();
		if (!progDef)
		{
			return nullptr;
		}

		auto layout = ConstVertexLayoutWrapper{ progDef->varying().vertex() }.getBgfx();

		std::shared_ptr<Mesh::Definition> defPtr;
		MeshConfig config{ .index32 = src->index32() };
		if (src->has_sphere())
		{
			auto shape = protobuf::convert(src->sphere().shape());
			auto def = MeshData{ shape, src->sphere().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_cube())
		{
			auto shape = protobuf::convert(src->cube().shape());
			auto def = MeshData{ shape, src->cube().type() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_capsule())
		{
			auto shape = protobuf::convert(src->capsule().shape());
			auto def = MeshData{ shape, src->capsule().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_rectangle())
		{
			auto shape = protobuf::convert(src->rectangle().shape());
			auto def = MeshData{ shape, src->rectangle().type() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_data())
		{
			auto def = MeshData{ src->data() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else
		{
			return unexpected<std::string>{ "Unsupported mesh type" };
		}

		return defPtr;
	}
}