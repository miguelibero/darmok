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
    class ModelNodeImpl final
    {
    public:
        ModelNodeImpl(AssimpNode& node, AssimpScene& scene) noexcept;
        std::string_view getName() const noexcept;
        glm::mat4 getTransform() const noexcept;
        const ReadOnlyCollection<ModelNode>& getChildren() const noexcept;  
        const AssimpScene& getScene() const noexcept;          
    private:
        AssimpNode& _node;
        AssimpScene& _scene;
    };

    class ITextureLoader;

    class ModelImpl final
    {
    public:
        ModelImpl(AssimpScene&& scene, const std::string& path = {}) noexcept;
        ModelNode& getRootNode() noexcept;
    private:
        AssimpScene _scene;
    };

    class IDataLoader;

    class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader);
		std::shared_ptr<Model> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
	};
}