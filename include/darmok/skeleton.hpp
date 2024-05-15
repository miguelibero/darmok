#pragma once

#include <memory>
#include <string_view>
#include <stdexcept>
#include <bx/bx.h>

namespace darmok
{
    class SkeletonImpl;

    class Skeleton final
    {
    public:
        Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept;
    private:
        std::unique_ptr<SkeletonImpl> _impl;
    };

    class SkeletalAnimationImpl;

	class SkeletalAnimation final
    {
    public:
        SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept;        
    private:
        std::unique_ptr<SkeletalAnimationImpl> _impl;
    };

    class BX_NO_VTABLE ISkeletonLoader
	{
	public:
        using result_type = std::shared_ptr<Skeleton>;

        DLLEXPORT virtual ~ISkeletonLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

    class BX_NO_VTABLE ISkeletalAnimationLoader
	{
	public:
        using result_type = std::shared_ptr<SkeletalAnimation>;
		DLLEXPORT virtual ~ISkeletalAnimationLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

    class EmptySkeletonLoader : public ISkeletonLoader
	{
	public:
        DLLEXPORT std::shared_ptr<Skeleton> operator()(std::string_view name) override
        {
            throw std::runtime_error("no skeletal animation implementation");
        }
	};

    class EmptySkeletalAnimationLoader : public ISkeletalAnimationLoader
	{
	public:
        DLLEXPORT std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override
        {
            throw std::runtime_error("no skeletal animation implementation");
        }
	};
}