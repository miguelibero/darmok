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
    class ModelSceneConfigurer;

    struct ModelSceneConfig final
    {
        EntityRegistry& registry;
        bgfx::VertexLayout layout;
        AssetContext& assets;
    };

    class BX_NO_VTABLE IModelNode
    {
    public:
        IModelNode() = default;
        virtual ~IModelNode() = default;

        virtual std::string_view getName() const noexcept = 0;
        virtual glm::mat4 getTransform() const noexcept = 0;
        virtual const ConstRefCollection<IModelNode>& getChildren() const noexcept = 0;
        virtual void configureEntity(Entity entity, const ModelSceneConfig& config) const = 0;
    };

    class BX_NO_VTABLE IModel
    {
    public:
        IModel() = default;
        virtual ~IModel() = default;

        virtual IModelNode& getRootNode() noexcept = 0;
        virtual const IModelNode& getRootNode() const noexcept = 0;
    };

    class ModelSceneConfigurer final
    {
    public:
        ModelSceneConfigurer(EntityRegistry& registry, const bgfx::VertexLayout& layout, AssetContext& assets);

        ModelSceneConfigurer& setParent(Entity parent) noexcept;

        Entity run(const IModel& model) const noexcept;
        Entity run(const IModelNode& node) const noexcept;

        template<typename C>
        Entity run(const IModelNode& node, C callback) const
        {
            return run(node, _parent, callback);
        }

        template<typename C>
        Entity run(const IModel& model, C callback) const
        {
            return run(model.getRootNode(), callback);
        }

    private:
        ModelSceneConfig _config;
        Entity _parent;

        Entity add(const IModelNode& node, Entity parent) const noexcept;
        Entity run(const IModelNode& node, Entity parent) const noexcept;

        template<typename C>
        Entity run(const IModelNode& node, Entity parent, C callback) const
        {
            auto entity = add(node, parent);
            callback(node, entity);
            for (auto& child : node.getChildren())
            {
                run(child, entity, callback);
            }
            return entity;
        }
    };

    class BX_NO_VTABLE IModelLoader
	{
	public:
        virtual ~IModelLoader() = default;
		virtual std::shared_ptr<IModel> operator()(std::string_view name) = 0;
	};

    class EmptyModelLoader : public IModelLoader
	{
	public:
		std::shared_ptr<IModel> operator()(std::string_view name) override;
	};
}