#pragma once

namespace darmok
{
	struct DARMOK_EXPORT AssetPackConfig final
	{
		ProgramCompilerConfig programCompilerConfig;
		OptionalRef<IAssetContext> fallback;
	};
}