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

    class AssimpModelNodeChildrenCollection final : public ValCollection<std::shared_ptr<IModelNode>>
    {
    public:
        AssimpModelNodeChildrenCollection(const AssimpNode& parent) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] std::shared_ptr<IModelNode> operator[](size_t pos) const  override;
    private:
        std::vector<std::shared_ptr<AssimpModelNode>> _children;
    };

    class AssimpModelNode final : public IModelNode
    {
    public:
        AssimpModelNode(const std::shared_ptr<AssimpNode>& assimp) noexcept;

        std::string_view getName() const noexcept override;
        glm::mat4 getTransform() const noexcept override;
        const ValCollection<std::shared_ptr<IModelNode>>& getChildren() const noexcept override;
        void configureEntity(Entity entity, const ModelSceneConfig& config) const override;

    private:
        std::shared_ptr<AssimpNode> _assimp;
        AssimpModelNodeChildrenCollection _children;

        void configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept;
        void configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept;
    };

    class AssimpModel final : public IModel
    {
    public:
        AssimpModel(const AssimpScene& assimp) noexcept;
        std::shared_ptr<IModelNode> getRootNode() const noexcept override;

    private:
        AssimpScene _assimp;
        std::shared_ptr<AssimpModelNode> _rootNode;
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