#pragma once
#include <optional>
#include <vector>
#include <string>

namespace ozz::animation::offline
{
    struct RawSkeleton;
    struct RawAnimation;
}

struct aiScene;
struct aiNode;

namespace darmok
{
    class AssimpSkeletonConverter final
    {
    public:
        using Skeleton = ozz::animation::offline::RawSkeleton;
        using Animation = ozz::animation::offline::RawAnimation;
        AssimpSkeletonConverter(const aiScene& scene) noexcept;
        bool update(Skeleton& skel) noexcept;
        bool update(const std::string& name, Animation& anim) noexcept;
        std::vector<std::string> getAnimationNames();
    private:
        const aiScene& _scene;

        // void extractSkeleton(const aiNode& node) const;
    };
}