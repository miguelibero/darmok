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
    class AssimpModelNode;

    class AssimpModelNodeChildrenCollection final : public ConstRefCollection<IModelNode>
    {
    public:
        AssimpModelNodeChildrenCollection(const std::vector<AssimpModelNode>& children) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] const IModelNode& operator[](size_t pos) const  override;
    private:
        const std::vector<AssimpModelNode>& _children;
    };

    class AssimpModelNode final : public IModelNode
    {
    public:
        AssimpModelNode(const AssimpNode& assimp) noexcept;
        AssimpModelNode(const AssimpModelNode& other) noexcept;
        AssimpModelNode& operator=(const AssimpModelNode& other) noexcept;

        std::string_view getName() const noexcept override;
        glm::mat4 getTransform() const noexcept override;
        const ConstRefCollection<IModelNode>& getChildren() const noexcept override;
        void configureEntity(Entity entity, const ModelSceneConfig& config) const override;

    private:
        AssimpNode _assimp;
        AssimpModelNodeChildrenCollection _childrenCollection;
        std::vector<AssimpModelNode> _children;

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
        AssimpScene _assimp;
        AssimpModelNode _rootNode;
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