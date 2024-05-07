#pragma once

#include <bx/bx.h>
#include <string_view>
#include <memory>
#include <stdexcept>

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
		virtual ~ISkeletonLoader() = default;
		virtual std::shared_ptr<Skeleton> operator()(std::string_view name) = 0;
	};

    class BX_NO_VTABLE ISkeletalAnimationLoader
	{
	public:
		virtual ~ISkeletalAnimationLoader() = default;
		virtual std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) = 0;
	};

    class EmptySkeletonLoader : public ISkeletonLoader
	{
	public:
		std::shared_ptr<Skeleton> operator()(std::string_view name) override
        {
            throw std::runtime_error("no skeletal animation library");
        }
	};

    class EmptySkeletalAnimationLoader : public ISkeletalAnimationLoader
	{
	public:
		std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override
        {
            throw std::runtime_error("no skeletal animation library");
        }
	};
}