#pragma once

#include <darmok/scene.hpp>
#include <darmok/shape.hpp>
#include <darmok/physics3d_fwd.hpp>
#include <memory>
#include <bx/bx.h>
#include <variant>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
	class DARMOK_EXPORT BX_NO_VTABLE IPhysics3dUpdater
	{
	public:
		virtual ~IPhysics3dUpdater() = default;
		virtual void fixedUpdate(float fixedDeltaTime) = 0;
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
        void addUpdater(IPhysics3dUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3dUpdater& updater) noexcept;
    private:
        std::unique_ptr<Physics3dSystemImpl> _impl;
    };

    using RigidBody3dShape = std::variant<Cuboid, Sphere, Plane, Capsule>;

    class RigidBody3dImpl;

    class RigidBody3d final
    {
    public:
        using MotionType = RigidBody3dMotionType;
        using Shape = RigidBody3dShape;

        RigidBody3d(const Shape& shape, MotionType motion = MotionType::Dynamic) noexcept;
        RigidBody3d(const Shape& shape, float density, MotionType motion = MotionType::Dynamic) noexcept;
        ~RigidBody3d() noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        float getDensity() const noexcept;
        float getMass() const noexcept;
        RigidBody3dImpl& getImpl() noexcept;
        const RigidBody3dImpl& getImpl() const noexcept;

    private:
        std::unique_ptr<RigidBody3dImpl> _impl;
    };
}