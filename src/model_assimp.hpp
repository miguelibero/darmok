#pragma once

#include <darmok/model.hpp>
#include <darmok/optional_ref.hpp>

#include <assimp/Importer.hpp>
#include "assimp.hpp"

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class Mesh;

    class AssimpModelNode final : public IModelNode
    {
    public:
        AssimpModelNode(const AssimpNode& assimp) noexcept;

        std::string_view getName() const noexcept override;
        glm::mat4 getTransform() const noexcept override;
        const ReadOnlyCollection<IModelNode>& getChildren() const noexcept override;
        void configureEntity(Entity entity, const ModelSceneConfig& config) const override;

    private:
        const AssimpNode& _assimp;
        VectorReadOnlyCollection<IModelNode> _children;

        void configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept;
        void configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept;
    };

    class AssimpModel final : public IModel
    {
    public:
        AssimpModel(AssimpScene&& assimp) noexcept;
        IModelNode& getRootNode() noexcept override;
        const IModelNode& getRootNode() const noexcept override;

    private:
        AssimpModelNode _rootNode;
        AssimpScene _assimp;
    };

    class IDataLoader;

    class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc);
		std::shared_ptr<IModel> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
        bx::AllocatorI& _alloc;
	};
}