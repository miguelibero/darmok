#pragma once

#include <bgfx/bgfx.h>
#include <RmlUi/Core.h>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include "embedded_shader.hpp"
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory>

namespace darmok
{
    class App;
	class Program;
	class Mesh;
	class Texture;

	class RmluiRender final : public Rml::RenderInterface
	{
	public:
		void RenderGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept override;
		void EnableScissorRegion(bool enable) noexcept override;
		void SetScissorRegion(int x, int y, int width, int height) noexcept override;

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) noexcept override;
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) noexcept override;
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) noexcept override;

		bool LoadTexture(Rml::TextureHandle& handle, Rml::Vector2i& textureDimensions, const Rml::String& source) noexcept override;
		bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& sourceDimensions) noexcept override;
		void ReleaseTexture(Rml::TextureHandle texture) noexcept override;
		void SetTransform(const Rml::Matrix4f* transform) noexcept override;
		
		void init(App& app);
		void beforeRender(bgfx::ViewId viewId) noexcept;
		void afterRender() noexcept;
		void shutdown() noexcept;

	private:

		struct CompiledGeometry final
		{
			std::unique_ptr<Mesh> mesh;
			std::unique_ptr<Texture> texture;
		};

		std::unordered_map< Rml::CompiledGeometryHandle, CompiledGeometry> _compiledGeometries;
		std::optional<bgfx::ViewId> _viewId;
		std::unique_ptr<Program> _program;
		bgfx::VertexLayout _layout;
		bgfx::UniformHandle _textureUniform;
		static const bgfx::EmbeddedShader _embeddedShaders[];
		glm::mat4 _transform;
	};

	class RmluiSystem final : public Rml::SystemInterface
	{
	public:
		RmluiSystem() noexcept;
		void init(App& app);
		void update(float dt) noexcept;
		double GetElapsedTime() override;
	private:
		double _elapsedTime;
	};

	class IDataLoader;

	class RmluiDataFile final : public Rml::FileInterface
	{
	public:
		void init(App& app);
		Rml::FileHandle Open(const Rml::String& path) noexcept override;
		void Close(Rml::FileHandle file) noexcept override;
		size_t Read(void* buffer, size_t size, Rml::FileHandle file) noexcept override;
		bool Seek(Rml::FileHandle file, long offset, int origin) noexcept override;
		size_t Tell(Rml::FileHandle file) noexcept override;
		size_t Length(Rml::FileHandle file) noexcept override;
		bool LoadFile(const Rml::String& path, Rml::String& out_data) noexcept override;
	private:
		struct Element
		{
			Data data;
			size_t position;
		};

		OptionalRef<IDataLoader> _dataLoader;
		using ElementMap = std::unordered_map<Rml::FileHandle, Element>;
		ElementMap _elements;

		OptionalRef<Element> find(Rml::FileHandle file) noexcept;
	};

    class RmluiAppComponentImpl final : public IWindowListener, public IKeyboardListener, public IMouseListener
    {
    public:
		OptionalRef<Rml::Context> getContext() const noexcept;

		void init(App& app);
		void shutdown() noexcept;
		bool render(bgfx::ViewId viewId) const noexcept;
		bool update(float dt) noexcept;

		void onWindowPixelSize(const glm::uvec2& size) noexcept override;
		void onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept override;
		void onKeyboardChar(const Utf8Char& chr) noexcept override;
		void onMouseActive(bool active) noexcept override;
		void onMousePositionChange(const glm::vec2& delta) noexcept override;
		void onMouseScrollChange(const glm::vec2& delta) noexcept override;
		void onMouseButton(MouseButton button, bool down) noexcept override;


	private:
		OptionalRef<Rml::Context> _context;
		mutable RmluiRender _render;
		RmluiSystem _system;
		RmluiDataFile _file;
		OptionalRef<App> _app;

		using KeyboardMap = std::unordered_map<KeyboardKey, Rml::Input::KeyIdentifier>;
		static const KeyboardMap& getKeyboardMap() noexcept;

		using KeyboardModifierMap = std::unordered_map<std::variant<uint8_t, KeyboardKey>, Rml::Input::KeyModifier>;
		static const KeyboardModifierMap& getKeyboardModifierMap() noexcept;

		int getKeyModifierState() const noexcept;
    };
}