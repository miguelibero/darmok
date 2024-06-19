#include "skeleton_assimp.hpp"
#include <darmok/skeleton_assimp.hpp>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>

namespace darmok
{
    AssimpSkeletonConverter::AssimpSkeletonConverter(const aiScene& scene) noexcept
        : _scene(scene)
    {
    }

    bool AssimpSkeletonConverter::update(Skeleton& skel) noexcept
    {
        return false;
    }

    bool AssimpSkeletonConverter::update(const std::string& name, Animation& anim) noexcept
    {
        return false;
    }

    std::vector<std::string> AssimpSkeletonConverter::getAnimationNames()
    {
        std::vector<std::string> names;
        return names;
    }
}