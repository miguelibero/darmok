#pragma once

#include <darmok/export.h>
#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>

namespace darmok
{
	class IComponentLoadContext;

	class DARMOK_EXPORT Prefab final
	{
	public:
		Prefab(std::string scenePath = {}) noexcept;

		using Definition = protobuf::Prefab;

		expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept;
		static Definition createDefinition() noexcept;
	private:
		std::string _scenePath;
	};
}