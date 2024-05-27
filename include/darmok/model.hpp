#pragma once

#include <memory>
#include <string_view>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/collection.hpp>
#include <glm/glm.hpp>

namespace darmok
{

    class AssetContext;

    struct ModelSceneConfig final
    {
        EntityRegistry& registry;
        bgfx::VertexLayout layout;
        AssetContext& assets;
    };

    class ModelNodeImpl;

    class ModelNode final
    {
    public:
        ModelNode(std::unique_ptr<ModelNodeImpl>&& impl) noexcept;

        DLLEXPORT std::string_view getName() const noexcept;
        DLLEXPORT glm::mat4 getTransform() const noexcept;
        DLLEXPORT const ValCollection<std::shared_ptr<ModelNode>>& getChildren() const noexcept;
        DLLEXPORT void configureEntity(Entity entity, const ModelSceneConfig& config) const;
    private:
        std::unique_ptr<ModelNodeImpl> _impl;
    };

    class ModelImpl;

    class Model final
    {
    public:
        Model(std::unique_ptr<ModelImpl>&& impl) noexcept;
        DLLEXPORT std::shared_ptr<ModelNode> getRootNode() const noexcept;
    private:
        std::unique_ptr<ModelImpl> _impl;
    };

    class ModelSceneConfigurer final
    {
    public:
        DLLEXPORT ModelSceneConfigurer(EntityRegistry& registry, const bgfx::VertexLayout& layout, AssetContext& assets);

        DLLEXPORT ModelSceneConfigurer& setParent(Entity parent) noexcept;

        DLLEXPORT Entity run(const std::shared_ptr<Model>& model) const noexcept;
        DLLEXPORT Entity run(const std::shared_ptr<ModelNode>& node) const noexcept;

        template<typename C>
        Entity run(const std::shared_ptr<ModelNode>& node, C callback) const
        {
            return run(node, _parent, callback);
        }

        template<typename C>
        Entity run(const std::shared_ptr<Model>& model, C callback) const
        {
            return run(model->getRootNode(), callback);
        }

    private:
        ModelSceneConfig _config;
        Entity _parent;

        DLLEXPORT Entity add(const std::shared_ptr<ModelNode>& node, Entity parent) const noexcept;
        DLLEXPORT Entity run(const std::shared_ptr<ModelNode>& node, Entity parent) const noexcept;

        template<typename C>
        Entity run(const std::shared_ptr<ModelNode>& node, Entity parent, C callback) const
        {
            auto entity = add(node, parent);
            callback(node, entity);
            for (auto child : node->getChildren())
            {
                run(child, entity, callback);
            }
            return entity;
        }
    };

    class BX_NO_VTABLE IModelLoader
	{
	public:
        using result_type = std::shared_ptr<Model>;

        DLLEXPORT virtual ~IModelLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

    class EmptyModelLoader : public IModelLoader
	{
	public:
        DLLEXPORT std::shared_ptr<Model> operator()(std::string_view name) override;
	};
}