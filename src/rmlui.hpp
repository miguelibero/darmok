#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <RmlUi/Core.h>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/viewport.hpp>
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
	struct Viewport;

	class RmluiRenderInterface final : public Rml::RenderInterface
	{
	public:
		RmluiRenderInterface() noexcept;
		~RmluiRenderInterface() noexcept;
		void RenderGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept override;
		void EnableScissorRegion(bool enable) noexcept override;
		void SetScissorRegion(int x, int y, int width, int height) noexcept override;

		Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture) noexcept override;
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation) noexcept override;
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) noexcept override;

		bool LoadTexture(Rml::TextureHandle& handle, Rml::Vector2i& dimensions, const Rml::String& source) noexcept override;
		bool GenerateTexture(Rml::TextureHandle& handle, const Rml::byte* source, const Rml::Vector2i& dimensions) noexcept override;
		void ReleaseTexture(Rml::TextureHandle texture) noexcept override;
		void SetTransform(const Rml::Matrix4f* transform) noexcept override;
		
		void init(App& app);
		void beforeRender(bgfx::ViewId viewId, const Viewport& vp) noexcept;
		void renderMouseCursor(const Rml::Sprite& sprite, const glm::vec2& position) noexcept;
		bool afterRender() noexcept;
		void shutdown() noexcept;
		void setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		std::shared_ptr<Texture> getTargetTexture() noexcept;

	private:

		struct CompiledGeometry final
		{
			std::unique_ptr<Mesh> mesh;
			Rml::TextureHandle texture;
		};

		std::unordered_map<Rml::TextureHandle, std::unique_ptr<Texture>> _textures;
		std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> _compiledGeometries;
		std::optional<bgfx::ViewId> _viewId;
		Viewport _viewport;

		OptionalRef<bgfx::Encoder> _encoder;
		std::unique_ptr<Program> _program;
		bgfx::VertexLayout _layout;
		bgfx::UniformHandle _textureUniform;
		glm::mat4 _transform;
		glm::ivec4 _scissor;
		bool _scissorEnabled;
		bool _rendered;
		bool _viewSetup;
		std::shared_ptr<Texture> _targetTexture;
		bgfx::FrameBufferHandle _frameBuffer;
		OptionalRef<bx::AllocatorI> _alloc;
		static const uint64_t _state;

		void submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept;
		void setupView() noexcept;

	};

	class RmluiSystemInterface final : public Rml::SystemInterface
	{
	public:
		RmluiSystemInterface() noexcept;
		void init(App& app);
		void update(float dt) noexcept;
		double GetElapsedTime() override;

		const std::string& getMouseCursor() const noexcept;
		void SetMouseCursor(const Rml::String& name) noexcept override;
	private:
		double _elapsedTime;
		std::string _mouseCursor;
	};

	class IDataLoader;

	class RmluiFileInterface final : public Rml::FileInterface
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

	class BX_NO_VTABLE IRmluiListener
	{
	public:
		virtual ~IRmluiListener() noexcept = default;
		virtual void onRmluiInitialized() = 0;
		virtual void onRmluiContextCreated(Rml::Context& context) = 0;
		virtual void onRmluiShutdown() = 0;
	};

    class RmluiAppComponentImpl final : public IWindowListener, public IKeyboardListener, public IMouseListener
    {
    public:
		RmluiAppComponentImpl(const std::string& name) noexcept;
		RmluiAppComponentImpl(const std::string& name, const Viewport& viewport) noexcept;
		~RmluiAppComponentImpl() noexcept;

		void setViewport(const std::optional<Viewport>& viewport) noexcept;
		const std::optional<Viewport>& getViewport() const noexcept;
		Viewport getCurrentViewport() const noexcept;

		void setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		std::shared_ptr<Texture> getTargetTexture() const noexcept;

		void setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;
		void setMousePosition(const glm::vec2& position) noexcept;

		OptionalRef<Rml::Context> getContext() const noexcept;
		RmluiRenderInterface& getRenderInterface() noexcept;

		void init(App& app);
		void shutdown() noexcept;
		bool render(bgfx::ViewId viewId) const noexcept;
		bool update(float dt) noexcept;

		void onWindowPixelSize(const glm::uvec2& size) noexcept override;
		void onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept override;
		void onKeyboardChar(const Utf8Char& chr) noexcept override;
		void onMouseActive(bool active) noexcept override;
		void onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept override;
		void onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept override;
		void onMouseButton(MouseButton button, bool down) noexcept override;

	private:
		RmluiSystemInterface _system;
		RmluiFileInterface _file;

		std::string _name;
		OptionalRef<Rml::Context> _context;
		OptionalRef<App> _app;
		
		mutable RmluiRenderInterface _render;
		std::optional<Viewport> _viewport;
		bool _inputActive;

		glm::vec2 _mousePosition;
		std::string _defaultMouseCursor;

		using KeyboardMap = std::unordered_map<KeyboardKey, Rml::Input::KeyIdentifier>;
		static const KeyboardMap& getKeyboardMap() noexcept;

		using KeyboardModifierMap = std::unordered_map<std::variant<uint8_t, KeyboardKey>, Rml::Input::KeyModifier>;
		static const KeyboardModifierMap& getKeyboardModifierMap() noexcept;

		int getKeyModifierState() const noexcept;
		OptionalRef<const Rml::Sprite> getMouseCursorSprite() const noexcept;
		OptionalRef<const Rml::Sprite> getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept;
    };
}