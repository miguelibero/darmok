#include <darmok/program_def.hpp>
#include <rapidjson/document.h>

namespace darmok
{
	class JsonDataProgramDefinitionLoader final : public IProgramDefinitionLoader
	{
	public:
		JsonDataProgramDefinitionLoader(IDataLoader& dataLoader);
		ProgramDefinition operator()(std::string_view name) override;
		static void read(ProgramDefinition& progDef, const rapidjson::Document& json);
	private:
		IDataLoader& _dataLoader;
	};
}