
#include <darmok/quad.hpp>
#include <darmok/utils.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/material.hpp>

namespace darmok
{
    QuadComponent::QuadComponent(const std::shared_ptr<Material>& material, const glm::vec2& size, const glm::vec2& offset) noexcept
        : _material(material)
        , _size(size)
        , _offset(offset)
        , _vertexBuffer{ bgfx::kInvalidHandle }
        , _changed(false)
    {
    }

    QuadComponent::~QuadComponent() noexcept
    {
        bgfx::destroy(_vertexBuffer);
    }

    const glm::vec2& QuadComponent::getSize() const noexcept
    {
        return _size;
    }

    QuadComponent& QuadComponent::setSize(const glm::vec2& size) noexcept
    {
        if (_size != size)
        {
            _size = size;
            _changed = true;
        }
        return *this;
    }

    const glm::vec2& QuadComponent::getOffset() const noexcept
    {
        return _offset;
    }

    QuadComponent& QuadComponent::setOffset(const glm::vec2& offset) noexcept
    {
        if (_offset != offset)
        {
            _offset = offset;
            _changed = true;
        }
        return *this;
    }

    const std::shared_ptr<Material>& QuadComponent::getMaterial() const noexcept
    {
        return _material;
    }

    QuadComponent& QuadComponent::setMaterial(const std::shared_ptr<Material>& material) noexcept
    {
        if (_material != material)
        {
            _material = material;
            _changed = true;
        }
        return *this;
    }

    void QuadComponent::update() noexcept
    {
        auto& layout = _material->getVertexLayout();
        if (!isValid(_vertexBuffer))
        {
            _vertexBuffer = bgfx::createDynamicVertexBuffer(1, layout, BGFX_BUFFER_ALLOW_RESIZE);
            _changed = true;
        }

        if (!_changed)
        {
            return;
        }

        static const std::vector<glm::vec2> edges{
            { 1,  1 }, { 0,  1 }, { 0,  0 }, { 1,  0 },
        };

        std::vector<glm::vec2> positions;
        positions.reserve(edges.size());
        for (auto& edge : edges)
        {
            positions.push_back((edge * _size) + _offset);
        }

        // TODO: optimize using the same memory
        VertexDataWriter writer(layout, positions.size());
        writer.set(bgfx::Attrib::Position, positions);
        _vertices = std::move(writer.finish());
        bgfx::update(_vertexBuffer, 0, _vertices.makeRef());
        _changed = false;
    }

    void QuadComponent::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, uint8_t vertexStream) const
    {
        if (!isValid(_vertexBuffer))
        {
            throw std::runtime_error("invalid mesh vertex buffer");
        }
        encoder.setVertexBuffer(vertexStream, _vertexBuffer);
        _material->submit(encoder, viewId);
    }

    void QuadUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
    }

    void QuadUpdater::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        for (auto [entity, quad] : registry.view<QuadComponent>().each())
        {
            quad.update();
        }
    }

    QuadRenderer::QuadRenderer() noexcept
        : _lineIndexBuffer{ bgfx::kInvalidHandle }
        , _triIndexBuffer{ bgfx::kInvalidHandle }
    {
    }

    void QuadRenderer::init(Scene& scene, App& app) noexcept
    {
        CameraSceneRenderer::init(scene, app);
        static const std::vector<VertexIndex> lineIndices{ 0, 1, 1, 2, 2, 3, 3, 0 };
        static const std::vector<VertexIndex> triIndices{ 0, 1, 2, 2, 3, 0 };

        _lineIndexBuffer = bgfx::createIndexBuffer(makeVectorRef(lineIndices));
        _triIndexBuffer = bgfx::createIndexBuffer(makeVectorRef(triIndices));
    }


    void QuadRenderer::shutdown() noexcept
    {
        bgfx::destroy(_lineIndexBuffer);
        bgfx::destroy(_triIndexBuffer);
    }

    bgfx::IndexBufferHandle QuadRenderer::getIndexBuffer(MaterialPrimitiveType type) const
    {
        switch (type)
        {
        case MaterialPrimitiveType::Triangle:
            return _triIndexBuffer;
        case MaterialPrimitiveType::Line:
            return _lineIndexBuffer;
        }
        return { bgfx::kInvalidHandle };
    }

    bgfx::ViewId QuadRenderer::render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept
    {
        if (!_scene)
        {
            return viewId;
        }
        auto& registry = _scene->getRegistry();
        auto quads = cam.createEntityView<QuadComponent>(registry);
        auto rendered = false;
        for (auto entity : quads)
        {
            auto& quad = registry.get<const QuadComponent>(entity);
            if (quad.getMaterial() == nullptr)
            {
                continue;
            }
            rendered = true;
            Transform::bgfxConfig(entity, encoder, registry);
            auto indexBuffer = getIndexBuffer(quad.getMaterial()->getPrimitiveType());
            if (isValid(indexBuffer))
            {
                encoder.setIndexBuffer(indexBuffer);
            }
            quad.render(encoder, viewId);
        }
        if (rendered)
        {
            ++viewId;
        }
        return viewId;
    }
}