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
#include <darmok/collection.hpp>
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
	struct Rectangle;
	struct TextureAtlasElement;

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

		static glm::mat4 convert(const Rml::Matrix4f& v) noexcept;
		static Rectangle convert(const Rml::Rectanglef& v) noexcept;

		static Color convert(const Rml::Colourf& v) noexcept;
		static Color convert(const Rml::Colourb& v) noexcept;
		static Rml::Colourb convert(const Color& v) noexcept;
	};

	class RmluiCanvasImpl;

	class RmluiRenderInterface final : public Rml::RenderInterface
	{
	public:
		RmluiRenderInterface(App& app) noexcept;
		~RmluiRenderInterface() noexcept;
		Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) noexcept override;
		void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) noexcept override;
		void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) noexcept override;

		void EnableScissorRegion(bool enable) noexcept override;
		void SetScissorRegion(Rml::Rectanglei region) noexcept override;

		Rml::TextureHandle LoadTexture(Rml::Vector2i& dimensions, const Rml::String& source) noexcept override;
		Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i dimensions) noexcept override;
		void ReleaseTexture(Rml::TextureHandle texture) noexcept override;
		void SetTransform(const Rml::Matrix4f* transform) noexcept override;

		void renderCanvas(RmluiCanvasImpl& canvas, IRenderGraphContext& context) noexcept;
		bool renderSprite(const Rml::Sprite& sprite, const glm::vec2& position) noexcept;
		OptionalRef<Texture> getSpriteTexture(const Rml::Sprite& sprite) noexcept;

	private:
		std::mutex _canvasMutex;
		App& _app;
		std::unique_ptr<Program> _program;
		bgfx::UniformHandle _textureUniform;
		std::unordered_map<Rml::String, std::reference_wrapper<Texture>> _textureSources;
		std::unordered_map<Rml::TextureHandle, std::unique_ptr<Texture>> _textures;
		std::unordered_map<Rml::CompiledGeometryHandle, std::unique_ptr<Mesh>> _meshes;
		glm::mat4 _trans;
		glm::mat4 _baseTrans;
		glm::ivec4 _scissor;
		bool _scissorEnabled;
		OptionalRef<IRenderGraphContext> _context;

		static const uint64_t _state;

		glm::mat4 getTransformMatrix(const glm::vec2& position);
	};

	class RmluiSystemInterface final : public Rml::SystemInterface
	{
	public:
		RmluiSystemInterface(App& app) noexcept;
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
		RmluiFileInterface(App& app);
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

		std::string getName() const noexcept;
		glm::mat4 getModelMatrix() const noexcept;
		glm::mat4 getProjectionMatrix() const noexcept;
		glm::mat4 getRenderMatrix() const noexcept;

		void setVisible(bool visible) noexcept;
		bool isVisible() const noexcept;

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

		bool isInputActive() const noexcept;
		void setInputActive(bool active) noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		void applyViewportMousePositionDelta(const glm::vec2& delta) noexcept;
		void setViewportMousePosition(const glm::vec2& position) noexcept;
		glm::vec2 getViewportMousePosition() const noexcept;

		const glm::vec2& getMousePosition() const noexcept;
		void setMousePosition(const glm::vec2& position) noexcept;

		void processKey(Rml::Input::KeyIdentifier key, int state, bool down) noexcept;
		void processTextInput(const Rml::String& text) noexcept;
		void processMouseLeave() noexcept;
		void processMouseWheel(const Rml::Vector2f& val, int keyState) noexcept;
		void processMouseButton(int num, int keyState, bool down) noexcept;

		OptionalRef<const Rml::Sprite> getMouseCursorSprite() const noexcept;

		void addListener(IRmluiCanvasListener& listener) noexcept;
		void addListener(std::unique_ptr<IRmluiCanvasListener>&& listener) noexcept;
		bool removeListener(const IRmluiCanvasListener& listener) noexcept;

		void onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element);
		bool loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine);
		void render(IRenderGraphContext& context) noexcept;
	private:
		RmluiCanvas& _canvas;
		OptionalRef<Rml::Context> _context;
		OptionalRef<RmluiRendererImpl> _comp;
		bool _inputActive;
		glm::vec2 _mousePosition;
		bool _visible;
		std::optional<glm::uvec2> _size;
		glm::vec3 _offset;
		std::string _name;
		MousePositionMode _mousePositionMode;
		OwnRefCollection<IRmluiCanvasListener> _listeners;

		OptionalRef<const Rml::Sprite> getMouseCursorSprite(Rml::ElementDocument& doc) const noexcept;
		void updateContextSize() noexcept;
		glm::mat4 getBaseModelMatrix() const noexcept;
	};

	class Transform;

	class RmluiEventForwarder final : public Rml::EventListener
	{
	public:
		RmluiEventForwarder(const std::string& value, Rml::Element& element) noexcept;
		~RmluiEventForwarder() noexcept;
		void remove() noexcept;
		void ProcessEvent(Rml::Event& event) override;
		Rml::Element& getElement() noexcept;
		const std::string& getValue() const noexcept;
	private:
		std::string _value;
		Rml::Element& _element;
		static const std::string _eventAttrPrefix;
	};

	class RmluiPlugin final : Rml::Plugin, Rml::EventListenerInstancer, Rml::ElementInstancer
	{
	public:
		~RmluiPlugin() noexcept;
		static std::shared_ptr<RmluiPlugin> getInstance(App& app) noexcept;
		static std::weak_ptr<RmluiPlugin> getWeakInstance() noexcept;
		RmluiSystemInterface& getSystem() noexcept;
		RmluiRenderInterface& getRender() noexcept;

		void addRenderer(RmluiRendererImpl& renderer) noexcept;
		bool removeRenderer(const RmluiRendererImpl& renderer) noexcept;

		void onCanvasDestroyed(RmluiCanvas& canvas) noexcept;
		void onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element);

		bool loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine = -1);
		bool loadExternalScript(Rml::ElementDocument& doc, const std::string& sourcePath);

	private:
		RmluiPlugin(App& app) noexcept;
		static std::weak_ptr<RmluiPlugin> _instance;
		App& _app;
		RmluiSystemInterface _system;
		RmluiFileInterface _file;
		RmluiRenderInterface _render;
		std::vector<std::unique_ptr<RmluiEventForwarder>> _eventForwarders;
		std::vector<std::reference_wrapper<RmluiRendererImpl>> _renderers;

		Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
		void OnDocumentUnload(Rml::ElementDocument* doc) noexcept override;

		Rml::ElementPtr InstanceElement(Rml::Element* parent, const Rml::String& tag, const Rml::XMLAttributes& attributes) noexcept override;
		void ReleaseElement(Rml::Element* element) noexcept override;
	};

	class RmluiDocument final : public Rml::ElementDocument
	{
	public:
		RmluiDocument(RmluiPlugin& plugin, const Rml::String& tag);
		void LoadInlineScript(const Rml::String& content, const Rml::String& sourcePath, int sourceLine) override;
		void LoadExternalScript(const Rml::String& sourcePath) override;
	private:
		RmluiPlugin& _plugin;
	};

    class RmluiRendererImpl final : IKeyboardListener, IMouseListener
    {
    public:
		~RmluiRendererImpl() noexcept;

		void init(Camera& cam, Scene& scene, App& app);
		void shutdown();
		void update(float dt) noexcept;
		void renderReset() noexcept;
		void beforeRenderView(IRenderGraphContext& context) noexcept;

		void onCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element);
		bool loadScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine);

		void loadFont(const std::string& path, bool fallback = false) noexcept;

		RmluiSystemInterface& getRmluiSystem() noexcept;
		RmluiRenderInterface& getRmluiRender() noexcept;
		bx::AllocatorI& getAllocator();
		OptionalRef<const Camera> getCamera() const noexcept;
		glm::uvec2 getViewportSize() const noexcept;
		OptionalRef<Transform> getTransform(const RmluiCanvas& canvas) noexcept;
		glm::mat4 getDefaultProjectionMatrix() const noexcept;
		RenderGraphDefinition& getRenderGraph() noexcept;
		const RenderGraphDefinition& getRenderGraph() const noexcept;
		int getKeyModifierState() const noexcept;
	private:
		OptionalRef<Camera> _cam;
		OptionalRef<Scene> _scene;
		OptionalRef<App> _app;
		RenderGraphDefinition _renderGraph;
		std::shared_ptr<RmluiPlugin> _plugin;

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