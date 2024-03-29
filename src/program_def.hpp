#include <darmok/program_def.hpp>

namespace darmok
{
	class JsonDataProgramDefinitionLoader final : public IProgramDefinitionLoader
	{
	public:
		JsonDataProgramDefinitionLoader(IDataLoader& dataLoader);
		std::shared_ptr<ProgramDefinition> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};
}