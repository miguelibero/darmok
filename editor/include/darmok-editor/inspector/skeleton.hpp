#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/skeleton.hpp>

struct aiScene;

namespace darmok::editor
{
    class ArmatureInspectorEditor final : public AssetObjectEditor<Armature::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Armature::Definition& armature) noexcept override;
    private:
        std::shared_ptr<aiScene> _scene;
        std::vector<std::string> _meshes;
        size_t _meshIndex = 0;

        expected<void, std::string> loadSceneMesh(Armature::Definition& armature) noexcept;
    };

    class SkinnableInspectorEditor final : public ComponentObjectEditor<Skinnable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Skinnable::Definition& skinnable) noexcept override;
    };

    class SkeletalAnimatorInspectorEditor final : public ComponentObjectEditor<SkeletalAnimator>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimator::Definition& animator) noexcept override;
    };
}