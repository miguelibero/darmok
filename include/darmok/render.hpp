#pragma once

#include <darmok/export.h>
#include <memory>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Material;
    class Texture;
    class IMesh;
    class Program;

    class DARMOK_EXPORT Renderable final
    {
    public:
        Renderable(const std::shared_ptr<IMesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<IMesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        bool isEnabled() const noexcept;
        Renderable& setEnabled(bool enabled) noexcept;
        bool valid() const noexcept;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;
    private:
        bool _enabled;
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}