#pragma once

#include <memory>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Material;
    class Texture;
    class IMesh;

    class Renderable final
    {
    public:
        DLLEXPORT Renderable(const std::shared_ptr<IMesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        DLLEXPORT Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Texture>& texture) noexcept;
        DLLEXPORT Renderable(const std::shared_ptr<Material>& material) noexcept;
        DLLEXPORT std::shared_ptr<IMesh> getMesh() const noexcept;
        DLLEXPORT Renderable& setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
        DLLEXPORT std::shared_ptr<Material> getMaterial() const noexcept;
        DLLEXPORT Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        DLLEXPORT operator bool() const noexcept;
        DLLEXPORT void render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;
    private:
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };

}