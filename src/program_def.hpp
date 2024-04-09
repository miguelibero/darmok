#include <darmok/program_def.hpp>
#include <rapidjson/document.h>
#include <optional>

namespace darmok
{
	class JsonDataProgramDefinitionLoader final : public IProgramDefinitionLoader
	{
	public:
		JsonDataProgramDefinitionLoader(IDataLoader& dataLoader);
		ProgramDefinition operator()(std::string_view name) override;
		static void read(ProgramDefinition& progDef, const rapidjson::Document& json);

		template<typename I>
		static void read(ProgramDefinition& progDef, const rapidjson::Document& json, I includeCallback)
		{
			read(progDef, json);
			if (json.HasMember("include"))
			{
				for (auto& elm : json["include"].GetArray())
				{
					std::string_view include(elm.GetString(), elm.GetStringLength());
					std::optional<ProgramDefinition> comp = includeCallback(include);
					if (comp)
					{
						progDef += comp.value();
					}
				}
			}
		}

	private:
		IDataLoader& _dataLoader;
	};
}