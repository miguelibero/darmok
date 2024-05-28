#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>
#include <bx/bx.h>

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

    class SkeletalAnimationController final
    {
    public:
        ~SkeletalAnimationController();
        DLLEXPORT SkeletalAnimationController(const std::shared_ptr<Skeleton>& skel, const std::vector<std::shared_ptr<SkeletalAnimation>>& animations = {}) noexcept;
        DLLEXPORT SkeletalAnimationController& addAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept;
        DLLEXPORT bool playAnimation(std::string_view name) noexcept;
        DLLEXPORT glm::mat4 getModelMatrix(const std::string& joint) const noexcept;
        void update(float deltaTime) noexcept;
    private:
        std::unique_ptr<SkeletalAnimationControllerImpl> _impl;
    };

    class SkeletalAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        DLLEXPORT void init(Scene& scene, App& app) noexcept override;
        DLLEXPORT void update(float deltaTime) noexcept override;
    private:
        OptionalRef<Scene> _scene;
    };

    class Mesh;

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
        DLLEXPORT void init(Camera& cam, Scene& scene, App& app) noexcept override;
        DLLEXPORT void getEntityTransforms(Entity entity, std::vector<glm::mat4>& transforms) const override;
    private:
        OptionalRef<Scene> _scene;
    };
}