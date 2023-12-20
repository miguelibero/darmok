#pragma once

#include <darmok/scene.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    class Material;
    class Model;
    class ModelMesh;

    Entity addModelToScene(Scene& scene, const Model& model, Entity entity = 0);

    class MeshData final
    {
    public:
        MeshData(const std::shared_ptr<Material>& material, std::vector<float>&& vertices, bgfx::VertexLayout layout, std::vector<VertexIndex>&& indices) noexcept;

        static std::shared_ptr<MeshData> fromModel(const ModelMesh& modelMesh, const std::string& basePath = {});

        const std::shared_ptr<Material>& getMaterial() const;
        const bgfx::VertexBufferHandle& getVertexBuffer() const;
        const bgfx::IndexBufferHandle& getIndexBuffer() const;
    private:
        std::shared_ptr<Material> _material;
        std::vector<float> _vertices;
        std::vector<VertexIndex> _indices;
        VertexBuffer _vertexBuffer;
        IndexBuffer _indexBuffer;

        MeshData(const MeshData& other) noexcept = delete;
        MeshData& operator=(const MeshData& other) noexcept = delete;
    };

    class Mesh final
    {
    public:
        Mesh(const std::vector<std::shared_ptr<MeshData>>& data = {});
        const std::vector<std::shared_ptr<MeshData>>& getData() const;
        void setData(const std::vector<std::shared_ptr<MeshData>>& data);
    private:
        std::vector<std::shared_ptr<MeshData>> _datas;
    };

    class MeshRenderer final : public ISceneRenderer
    {
    public:
        MeshRenderer(bgfx::ProgramHandle program = { bgfx::kInvalidHandle });
        ~MeshRenderer();
        void init(Registry& registry) override;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry) override;
    private:
        bgfx::ProgramHandle _program;
        bgfx::UniformHandle _texColorUniforn;

        void renderData(const MeshData& data, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry);
    };
}