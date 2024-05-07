#pragma once

#include <memory>
#include <string_view>
#include <stdexcept>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <darmok/scene_fwd.hpp>
#include <darmok/collection.hpp>
#include <glm/glm.hpp>

namespace darmok
{
    class ModelNodeImpl;

    class ModelNode final
    {
    public:
        ModelNode(std::unique_ptr<ModelNodeImpl>&& impl) noexcept;

        std::string_view getName() const;
        glm::mat4 getTransform() const;
        const ReadOnlyCollection<ModelNode>& getChildren() const;            
    private:
        std::unique_ptr<ModelNodeImpl> _impl;
    };

    class ModelImpl;

    class Model final
    {
    public:
        Model(std::unique_ptr<ModelImpl>&& impl) noexcept;
        ModelNode& getRootNode() noexcept;

    private:
        std::unique_ptr<ModelImpl> _impl;
    };

    class ITextureLoader;
    class Scene;

    class ModelSceneConfigurer
    {
    public:
        ModelSceneConfigurer(Scene& scene, const bgfx::VertexLayout& layout);
        
        ModelSceneConfigurer& setParent(Entity parent) noexcept;
        ModelSceneConfigurer& setTextureLoader(ITextureLoader& loader) noexcept;
        ModelSceneConfigurer& setAllocator(bx::AllocatorI* alloc) noexcept;

        Entity run(const Model& model) const noexcept;
        Entity run(const ModelNode& node) const noexcept;

        template<typename C>
        Entity run(const ModelNode& node, C callback) const
        {
            auto entity = run(node);
            callback(*this, entity);
            for (auto& child : node.getChildren())
            {
                run(child, callback);
            }
            return entity;
        }

        template<typename C>
        Entity run(const Model& model, C callback) const
        {
            return run(model.getRootNode(), callback);
        }

    private:
        Scene& scene;
        const bgfx::VertexLayout _layout;
        OptionalRef<ITextureLoader> _textureLoader;
        bx::AllocatorI* _alloc;
    };

    class BX_NO_VTABLE IModelLoader
	{
	public:
        virtual ~IModelLoader() = default;
		virtual std::shared_ptr<Model> operator()(std::string_view name) = 0;
	};

    class EmptyModelLoader : public IModelLoader
	{
	public:
		std::shared_ptr<Model> operator()(std::string_view name) override
        {
            throw std::runtime_error("no model implementation");
        }
	};
}