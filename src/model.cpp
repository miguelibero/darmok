#include <darmok/model.hpp>
#include <darmok/transform.hpp>
#include <stdexcept>

namespace darmok
{
    ModelSceneConfigurer::ModelSceneConfigurer(EntityRegistry& registry, const bgfx::VertexLayout& layout, AssetContext& assets)
        : _config{ registry, layout, assets }
        , _parent(entt::null)
    {
    }

    ModelSceneConfigurer& ModelSceneConfigurer::setPath(const std::string& path) noexcept
    {
        _config.path = path;
        return *this;
    }

    ModelSceneConfigurer& ModelSceneConfigurer::setParent(Entity parent) noexcept
    {
        _parent = parent;
        return *this;
    }

    Entity ModelSceneConfigurer::run(const IModel& model) const noexcept
    {
        return run(model.getRootNode());
    }

    Entity ModelSceneConfigurer::run(const IModelNode& node, Entity parent) const noexcept
    {
        auto entity = add(node, parent);
        for (auto& child : node.getChildren())
        {
            run(child, entity);
        }
        return entity;
    }

    Entity ModelSceneConfigurer::add(const IModelNode& node, Entity parent) const noexcept
    {
        auto entity = _config.registry.create();
        OptionalRef<Transform> parentTrans;
		if (parent != entt::null)
		{
			parentTrans = _config.registry.get_or_emplace<Transform>(parent);
		}
		_config.registry.emplace<Transform>(entity, node.getTransform(), parentTrans);
        node.configureEntity(entity, _config);
        return entity;
    }

    std::shared_ptr<IModel> EmptyModelLoader::operator()(std::string_view name)
    {
        throw std::runtime_error("no model implementation");
    }
}