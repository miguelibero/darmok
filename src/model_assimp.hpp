#pragma once

#include <darmok/model.hpp>
#include <darmok/optional_ref.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include "assimp.hpp"

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class AssimpModelNodeChildrenCollection final : public ValCollection<std::shared_ptr<ModelNode>>
    {
    public:
        AssimpModelNodeChildrenCollection(const AssimpNode& parent) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] std::shared_ptr<ModelNode> operator[](size_t pos) const  override;
    private:
        std::vector<std::shared_ptr<ModelNode>> _children;
    };

    class ModelNodeImpl final
    {
    public:
        ModelNodeImpl(const std::shared_ptr<AssimpNode>& assimp) noexcept;

        std::string_view getName() const noexcept;
        glm::mat4 getTransform() const noexcept;
        const ValCollection<std::shared_ptr<ModelNode>>& getChildren() const noexcept;
        void configureEntity(Entity entity, const ModelSceneConfig& config) const;

    private:
        std::shared_ptr<AssimpNode> _assimp;
        AssimpModelNodeChildrenCollection _children;

        void configureMesh(const AssimpMesh& mesh, Entity entity, const ModelSceneConfig& config) const noexcept;
        void configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept;
        void configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept;
    };

    class ModelImpl final
    {
    public:
        ModelImpl(const AssimpScene& assimp) noexcept;
        std::shared_ptr<ModelNode> getRootNode() const noexcept;

    private:
        AssimpScene _assimp;
        std::shared_ptr<ModelNode> _rootNode;
    };

    class IDataLoader;

    class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc);
		std::shared_ptr<Model> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
        bx::AllocatorI& _alloc;
	};
}