#pragma once

#include <darmok/scene.hpp>
#include <memory>
#include <bx/bx.h>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
	class DARMOK_EXPORT BX_NO_VTABLE IPhysics3DUpdater
	{
	public:
		virtual ~IPhysics3DUpdater() = default;
		virtual void fixedUpdate() = 0;
	};

    enum class Physics3dLayer : uint8_t
    {
        NonMoving,
        Moving,
        Count
    };

    class Physics3dSystemImpl;

    class DARMOK_EXPORT Physics3dSystem final : public ISceneLogicUpdater
    {
    public:
        Physics3dSystem(bx::AllocatorI& alloc) noexcept;
        ~Physics3dSystem() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void addUpdater(IPhysics3DUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3DUpdater& updater) noexcept;
    private:
        std::unique_ptr<Physics3dSystemImpl> _impl;
    };
}