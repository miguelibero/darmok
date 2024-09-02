#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <RmlUi/Core.h>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
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

	class RmluiCanvasImpl;

	class RmluiRenderInterface final : public Rml::RenderInterface, IRenderPass
	{
	public:
		RmluiRenderInterface(RmluiCanvasImpl& canvas) noexcept;

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
		
		void renderReset() noexcept;

		void renderPassDefine(RenderPassDefinition& def) noexcept override;
		void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
		void renderPassExecute(IRenderGraphContext& context) noexcept override;

	private:

		struct CompiledGeometry final
		{
			std::unique_ptr<Mesh> mesh;
			Rml::TextureHandle texture;
		};

		RmluiCanvasImpl& _canvas;
		std::unordered_map<Rml::TextureHandle, std::unique_ptr<Texture>> _textures;
		std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> _compiledGeometries;
		glm::mat4 _transform;
		glm::ivec4 _scissor;
		bool _scissorEnabled;
		OptionalRef<IRenderGraphContext> _context;

		static const uint64_t _state;

		void submitGeometry(Rml::TextureHandle texture, const Rml::Vector2f& translation) noexcept;
		glm::mat4 getTransformMatrix(const glm::vec2& position);
		void renderMouseCursor(const Rml::Sprite& sprite, const glm::vec2& position) noexcept;

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

	class RmluiRendererImpl;
	class RmluiCanvas;
	class Transform;

	class RmluiCanvasImpl final
	{
	public:
		using MousePositionMode = RmluiCanvasMousePositionMode;

		RmluiCanvasImpl(RmluiCanvas& canvas, const std::string& name, const std::optional<glm::uvec2>& size = std::nullopt);
		~RmluiCanvasImpl() noexcept;

		void init(RmluiRendererImpl& comp);
		void shutdown() noexcept;
		bool update() noexcept;
		void renderReset() noexcept;
		void render(IRenderGraphContext& context, const glm::mat4& trans) noexcept;

		std::string getName() const noexcept;
		glm::mat4 getModelMatrix() const noexcept;
		glm::mat4 getProjectionMatrix() const noexcept;

		void setEnabled(bool enabled) noexcept;
		bool getEnabled() const noexcept;

		void setMousePositionMode(MousePositionMode mode) noexcept;
		MousePositionMode getMousePositionMode() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		void setSize(const std::optional<glm::uvec2>& vp) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		const glm::vec3& getOffset() const noexcept;
		void setOffset(const glm::vec3& offset) noexcept;

		Rml::Context& getContext();
		const Rml::Context& getContext() const;

		OptionalRef<RmluiRendererImpl> getComponent() noexcept;
		OptionalRef<const RmluiRendererImpl> getComponent() const noexcept;

		bool getInputActive() const noexcept;
		void setInputActive(bool active) noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		void addViewportMousePositionDelta(const glm::vec2& delta) noexcept;
		void setViewportMousePosition(const glm::vec2& position);

		const glm::vec2& getMousePosition() const noexcept;
		void setMousePosition(const glm::vec2& position) noexcept;

		void processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept;
		void processTextInput(const Rml::String& text) noexcept;
		void processMouseLeave() noexcept;
		void processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept;
		void processMouseButton(int num, int keyState, bool down) noexcept;

		OptionalRef<const Rml::Sprite> getMouseCursorSprite() const noexcept;

	private:
		RmluiCanvas& _canvas;
		OptionalRef<Rml::Context> _context;
		std::optional<RmluiRenderInterface> _render;
		OptionalRef<RmluiRendererImpl> _comp;
		bool _inputActive;
		glm::vec2 _mousePosition;
		bool _enabled;
		std::optional<glm::uvec2> _size;
		glm::vec3 _offset;
		std::string _name;
		MousePositionMode _mousePositionMode;

		OptionalRef<const Rml::Sprite> getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept;
		void updateContextSize() noexcept;
	};

	class Transform;

    class RmluiRendererImpl final : IKeyboardListener, IMouseListener
    {
    public:
		~RmluiRendererImpl() noexcept;

		void init(Camera& cam, Scene& scene, App& app);
		void shutdown();
		void update(float dt) noexcept;
		void renderReset() noexcept;

		RmluiSystemInterface& getSystem() noexcept;
		bx::AllocatorI& getAllocator();
		std::shared_ptr<Program> getProgram() const noexcept;
		OptionalRef<const Camera> getCamera() const noexcept;
		glm::uvec2 getViewportSize() const noexcept;
		OptionalRef<Transform> getTransform(const RmluiCanvas& canvas) noexcept;
		glm::mat4 getDefaultProjectionMatrix() const noexcept;
		const bgfx::UniformHandle& getTextureUniform() const noexcept;
		RenderGraphDefinition& getRenderGraph() noexcept;
		const RenderGraphDefinition& getRenderGraph() const noexcept;

		int getKeyModifierState() const noexcept;

	private:
		RmluiSystemInterface _system;
		RmluiFileInterface _file;
		OptionalRef<Camera> _cam;
		OptionalRef<Scene> _scene;
		OptionalRef<App> _app;
		std::shared_ptr<Program> _program;
		RenderGraphDefinition _renderGraph;
		bgfx::UniformHandle _textureUniform;

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