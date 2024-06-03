#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>
#include <darmok/render.hpp>
#include <bx/bx.h>

#ifndef DARMOK_SKELETON_MAX_BONES
#define DARMOK_SKELETON_MAX_BONES 64
#endif

namespace darmok
{
    class SkeletonImpl;

    class Skeleton final
    {
    public:
        Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept;
        ~Skeleton();
        SkeletonImpl& getImpl();
        const SkeletonImpl& getImpl() const;
    private:
        std::unique_ptr<SkeletonImpl> _impl;
    };

    class SkeletalAnimationImpl;

	class SkeletalAnimation final
    {
    public:
        SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept;
        ~SkeletalAnimation();
        SkeletalAnimationImpl& getImpl();
        const SkeletalAnimationImpl& getImpl() const;
        std::string_view getName() const noexcept;
        float getDuration() const noexcept;
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

    class SkeletalAnimationControllerImpl;
    class Transform;
    class Material;

    class SkeletalAnimationController final
    {
    public:
        DLLEXPORT SkeletalAnimationController(const std::shared_ptr<Skeleton>& skel, const std::vector<std::shared_ptr<SkeletalAnimation>>& animations = {}) noexcept;
        DLLEXPORT ~SkeletalAnimationController();
        DLLEXPORT SkeletalAnimationController& addAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept;
        DLLEXPORT bool playAnimation(std::string_view name, bool loop = true) noexcept;
        DLLEXPORT glm::mat4 getModelMatrix(const std::string& joint) const noexcept;
        DLLEXPORT std::vector<glm::mat4> getBoneMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;
        DLLEXPORT SkeletalAnimationController& setPlaybackSpeed(float speed) noexcept;
        void update(float deltaTime) noexcept;
    private:
        std::unique_ptr<SkeletalAnimationControllerImpl> _impl;
    };

    class RenderableSkeleton final
    {
    public:
        DLLEXPORT RenderableSkeleton(const std::shared_ptr<IMesh>& boneMesh, const std::shared_ptr<Material>& mat) noexcept;
        ~RenderableSkeleton() noexcept;
        void init(Scene& scene) noexcept;
        void update(float deltaTime) noexcept;
        void shutdown() noexcept;
    private:
        OptionalRef<Scene> _scene;
        std::shared_ptr<Material> _material;
        std::shared_ptr<IMesh> _boneMesh;
        OptionalRef<SkeletalAnimationController> _ctrl;
        std::vector<OptionalRef<Transform>> _bones;
    };

    class SkeletalAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        void init(Scene& scene, App& app) noexcept override;
        void update(float deltaTime) noexcept override;
        void shutdown() noexcept override;
    private:
        OptionalRef<Scene> _scene;

        void onSkeletonConstructed(EntityRegistry& registry, Entity entity) noexcept;
    };


    struct ArmatureBone final
    {
        std::string joint;
        glm::mat4 inverseBindPose;
    };

    class Armature final
    {
    public:
        DLLEXPORT Armature(const std::vector<ArmatureBone>& bones) noexcept;
        DLLEXPORT Armature(std::vector<ArmatureBone>&& bones) noexcept;
        DLLEXPORT const std::vector<ArmatureBone>& getBones() const noexcept;

    private:
        std::vector<ArmatureBone> _bones;
    };

    class Skinnable
    {
    public:
        DLLEXPORT Skinnable(const std::shared_ptr<Armature>& armature = nullptr) noexcept;
        const std::shared_ptr<Armature>& getArmature() const noexcept;
        void setArmature(const std::shared_ptr<Armature>& armature) noexcept;
    private:
        std::shared_ptr<Armature> _armature;
    };

    class SkeletalAnimationCameraComponent final : public ICameraComponent
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept override;
        void shutdown() noexcept override;
    private:
        bgfx::UniformHandle _skinningUniform;
        std::vector<glm::mat4> _skinning;
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        OptionalRef<SkeletalAnimationController> getController(Entity entity) const noexcept;
    };
}