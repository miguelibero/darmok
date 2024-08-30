#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <RmlUi/Core.h>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/viewport.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/rmlui_fwd.hpp>
#include <unordered_map>
#include <variant>
#include <optional>
#include <memory>
#include <mutex>

namespace darmok
{
    class App;
	class Program;
	class Mesh;
	class Texture;
	struct Viewport;
	struct TextureAtlasBounds;

	struct RmluiUtils final
	{
		template<typename T>
		static Rml::Vector2<T> convert(const glm::vec<2, T>& v) noexcept
		{
			return { v.x, v.y };
		}

		template<typename T>
		static glm::vec<2, T> convert(const Rml::Vector2<T>& v) noexcept
		{
			return { v.x, v.y };
		}

		template<typename T>
		static Rml::Vector3<T> convert(const glm::vec<3, T>& v) noexcept
		{
			return { v.x, v.y, v.z };
		}

		template<typename T>
		static glm::vec<3, T> convert(const Rml::Vector3<T>& v) noexcept
		{
			return { v.x, v.y, v.z };
		}

		template<typename T>
		static Rml::Vector4<T> convert(const glm::vec<4, T>& v) noexcept
		{
			return { v.x, v.y, v.z, v.w };
		}

		template<typename T>
		static glm::vec<4, T> convert(const Rml::Vector4<T>& v) noexcept
		{
			return { v.x, v.y, v.z, v.w };
		}

		static Color convert(const Rml::Colourf& v) noexcept;
		static Color convert(const Rml::Colourb& v) noexcept;
		static Rml::Colourb convert(const Color& v) noexcept;
		static TextureAtlasBounds convert(const Rml::Rectangle& v) noexcept;
	};

	class RmluiRenderInterface final : public Rml::RenderInterface
	{
	public:
		RmluiRenderInterface(const std::shared_ptr<Program>& prog, bx::AllocatorI& alloc) noexcept;
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
		
		void beforeRender(IRenderGraphContext& context, const glm::mat4& sceneTransform, const glm::uvec2& canvasSize) noexcept;

		void renderMouseCursor(const Rml::Sprite& sprite, const glm::vec2& position) noexcept;
		void afterRender() noexcept;

	private:

		struct CompiledGeometry final
		{
			std::unique_ptr<Mesh> mesh;
			Rml::TextureHandle texture;
		};

		std::unordered_map<Rml::TextureHandle, std::unique_ptr<Texture>> _textures;
		std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> _compiledGeometries;
		std::optional<bgfx::ViewId> _viewId;

		OptionalRef<bgfx::Encoder> _encoder;
		std::shared_ptr<Program> _program;
		bgfx::UniformHandle _textureUniform;
		glm::mat4 _rmluiTransform;
		glm::mat4 _sceneTransform;
		glm::uvec2 _canvasSize;
		glm::ivec4 _scissor;
		bool _scissorEnabled;
		bx::AllocatorI& _alloc;
		static const uint64_t _state;

		void submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept;
		glm::mat4 getTransform(const glm::vec2& position);
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

	class RmluiComponentImpl;
	class Transform;

	class RmluiCanvasImpl final
	{
	public:
		using MousePositionMode = RmluiCanvasMousePositionMode;

		RmluiCanvasImpl(const std::string& name, std::optional<glm::uvec2> size);
		~RmluiCanvasImpl() noexcept;

		void init(RmluiComponentImpl& comp);
		void shutdown() noexcept;
		bool update() noexcept;
		void renderReset() noexcept;
		void render(IRenderGraphContext& context, const glm::mat4& trans) noexcept;

		std::string getName() const noexcept;

		void setEnabled(bool enabled) noexcept;
		bool getEnabled() const noexcept;

		void setMousePositionMode(MousePositionMode mode) noexcept;
		MousePositionMode getMousePositionMode() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		void setSize(std::optional<glm::uvec2> size) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		Rml::Context& getContext() noexcept;
		const Rml::Context& getContext() const noexcept;

		bool getInputActive() const noexcept;
		void setInputActive(bool active) noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		void addViewportMousePositionDelta(const glm::vec2& delta, OptionalRef<const Transform> canvasTrans) noexcept;
		void setViewportMousePosition(const glm::vec2& position, OptionalRef<const Transform> canvasTrans);

		void setMousePosition(const glm::vec2& position) noexcept;
		const glm::vec2& getMousePosition() const noexcept;

		void processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept;
		void processTextInput(const Rml::String& text) noexcept;
		void processMouseLeave() noexcept;
		void processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept;
		void processMouseButton(int num, int keyState, bool down) noexcept;

	private:
		OptionalRef<Rml::Context> _context;
		std::optional<RmluiRenderInterface> _render;
		OptionalRef<RmluiComponentImpl> _comp;
		bool _inputActive;
		glm::vec2 _mousePosition;
		bool _enabled;
		std::optional<glm::uvec2> _size;
		std::string _name;
		MousePositionMode _mousePositionMode;

		OptionalRef<const Rml::Sprite> getMouseCursorSprite() const noexcept;
		OptionalRef<const Rml::Sprite> getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept;

	};

    class RmluiComponentImpl final : IKeyboardListener, IMouseListener
    {
    public:
		~RmluiComponentImpl() noexcept;

		void init(Camera& cam, Scene& scene, App& app);
		void shutdown();
		void update(float dt) noexcept;
		void renderReset() noexcept;
		void beforeRenderView(IRenderGraphContext& context);

		RmluiSystemInterface& getSystem() noexcept;
		bx::AllocatorI& getAllocator();
		std::shared_ptr<Program> getProgram() const noexcept;
		OptionalRef<const Camera> getCamera() const noexcept;

		int getKeyModifierState() const noexcept;
		const glm::uvec2& getWindowPixelSize() const;

	private:
		RmluiSystemInterface _system;
		RmluiFileInterface _file;
		OptionalRef<Camera> _cam;
		OptionalRef<Scene> _scene;
		OptionalRef<App> _app;
		std::shared_ptr<Program> _program;

		void onCanvasConstructed(EntityRegistry& registry, Entity entity);
		void onCanvasDestroyed(EntityRegistry& registry, Entity entity);

		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept override;
		void onKeyboardChar(const Utf8Char& chr) noexcept override;
		void onMouseActive(bool active) noexcept override;
		void onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept override;
		void onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept override;
		void onMouseButton(MouseButton button, bool down) noexcept override;

		using KeyboardMap = std::unordered_map<KeyboardKey, Rml::Input::KeyIdentifier>;
		static const KeyboardMap& getKeyboardMap() noexcept;

		using KeyboardModifierMap = std::unordered_map<std::variant<KeyboardModifier, KeyboardKey>, Rml::Input::KeyModifier>;
		static const KeyboardModifierMap& getKeyboardModifierMap() noexcept;
    };
}