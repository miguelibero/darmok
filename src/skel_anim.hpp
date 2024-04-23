#pragma once

#include <darmok/skel_anim.hpp>

namespace darmok
{
    class OzzSkeletonLoader final : public ISkeletonLoader
	{
	public:
		OzzSkeletonLoader(IDataLoader& dataLoader);
		std::shared_ptr<ISkeleton> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
		OptionalRef<ITextureLoader> _textureLoader;
		bx::AllocatorI* _alloc;
	};
}