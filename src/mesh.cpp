#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/shape_serialize.hpp>

namespace darmok
{
	MeshDefinitionFromSourceLoader::MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramDefinitionLoader& progDefLoader) noexcept
		: FromDefinitionLoader(srcLoader)
		, _progDefLoader{ progDefLoader }
	{
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(const std::shared_ptr<IMesh::Source>& src)
	{
		if (!src)
		{
			return unexpected<std::string>{ "Mesh source is null" };
		}

		std::shared_ptr<Program::Definition> progDef;
		if (src->has_standard_program())
		{
			progDef = StandardProgramLoader::loadDefinition(src->standard_program());
		}
		if (!progDef && src->has_program_path())
		{
			auto progResult = _progDefLoader(src->program_path());
			if (!progResult)
			{
				return unexpected{ progResult.error() };
			}
			progDef = progResult.value();
		}
		if (!progDef)
		{
			return unexpected<std::string>{ "No program found for mesh source" };
		}

		auto layout = VaryingUtils::getBgfx(progDef->varying().vertex());

		std::shared_ptr<IMesh::Definition> defPtr;
		MeshConfig config{ .index32 = src->index32() };
		if (src->has_sphere())
		{
			auto shape = protobuf::convert(src->sphere().shape());
			auto def = MeshData{ shape, src->sphere().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<IMesh::Definition>(std::move(def));
		}
		if (src->has_cube())
		{
			auto shape = protobuf::convert(src->cube().shape());
			auto def = MeshData{ shape, src->cube().type() }.createDefinition(layout, config);
			defPtr = std::make_shared<IMesh::Definition>(std::move(def));
		}
		if (src->has_capsule())
		{
			auto shape = protobuf::convert(src->capsule().shape());
			auto def = MeshData{ shape, src->capsule().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<IMesh::Definition>(std::move(def));
		}

		if (defPtr)
		{
			return defPtr;
		}
		
		return unexpected<std::string>{ "Unsupported mesh type" };
	}
}