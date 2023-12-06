#include "scene.hpp"
#include <darmok/scene.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& pivot)
        : _position(position)
        , _rotation(rotation)
        , _scale(scale)
        , _pivot(pivot)
        , _changed(true)
    {
    }

    const glm::vec3& Transform::getPosition() const
    {
        return _position;
    }

    const glm::vec3& Transform::getRotation() const
    {
        return _rotation;
    }

    const glm::vec3& Transform::getScale() const
    {
        return _scale;
    }

    const glm::vec3& Transform::getPivot() const
    {
        return _pivot;
    }

    bool Transform::setPosition(const glm::vec3& v)
    {
        if (v == _position)
        {
            return false;
        }
        _changed = true;
        _position = v;
        return true;
    }

    bool Transform::setRotation(const glm::vec3& v)
    {
        if (v == _rotation)
        {
            return false;
        }
        _changed = true;
        _rotation = v;
        return true;
    }

    bool Transform::setScale(const glm::vec3& v)
    {
        if (v == _scale)
        {
            return false;
        }
        _changed = true;
        _scale = v;
        return true;
    }

    bool Transform::setPivot(const glm::vec3& v)
    {
        if (v == _pivot)
        {
            return false;
        }
        _changed = true;
        _pivot = v;
        return true;
    }

    bool Transform::update()
    {
        if (!_changed)
        {
            return false;
        }
        _matrix = glm::translate(-_pivot)
            * glm::scale(_scale)
            * glm::rotate(0.f, _rotation)
            * glm::translate(_pivot)
            * glm::translate(_position);
        return true;
    }

    const glm::mat4x4& Transform::getMatrix()
    {
        update();
        return _matrix;
    }

    const glm::mat4x4& Transform::getMatrix() const
    {
        return _matrix;
    }

    const uint8_t Colors::MaxValue = 255;
    const Color Colors::black = { 0, 0, 0, MaxValue };
    const Color Colors::white = { MaxValue, MaxValue, MaxValue, MaxValue };
    const Color Colors::red = { MaxValue, 0, 0, MaxValue };
    const Color Colors::green = { 0, MaxValue, 0, MaxValue };
    const Color Colors::blue = { 0, 0, MaxValue, MaxValue };

    bgfx::VertexLayout Sprite::createVertexLayout()
    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
        return layout;
    }

    bgfx::VertexLayout Sprite::_layout = Sprite::createVertexLayout();

    Sprite::Sprite(const bgfx::TextureHandle& texture, const std::vector<glm::vec2>& texCoords, const Color& color)
        : _texture(texture)
    {
        std::vector<SpriteVertex> vertices;
        vertices.reserve(texCoords.size());
        for (auto& coord : texCoords)
        {
            vertices.push_back({ coord, coord, color });
        }
        setVertices(vertices);
    }

    Sprite::Sprite(const bgfx::TextureHandle& texture, const Color& color)
        : _texture(texture)
    {
        setVertices({
            SpriteVertex{ glm::vec2(0, 0), glm::vec2(0, 0), color },
            SpriteVertex{ glm::vec2(1, 0), glm::vec2(1, 0), color },
            SpriteVertex{ glm::vec2(1, 1), glm::vec2(1, 1), color },
            SpriteVertex{ glm::vec2(0, 1), glm::vec2(0, 1), color },
        });
    }

    Sprite::Sprite(const bgfx::TextureHandle& texture, const std::vector<SpriteVertex>& vertices)
        : _texture(texture)
    {
        setVertices(vertices);
    }

    void Sprite::setVertices(const std::vector<SpriteVertex>& vertices)
    {
        bgfx::allocTransientVertexBuffer(&_vertexBuffer, vertices.size(), _layout);
        const SpriteVertex* src = &vertices.front();
        SpriteVertex* dst = (SpriteVertex*)_vertexBuffer.data;
        bx::memCopy(dst, src, vertices.size() * sizeof(SpriteVertex));
    }

    void Sprite::render(bgfx::Encoder* encoder, uint8_t textureUnit, uint8_t vertexStream)
    {
        encoder->setTexture(textureUnit, SceneImpl::getTexColorUniform(), _texture);
        encoder->setVertexBuffer(vertexStream, _vertexBuffer.handle, 0, _vertexBuffer.size);
    }

    const bgfx::TextureHandle& Sprite::getTexture() const
    {
        return _texture;
    }

    Program::Program(const bgfx::ProgramHandle& handle)
        : _handle(handle)
    {
    }

    const bgfx::ProgramHandle& Program::getHandle()
    {
        return _handle;
    }

    void Program::setHandle(const bgfx::ProgramHandle& handle)
    {
        _handle = handle;
    }

    Camera::Camera(const glm::mat4x4& v)
        : _matrix(v)
    {
    }

    void Camera::setMatrix(const glm::mat4x4& v)
    {
        _matrix = v;
    }

    const glm::mat4x4& Camera::getMatrix() const
    {
        return _matrix;
    }

    SceneImpl::SceneImpl(const bgfx::ProgramHandle& program)
        : _program(program)
    {
    }

    const bgfx::UniformHandle&  SceneImpl::getTexColorUniform()
    {
        static bgfx::UniformHandle uniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        return uniform;
    }

    Registry& SceneImpl::getRegistry()
    {
        return _registry;
    }

    const Registry& SceneImpl::getRegistry() const
    {
        return _registry;
    }

    void SceneImpl::render(bgfx::ViewId viewId)
    {
        auto cams = _registry.view<const Camera>();
        auto all = _registry.view<Entity>();
        bgfx::Encoder* encoder = bgfx::begin();

        uint64_t baseState = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_MSAA
            ;

        for (auto [entity, cam] : cams.each()) {

            auto projView = cam.getMatrix();
            auto trans = _registry.try_get<Transform>(entity);
            if (trans != nullptr)
            {
                projView *= trans->getMatrix();
            }
            for (auto [entity] : all.each()) {

                auto mtx = projView;
                auto trans = _registry.try_get<Transform>(entity);
                if (trans != nullptr)
                {
                    mtx *= trans->getMatrix();
                }
                encoder->setTransform(glm::value_ptr(mtx));

                uint64_t state = baseState;
                auto blend = _registry.try_get<Blend>(entity);
                if (blend != nullptr)
                {
                    state |= BGFX_STATE_BLEND_FUNC(blend->src, blend->dst);
                }
                encoder->setState(state);

                auto sprite = _registry.try_get<Sprite>(entity);
                if (sprite != nullptr)
                {
                    sprite->render(encoder);
                }

                auto progamHandle = _program;
                auto prog = _registry.try_get<Program>(entity);
                if (prog != nullptr)
                {
                    progamHandle = prog->getHandle();
                }
                encoder->submit(viewId, progamHandle);
            }
        }

        bgfx::end(encoder);
    }

    Scene::Scene(const bgfx::ProgramHandle& program)
    : _impl(std::make_unique<SceneImpl>(program))
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::createEntity()
    {
        return _impl->getRegistry().create();
    }

    Registry& Scene::getRegistry()
    {
        return _impl->getRegistry();
    }

    const Registry& Scene::getRegistry() const
    {
        return _impl->getRegistry();
    }

    void Scene::render(bgfx::ViewId viewId)
    {
        _impl->render(viewId);
    }

    SceneAppComponent::SceneAppComponent(const bgfx::ProgramHandle& program)
        : _scene(program)
    {
    }

    Scene& SceneAppComponent::getScene()
    {
        return _scene;
    }

    const Scene& SceneAppComponent::getScene() const
    {
        return _scene;
    }

    void SceneAppComponent::update(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window)
    {
        _scene.render(viewId);
    }
}