#include <darmok/mesh_core.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/shape_serialize.hpp>
#include <darmok/data.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/protobuf/program.pb.h>
#include <glm/gtx/component_wise.hpp>

#include "detail/mesh_core.hpp"

namespace darmok
{
	uint16_t MeshConfig::getFlags() const noexcept
	{
		uint16_t flags = 0;
		if (index32)
		{
			flags |= BGFX_BUFFER_INDEX32;
		}
		return flags;
	}

	size_t MeshConfig::getIndexSize() const noexcept
	{
		if (index32)
		{
			return 4;
		}
		return sizeof(VertexIndex);
	}

	MeshConfig MeshConfig::fromDefinition(const Definition& def) noexcept
	{
		return {
			.type = def.type(),
			.index32 = def.index32()
		};
	}

	Mesh::StaticVariant::StaticVariant(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config)
	{
		auto flags = config.getFlags();
		if (!vertices.empty())
		{
			vertexBuffer = bgfx::createVertexBuffer(vertices.copyMem(), layout, flags);
		}
		if (!indices.empty())
		{
			indexBuffer = bgfx::createIndexBuffer(indices.copyMem(), flags);
		}
	}

	Mesh::StaticVariant::~StaticVariant() noexcept
	{
		if (isValid(vertexBuffer))
		{
			bgfx::destroy(vertexBuffer);
		}
		if (isValid(indexBuffer))
		{
			bgfx::destroy(indexBuffer);
		}
	}

	Mesh::StaticVariant::StaticVariant(StaticVariant&& other)
		: vertexBuffer{ other.vertexBuffer }
		, indexBuffer{ other.indexBuffer }
	{
		other.vertexBuffer.idx = bgfx::kInvalidHandle;
		other.indexBuffer.idx = bgfx::kInvalidHandle;
	}

	Mesh::StaticVariant& Mesh::StaticVariant::operator=(StaticVariant&& other)
	{
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;
		other.vertexBuffer.idx = bgfx::kInvalidHandle;
		other.indexBuffer.idx = bgfx::kInvalidHandle;
		return *this;
	}

	bool Mesh::StaticVariant::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		if (!isValid(vertexBuffer))
		{
			return false;
		}
		encoder.setVertexBuffer(config.vertexStream, vertexBuffer, config.startVertex, config.numVertices);
		if (isValid(indexBuffer))
		{
			encoder.setIndexBuffer(indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
	}

	Mesh::DynamicVariant::DynamicVariant(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config)
	{
		auto flags = config.getFlags() | BGFX_BUFFER_ALLOW_RESIZE;
		if (!vertices.empty())
		{
			vertexBuffer = bgfx::createDynamicVertexBuffer(vertices.copyMem(), layout, flags);
		}
		if (!indices.empty())
		{
			indexBuffer = bgfx::createDynamicIndexBuffer(indices.copyMem(), flags);
		}
	}

	Mesh::DynamicVariant::~DynamicVariant() noexcept
	{
		if (isValid(vertexBuffer))
		{
			bgfx::destroy(vertexBuffer);
		}
		if (isValid(vertexBuffer))
		{
			bgfx::destroy(indexBuffer);
		}
	}

	Mesh::DynamicVariant::DynamicVariant(DynamicVariant&& other)
		: vertexBuffer{ other.vertexBuffer }
		, indexBuffer{ other.indexBuffer }
	{
		other.vertexBuffer.idx = bgfx::kInvalidHandle;
		other.indexBuffer.idx = bgfx::kInvalidHandle;
	}

	Mesh::DynamicVariant& Mesh::DynamicVariant::operator=(DynamicVariant&& other)
	{
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;
		other.vertexBuffer.idx = bgfx::kInvalidHandle;
		other.indexBuffer.idx = bgfx::kInvalidHandle;
		return *this;
	}

	bool Mesh::DynamicVariant::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		if (!isValid(vertexBuffer))
		{
			return false;
		}
		encoder.setVertexBuffer(config.vertexStream, vertexBuffer, config.startVertex, config.numVertices);
		if (isValid(indexBuffer))
		{
			encoder.setIndexBuffer(indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
	}

	Mesh::TransientVariant::TransientVariant(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config)
	{
		if (!vertices.empty())
		{
			auto vertNum = static_cast<uint32_t>(vertices.size()) / layout.getStride();
			if (vertNum > 0 && !bgfx::getAvailTransientVertexBuffer(vertNum, layout))
			{
				throw std::runtime_error("not enought transient vertex buffer space");
			}
			bgfx::allocTransientVertexBuffer(&vertexBuffer, vertNum, layout);
			bx::memCopy(vertexBuffer.data, vertices.ptr(), vertices.size());
		}
		else
		{
			vertexBuffer.data = nullptr;
			vertexBuffer.size = 0;
			vertexBuffer.handle.idx = bgfx::kInvalidHandle;
		}
		if (!indices.empty())
		{
			auto idxNum = static_cast<uint32_t>(indices.size() / config.getIndexSize());
			auto index32 = config.index32;
			if (!bgfx::getAvailTransientIndexBuffer(idxNum, index32))
			{
				throw std::runtime_error("not enought transient index buffer space");
			}
			bgfx::allocTransientIndexBuffer(&indexBuffer, idxNum, index32);
			bx::memCopy(indexBuffer.data, indices.ptr(), indices.size());
		}
		else
		{
			indexBuffer.data = nullptr;
			indexBuffer.size = 0;
			indexBuffer.handle.idx = bgfx::kInvalidHandle;
		}
	}

	bool Mesh::TransientVariant::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		encoder.setVertexBuffer(config.vertexStream, &vertexBuffer, config.startVertex, config.numVertices);
		if (config.numIndices > 0)
		{
			encoder.setIndexBuffer(&indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
	}

	Mesh::Variant Mesh::createVariant(Type type, const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config)
	{
				auto flags = config.getFlags();
		switch (config.type)
		{
			case Definition::Dynamic:
				return DynamicVariant(layout, vertices, indices, config);
			case Definition::Transient:
				return TransientVariant(layout, vertices, indices, config);
			default:
				return StaticVariant(layout, vertices, indices, config);
		}
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept
		: _type{ config.type }
		, _variant{ createVariant(_type, layout, vertices, indices, config) }
		, _layout{ layout }
		, _vertNum{ layout.getStride() != 0 ? vertices.size() / layout.getStride() : 0 }
		, _idxNum{ indices.size() / config.getIndexSize() }
	{
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, DataView vertices, Config config) noexcept
		: Mesh(layout, vertices, DataView{}, config)
	{
	}

	Mesh::Mesh(const Definition& def)
		: Mesh(ConstVertexLayoutWrapper{ def.layout() }.getBgfx(),
			DataView{ def.vertices() }, DataView{ def.indices() }, Config::fromDefinition(def))
	{
	}

	const bgfx::VertexLayout& Mesh::getVertexLayout() const noexcept
	{
		return _layout;
	}

	Mesh::Source Mesh::createSource() noexcept
	{
		Source src;
		src.mutable_program()->set_standard(protobuf::StandardProgram::Forward);
		src.set_name("Default Shape");
		auto& sphere = *src.mutable_sphere();
		sphere.mutable_shape()->set_radius(1.f);
		sphere.set_lod(32);

		return src;
	}

	std::string Mesh::toString() const noexcept
	{
		auto stride = _layout.getStride();
		return "Mesh(" + std::to_string(_vertNum) + " vertices, "
			+ std::to_string(stride) + " stride, "
			+ std::to_string(_idxNum) + " indices)";
	}

	void MeshRenderConfig::fix(uint32_t maxVertices, uint32_t maxIndices) noexcept
	{
		auto max = maxVertices - startVertex;
		if (numVertices == 0 || numVertices > max)
		{
			numVertices = max;
		}
		max = maxIndices - startVertex;
		if (numIndices == 0 || numIndices > max)
		{
			numIndices = max;
		}
	}

	bool Mesh::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		config.fix(static_cast<uint32_t>(_vertNum), static_cast<uint32_t>(_idxNum));
		if (config.numVertices == 0)
		{
			return false;
		}
		return std::visit([&](const auto& data) { return data.render(encoder, config); }, _variant);
	}

	bool Mesh::empty() const noexcept
	{
		return _vertNum == 0;
	}

	uint16_t Mesh::getVertexHandleIndex() const noexcept
	{
		return std::visit([](const auto& data) {
			if constexpr (std::is_same_v<std::decay_t<decltype(data)>, StaticVariant>)
			{
				return data.vertexBuffer.idx;
			}
			else if constexpr (std::is_same_v<std::decay_t<decltype(data)>, DynamicVariant>)
			{
				return data.vertexBuffer.idx;
			}
			else
			{
				return data.vertexBuffer.handle.idx;
			}
			}, _variant);
	}

	void Mesh::updateVertices(DataView data, uint32_t offset)
	{
		auto& buffer = std::get<DynamicVariant>(_variant).vertexBuffer;
		bgfx::update(buffer, offset, data.copyMem());
	}

	void Mesh::updateIndices(DataView data, uint32_t offset)
	{
		auto& buffer = std::get<DynamicVariant>(_variant).indexBuffer;
		bgfx::update(buffer, offset, data.copyMem());
	}	

	MeshData::MeshData(MeshType type) noexcept
		: type{ type }
	{
	}

	const bgfx::VertexLayout& MeshData::getDefaultVertexLayout() noexcept
	{
		static bgfx::VertexLayout layout;
		if (layout.getStride() == 0)
		{
			layout.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
		}
		return layout;
	}

	bool MeshData::empty() const noexcept
	{
		return vertices.empty();
	}

	void MeshData::clear() noexcept
	{
		vertices.clear();
		indices.clear();
	}

	MeshData& MeshData::setName(std::string_view name) noexcept
	{
		_name = name;
		return *this;
	}

	const std::string& MeshData::getName() const noexcept
	{
		return _name;
	}

	void MeshData::exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept
	{
		VertexDataWriter writer{ vertexLayout, static_cast<uint32_t>(vertices.size()) };

		uint32_t i = 0;
		for (auto& vertex : vertices)
		{
			writer.write(bgfx::Attrib::Position, i, vertex.position);
			writer.write(bgfx::Attrib::TexCoord0, i, vertex.texCoord);
			writer.write(bgfx::Attrib::Normal, i, vertex.normal);
			writer.write(bgfx::Attrib::Tangent, i, vertex.tangent);
			writer.write(bgfx::Attrib::Color0, i, vertex.color);

			glm::vec4 weights{ 1, 0, 0, 0 };
			glm::vec4 indices{ -1 };
			int j = 0;
			auto weightVector = vertex.weights;
			std::sort(weightVector.begin(), weightVector.end(), [](auto& a, auto& b) { return a.value > b.value; });
			for (auto& weight : weightVector)
			{
				if (weight.value <= 0.f)
				{
					continue;
				}
				indices[j] = weight.boneIndex;
				weights[j] = weight.value;
				if (++j > 3)
				{
					// TODO: maybe return error?
					break;
				}
			}
			writer.write(bgfx::Attrib::Indices, i, indices);
			writer.write(bgfx::Attrib::Weight, i, weights);

			++i;
		}

		vertexData = writer.finish();
		
		// TODO: support int32 indices
		indexData = DataView{ indices };
	}

	Mesh::Definition MeshData::createDefinition(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config) const
	{
		Mesh::Definition def;
		def.set_name(_name);
		def.set_type(type);
		def.set_index32(config.index32);

		VertexLayoutWrapper{ *def.mutable_layout() }.read(vertexLayout);

		Data vertices;
		Data indices;
		exportData(vertexLayout, vertices, indices);
		*def.mutable_vertices() = std::move(vertices).toString();
		*def.mutable_indices() = std::move(indices).toString();
		*def.mutable_bounds() = convert<protobuf::BoundingBox>(getBounds());
		return def;
	}

	Mesh MeshData::createMesh(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config) const
	{
		return createDefinition(vertexLayout, config);
	}

	std::shared_ptr<Mesh::Definition> MeshData::createSharedDefinition(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config) const
	{
		return std::make_shared<Mesh::Definition>(createDefinition(vertexLayout, config));
	}

	std::shared_ptr<Mesh> MeshData::createSharedMesh(const bgfx::VertexLayout& vertexLayout, const Mesh::Config& config) const
	{
		return std::make_shared<Mesh>(createDefinition(vertexLayout, config));
	}

	const std::vector<MeshData::Index> MeshData::_cuboidTriangleIndices
	{
		0,  1,  2,  2,  3,  0,
		4,  5,  6,  6,  7,  4,
		8,  9, 10, 10, 11,  8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	MeshData::MeshData(const Cube& cube, RectangleType type) noexcept
	{
		const static std::vector<Vertex> basicVertices = {
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 0,  1,  1 }, { 1, 0 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 0,  0,  1 }, { 1, 1 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 1,  0,  1 }, { 0, 1 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 1,  1,  0 }, { 0, 0 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 1,  0,  0 }, { 1, 0 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 0,  1,  0 }, { 0, 1 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 1,  1,  0 }, { 1, 0 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 0,  1,  0 }, { 1, 1 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 0,  1,  1 }, { 0, 1 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 1,  0,  1 }, { 0, 0 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 0,  0,  1 }, { 1, 0 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 1,  0,  0 }, { 0, 1 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  0,  1 }, { 1, 0 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  0,  0 }, { 1, 1 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  1,  0 }, { 0, 1 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 0,  1,  1 }, { 0, 0 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  1,  0 }, { 1, 0 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  0,  1 }, { 0, 1 }, { -1,  0,  0 }, {  0, -1,  0 } }
		};

		vertices = basicVertices;
		indices = _cuboidTriangleIndices;

		if (type == Mesh::Definition::OutlineRectangle)
		{
			convertQuadIndicesToLine();
		}

		if (cube.size.x == 0.F || cube.size.y == 0.F || cube.size.z == 0.F)
		{
			for (auto& vertex : vertices)
			{
				vertex.position = cube.origin;
			}
			return;
		}

		auto trans = glm::scale(glm::mat4(1), cube.size);
		trans = glm::translate(trans, (cube.origin / cube.size) - glm::vec3(0.5f));
		*this *= trans;
	}

	void MeshData::setupBasicRectangle() noexcept
	{
		static const std::vector<MeshData::Vertex> basicVertices = {
			{ {  0.5F,  0.5F, 0 }, { 1, 0 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ {  0.5F, -0.5F, 0 }, { 1, 1 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ { -0.5F, -0.5F, 0 }, { 0, 1 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ { -0.5F,  0.5F, 0 }, { 0, 0 }, { 0, 0, 1 }, { 1, 0, 0} }
		};
		static const std::vector<Index> basicIndices = { 0, 1, 2, 2, 3, 0 };
		vertices = basicVertices;
		indices = basicIndices;
	}

	MeshData::MeshData(const Rectangle& rect, RectangleType type) noexcept
	{
		setupBasicRectangle();

		if (type == Mesh::Definition::OutlineRectangle)
		{
			convertQuadIndicesToLine();
		}

		if (rect.size.x == 0.F || rect.size.y == 0.F)
		{
			for (auto& vertex : vertices)
			{
				vertex.position = glm::vec3(rect.origin, 0);
			}
			return;
		}

		auto trans = glm::scale(glm::mat4(1), glm::vec3(rect.size, 1.F));
		trans = glm::translate(trans, glm::vec3(rect.origin / rect.size, 0));
		*this *= trans;
	}

	MeshData::MeshData(const Plane& plane, RectangleType type, float scale) noexcept
	{
		setupBasicRectangle();

		if (type == Mesh::Definition::OutlineRectangle)
		{
			convertQuadIndicesToLine();
		}

		scalePositions(glm::vec3(scale));

		*this *= plane.getTransform();
	}

	MeshData::MeshData(const Frustum& frust, RectangleType type) noexcept
	{
		vertices = {
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 0, 1 } }
		};

		indices = _cuboidTriangleIndices;

		if (type == Mesh::Definition::OutlineRectangle)
		{
			convertQuadIndicesToLine();
		}
		else
		{
			calcNormals();
			calcTangents();
		}
	}

	MeshData& MeshData::convertQuadIndicesToLine() noexcept
	{
		std::vector<Index> v(indices);
		indices.clear();
		indices.reserve(v.size() * 8 / 6);
		for (size_t i = 0; i < v.size() - 5; i += 6)
		{
			indices.push_back(v[i]);
			indices.push_back(v[i + 1]);
			indices.push_back(v[i + 1]);
			indices.push_back(v[i + 2]);
			indices.push_back(v[i + 2]);
			indices.push_back(v[i + 4]);
			indices.push_back(v[i + 4]);
			indices.push_back(v[i]);
		}

		return *this;
	}

	MeshData& MeshData::subdivide(size_t amount) noexcept
	{
		vertices.reserve(vertices.size() * (3 + amount) / 3);
		indices.reserve(indices.size() * (1 + amount));
		for (size_t i = 0; i < amount; ++i)
		{
			auto size = indices.size() / 3;
			for (auto j = 0; j < size; ++j)
			{
				doSubdivide(j);
			}
		}
		return *this;
	}

	size_t MeshData::subdivideDensity(float maxDistance) noexcept
	{
		float maxv = -bx::kFloatInfinity;
		bool found = true;
		size_t count = 0;
		while(found)
		{
			found = false;
			for (auto i = 0; i < indices.size() / 3; ++i)
			{
				if (doSubdivide(i, maxDistance))
				{
					found = true;
					++count;
					break;
				}
			}
		}
		return count;
	}

	bool MeshData::doSubdivide(size_t i, float maxDistance) noexcept
	{
		i *= 3;
		const Index i1 = indices[i];
		const Index i2 = indices[i + 1];
		const Index i3 = indices[i + 2];
		const Index i4 = vertices.size();
		const Vertex v1 = vertices[i1];
		const Vertex v2 = vertices[i2];
		const Vertex v3 = vertices[i3];

		auto d = glm::vec3(
			glm::distance(v2.position, v1.position),
			glm::distance(v3.position, v2.position),
			glm::distance(v1.position, v3.position)
		);
		auto maxd = glm::compMax(d);
		if (maxd < maxDistance)
		{
			return false;
		}

		auto& v4 = vertices.emplace_back();
		if (maxd == d.x)
		{
			v4 = mix(v1, v2, 0.5F);
			indices[i + 1] = i4;
			indices.push_back(i4);
			indices.push_back(i2);
			indices.push_back(i3);
		}
		else if (maxd == d.y)
		{
			v4 = mix(v2, v3, 0.5F);
			indices[i + 2] = i4;
			indices.push_back(i1);
			indices.push_back(i4);
			indices.push_back(i3);
		}
		else if (maxd == d.z)
		{
			v4 = mix(v3, v1, 0.5F);
			indices[i + 2] = i4;
			indices.push_back(i4);
			indices.push_back(i2);
			indices.push_back(i3);
		}

		return true;
	}

	MeshData::Vertex MeshData::mix(const Vertex& v1, const Vertex& v2, float f) noexcept
	{
		return {
			.position = glm::mix(v1.position, v2.position, f),
			.texCoord = glm::mix(v1.texCoord, v2.texCoord, f),
			.normal = glm::normalize(glm::mix(v1.normal, v2.normal, f)),
			.tangent = glm::normalize(glm::mix(v1.tangent, v2.tangent, f)),
			.color = glm::mix(v1.color, v2.color, f),
		};
	}

	MeshData::MeshData(const Sphere& sphere, unsigned int lod) noexcept
		: MeshData(Capsule(0, sphere.radius, sphere.origin), lod)
	{
	}

	MeshData::MeshData(const Capsule& capsule, unsigned int lod) noexcept
	{
		unsigned int radialSegments = lod;
		unsigned int capSegments = lod;
		auto halfLength = capsule.cylinderHeight / capsule.radius * 0.5F;

		auto calcCapVertices = [this, capSegments, radialSegments, halfLength](bool topCap)
		{
			int baseIndex = static_cast<int>(vertices.size());
			float f = topCap ? 1.F : -1.F;
			for (int i = 0; i <= capSegments; ++i)
			{
				float v = float(i) / capSegments;
				float theta = v * glm::half_pi<float>();
				float sinTheta = glm::sin(theta);
				float cosTheta = glm::cos(theta);

				for (int j = 0; j <= radialSegments; ++j)
				{
					float u = float(j) / radialSegments;
					if (!topCap)
					{
						u = 1.F - u;
					}
					float phi = u * glm::two_pi<float>();
					float sinPhi = glm::sin(phi);
					float cosPhi = glm::cos(phi);

					auto normPos = glm::vec3(
						cosPhi * cosTheta,
						f * sinTheta,
						sinPhi * cosTheta);

					auto tangent = f * glm::vec3(-sinPhi, 0, cosPhi);
					vertices.emplace_back(
						normPos + glm::vec3(0, f * halfLength, 0),
						glm::vec2(u, topCap ? v : 1.0f - v),
						normPos,
						tangent
					);

					if (j < radialSegments && i < capSegments)
					{
						int current = baseIndex + i * (radialSegments + 1) + j;
						int next = current + radialSegments + 1;

						indices.push_back(current);
						indices.push_back(next);
						indices.push_back(current + 1);

						indices.push_back(current + 1);
						indices.push_back(next);
						indices.push_back(next + 1);
					}
				}
			}
		};
		
		calcCapVertices(true);
		calcCapVertices(false);

		// body
		int baseIndex = static_cast<int>(vertices.size());
		for (int i = 0; i < 2; ++i)
		{
			float v = float(i);
			float y = ((v * 2.F) - 1.F) * halfLength;

			for (int j = 0; j <= radialSegments; ++j)
			{
				float u = float(j) / radialSegments;
				float phi = u * glm::two_pi<float>();
				float sinPhi = glm::sin(phi);
				float cosPhi = glm::cos(phi);

				vertices.emplace_back(
					glm::vec3(cosPhi, y, sinPhi),
					glm::vec2(u, (1.F + v) * 0.5F),
					glm::vec3(cosPhi, 0, sinPhi),
					glm::vec3(-sinPhi, 0, cosPhi)
				);

				if (j < radialSegments && i == 0)
				{
					int current = baseIndex + j;
					int next = current + radialSegments + 1;

					indices.push_back(current);
					indices.push_back(next);
					indices.push_back(current + 1);

					indices.push_back(current + 1);
					indices.push_back(next);
					indices.push_back(next + 1);
				}
			}
		}

		auto trans = glm::translate(glm::mat4(capsule.radius), capsule.origin / capsule.radius);
		*this *= trans;
    }

	 MeshData::MeshData(const Ray& ray) noexcept
		 : MeshData(ray.toLine(), Mesh::Definition::Arrow)
	 {
	 }

	MeshData::MeshData(const Line& line, LineType type) noexcept
	{
		if (type == Mesh::Definition::Line)
		{
			vertices.emplace_back(line.points[0], glm::vec2{ 0, 0 });
			vertices.emplace_back(line.points[1], glm::vec2{ 1, 1 });
			return;
		}
		static const float kInter = .2f;
		
		static const std::vector<glm::vec3> pos = {
			{  0.f,  0.f, 1.f    }, {  .1f,  .1f, kInter },
			{ -.1f,  .1f, kInter }, { -.1f, -.1f, kInter },
			{  .1f, -.1f, kInter }, {  0.f,  0.f, 0.f }
		};

		static const std::vector<glm::vec2> tex = {
			{ 0.f, 0.f	}, { .1f, .1f },
			{ .1f, -.1f }, { -.1f, -.1f },
			{ -.1f, .1f }, { 0.f, 0.f }
		};
		vertices = {
			{
				{ pos[0], tex[0] }, { pos[2], tex[1] }, { pos[1], tex[1] },
				{ pos[5], tex[0] }, { pos[1], tex[2] }, { pos[2], tex[2] },
				{ pos[0], tex[0] }, { pos[3], tex[3] }, { pos[2], tex[3] },
				{ pos[5], tex[0] }, { pos[2], tex[4] }, { pos[3], tex[4] },
				{ pos[0], tex[2] }, { pos[4], tex[5] }, { pos[3], tex[2] },
				{ pos[5], tex[3] }, { pos[3], tex[5] }, { pos[4], tex[3] },
				{ pos[0], tex[4] }, { pos[1], tex[5] }, { pos[4], tex[4] },
				{ pos[5], tex[1] }, { pos[4], tex[5] }, { pos[1], tex[1] }
			}
		};

		auto trans = line.getTransform();

		for (auto& vertex : vertices)
		{
			vertex.position = trans * glm::vec4(vertex.position, 1.0);
		}

		calcNormals();
		calcTangents();
	}

	MeshData::MeshData(const Triangle& tri) noexcept
	{
		auto n = tri.getNormal();
		vertices = {
			{ 
				{ tri.vertices[0], { 0, 0 }, n },
				{ tri.vertices[1], { 1, 0 }, n },
				{ tri.vertices[2], { 1, 1 }, n }
			}
		};
		calcTangents();
	}

	MeshData::MeshData(const Polygon& poly) noexcept
	{
		for (auto& tri : poly.triangles)
		{
			*this += MeshData(tri);
		}
		*this *= glm::translate(glm::mat4(1), poly.origin);
	}

	MeshData::MeshData(const Grid& grid) noexcept
	{
		auto dx = grid.getAlong();		
		auto dy = glm::cross(dx, grid.normal);
		dx *= grid.separation.x;
		dy *= grid.separation.y;
		auto amount = glm::vec2(grid.amount) * 0.5F;
		vertices.reserve(2 * (size_t(grid.amount.x) + grid.amount.y));

		auto addVertex = [&](float x, float y)
		{
			auto p = grid.origin + (dx * x) + (dy * y);
			vertices.emplace_back(p, glm::vec2(x, y), grid.normal);
		};

		for (float x = -amount.x; x <= amount.x; ++x)
		{
			auto i = vertices.size();
			addVertex(x, -amount.y);
			addVertex(x, +amount.y);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
		for (float y = -amount.y; y <= amount.y; ++y)
		{
			auto i = vertices.size();
			addVertex(-amount.x, y);
			addVertex(amount.x, y);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
	}

	MeshData::MeshData(const Definition& def) noexcept
	{
		if (def.has_sphere())
		{
			*this = { def.sphere() };
		}
		else if (def.has_cube())
		{
			*this = { def.cube() };
		}
		else if (def.has_capsule())
		{
			*this = { def.capsule() };
		}
		else if (def.has_rectangle())
		{
			*this = { def.rectangle() };
		}
		else if (def.has_data())
		{
			*this = { def.data() };
		}
		_name = def.name();
	}

	MeshData::MeshData(const DataDefinition& def) noexcept
	: indices{ def.indices().begin(), def.indices().end() }
	{
		std::unordered_map<size_t, std::vector<MeshDataWeight>> weightsByVertex;

		size_t boneIndex = 0;
		for (auto& bone : def.bones())
		{
			for (auto& weight : bone.weights())
			{
				auto v = weight.value();
				if (v <= 0.f)
				{
					continue;
				}

				auto i = weight.vertex_id();
				weightsByVertex[i].push_back(MeshDataWeight
					{
						.boneIndex = boneIndex,
						.value = v
					});
			}
			++boneIndex;
		}

		vertices.reserve(def.vertices_size());
		size_t vertexIndex = 0;
		for (auto& v : def.vertices())
		{
			auto itr = weightsByVertex.find(vertexIndex);
			std::vector<MeshDataWeight> weights;
			if (itr != weightsByVertex.end())
			{
				weights = std::move(itr->second);
			}

			vertices.push_back({
				.position = convert<glm::vec3>(v.position()),
				.texCoord = convert<glm::vec2>(v.tex_coord()),
				.normal = convert<glm::vec3>(v.normal()),
				.tangent = convert<glm::vec3>(v.tangent()),
				.color = convert<Color>(v.color()),
				.weights = std::move(weights)
				});

			++vertexIndex;
		}
	}

	MeshData::MeshData(const CubeDefinition& def) noexcept
		: MeshData(convert<Cube>(def.shape()), def.type())
	{
	}

	MeshData::MeshData(const SphereDefinition& def) noexcept
		: MeshData(convert<Sphere>(def.shape()), def.lod())
	{
	}

	MeshData::MeshData(const CapsuleDefinition& def) noexcept
		: MeshData(convert<Capsule>(def.shape()), def.lod())
	{
	}

	MeshData::MeshData(const RectangleDefinition& def) noexcept
		: MeshData(convert<Rectangle>(def.shape()), def.type())
	{
	}

	MeshData MeshData::operator+(const MeshData& other) const noexcept
	{
		MeshData r{ *this };
		r += other;
		return r;
	}

	void MeshData::doCreateIndices(std::vector<Index>& indices, size_t size) noexcept
	{
		indices.reserve(size);
		for (size_t i = 0; i < size; ++i)
		{
			indices.push_back(i);
		}
	}

	MeshData& MeshData::createIndices() noexcept
	{
		doCreateIndices(indices, vertices.size());
		return *this;
	}

	MeshData& MeshData::operator+=(const MeshData& other) noexcept
	{
		auto offset = vertices.size();
		auto otherIndices = other.indices;
		if (indices.empty() && !otherIndices.empty())
		{
			createIndices();
		}
		if (!indices.empty() && otherIndices.empty())
		{
			doCreateIndices(otherIndices, other.vertices.size());
		}
		indices.reserve(indices.size() + otherIndices.size());
		for (auto& idx : otherIndices)
		{
			indices.push_back(offset + idx);
		}
		vertices.reserve(vertices.size() + other.vertices.size());
		vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());

		return *this;
	}

	MeshData& MeshData::operator*=(const glm::mat4& trans) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position = trans * glm::vec4(vertex.position, 1.F);
			vertex.normal = trans * glm::vec4(vertex.normal, 0.F);
			vertex.normal = glm::normalize(vertex.normal);
			vertex.tangent = trans * glm::vec4(vertex.tangent, 0.F);
			vertex.tangent = glm::normalize(vertex.tangent);
		}
		return *this;
	}

	MeshData& MeshData::operator*=(const Color& color) noexcept
	{
		auto ncolor = Colors::normalize(color);
		for (auto& vertex : vertices)
		{
			vertex.color = glm::vec4(vertex.color) * ncolor;
		}
		return *this;
	}

	MeshData& MeshData::scalePositions(const glm::vec3& scale) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position *= scale;
		}
		return *this;
	}

	MeshData& MeshData::translatePositions(const glm::vec3& pos) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position += pos;
		}
		return *this;
	}

	MeshData& MeshData::scaleTexCoords(const glm::vec2& scale) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord *= scale;
		}
		return *this;
	}
	MeshData& MeshData::translateTexCoords(const glm::vec2& pos) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord += pos;
		}
		return *this;
	}

	MeshData& MeshData::setColor(const Color& color) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.color = color;
		}
		return *this;
	}

	MeshData& MeshData::shiftIndices(Index offset) noexcept
	{
		for (auto& idx : indices)
		{
			idx += offset;
		}
		return *this;
	}


	MeshDataCalcTangentsOperation::MeshDataCalcTangentsOperation() noexcept
		: _iface{}
	{
		_iface.m_getNumFaces = getNumFaces;
		_iface.m_getNumVerticesOfFace = getNumFaceVertices;
		_iface.m_getNormal = getNormal;
		_iface.m_getPosition = getPosition;
		_iface.m_getTexCoord = getTexCoords;
		_iface.m_setTSpaceBasic = setTangent;
		_iface.m_setTSpace = nullptr;
		_context.m_pInterface = &_iface;
	}

	void MeshDataCalcTangentsOperation::operator()(MeshData& mesh) noexcept
	{
		_context.m_pUserData = &mesh;
		genTangSpaceDefault(&_context);
	}

	MeshData& MeshDataCalcTangentsOperation::getMeshDataFromContext(const SMikkTSpaceContext* context) noexcept
	{
		return *static_cast<MeshData*>(context->m_pUserData);
	}

	int MeshDataCalcTangentsOperation::getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		auto faceSize = getNumFaceVertices(context, iFace);
		auto index = (iFace * faceSize) + iVert;
		if (mesh.indices.empty())
		{
			return index;
		}
		return mesh.indices[index];
	}

	int MeshDataCalcTangentsOperation::getNumFaces(const SMikkTSpaceContext* context) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		if (mesh.indices.empty())
		{
			return static_cast<int>(mesh.vertices.size()) / 3;
		}
		return static_cast<int>(mesh.indices.size()) / 3;
	}

	int MeshDataCalcTangentsOperation::getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept
	{
		return 3;
	}

	void MeshDataCalcTangentsOperation::getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		auto index = getVertexIndex(context, iFace, iVert);
		auto& vert = mesh.vertices[index];
		outpos[0] = vert.position.x;
		outpos[1] = vert.position.y;
		outpos[2] = vert.position.z;
	}

	void MeshDataCalcTangentsOperation::getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		auto index = getVertexIndex(context, iFace, iVert);
		auto& vert = mesh.vertices[index];
		outnormal[0] = vert.normal.x;
		outnormal[1] = vert.normal.y;
		outnormal[2] = vert.normal.z;
	}

	void MeshDataCalcTangentsOperation::getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		auto index = getVertexIndex(context, iFace, iVert);
		auto& vert = mesh.vertices[index];
		outuv[0] = vert.texCoord.x;
		outuv[1] = vert.texCoord.y;
	}

	void MeshDataCalcTangentsOperation::setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept
	{
		MeshData& mesh = getMeshDataFromContext(context);
		auto index = getVertexIndex(context, iFace, iVert);
		auto& vert = mesh.vertices[index];
		vert.tangent.x = tangentu[0];
		vert.tangent.y = tangentu[1];
		vert.tangent.z = tangentu[2];
	}

	MeshData& MeshData::calcTangents() noexcept
	{
		MeshDataCalcTangentsOperation op;
		op(*this);
		return *this;
	}

	MeshData& MeshData::calcNormals() noexcept
	{
		for (auto& face : getFaces())
		{
			auto& vert1 = vertices[face[0]];
			auto& vert2 = vertices[face[1]];
			auto& vert3 = vertices[face[2]];
			auto edge1 = vert2.position - vert1.position;
			auto edge2 = vert3.position - vert1.position;

			auto normal = glm::cross(edge1, edge2);
			normal = glm::normalize(normal);

			vert1.normal = normal;
			vert2.normal = normal;
			vert3.normal = normal;
		}
		return *this;
	}

	std::vector<MeshData::Face> MeshData::getFaces() const noexcept
	{
		std::vector<Face> faces;
		if (indices.empty())
		{
			faces.reserve(vertices.size() / 3);
			for (Index i = 0; i < vertices.size(); i += 3)
			{
				Face face = { (Index)i, (Index)(i + 1), (Index)(i + 2) };
				faces.push_back(std::move(face));
			}
		}
		else
		{
			faces.reserve(indices.size() / 3);
			for (size_t i = 0; i < indices.size() - 2; i += 3)
			{
				Face face = { indices[i], indices[i + 1], indices[i + 2] };
				faces.push_back(std::move(face));
			}
		}

		return faces;
	}

	BoundingBox MeshData::getBounds() const noexcept
	{
		BoundingBox bb;

		if (indices.empty())
		{
			if (!vertices.empty())
			{
				bb.min = vertices[0].position;
				bb.max = bb.min;
			}
			for (auto& vert : vertices)
			{
				bb.expandToPosition(vert.position);
			}
		}
		else
		{
			bb.min = vertices[indices[0]].position;
			bb.max = bb.min;

			for (auto& idx : indices)
			{
				auto& vert = vertices[idx];
				bb.expandToPosition(vert.position);
			}
		}

		return bb;
	}

}
