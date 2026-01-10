#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene.hpp>
#include <darmok/rmlui_fwd.hpp>
#include <darmok/collection.hpp>
#include <darmok/protobuf/rmlui.pb.h>
#include <bx/bx.h>
#include <string>
#include <optional>
#include <RmlUi/Core/ScrollTypes.h>
#include <RmlUi/Core/EventListener.h>

namespace Rml
{
	class Context;
	class Element;
	class ElementDocument;
	class Event;
	class DataModelConstructor;
}

namespace darmok
{
	class RmluiAppComponent;
	class Texture;
	class Camera;
	class Transform;

	class DARMOK_EXPORT BX_NO_VTABLE IRmluiCanvasDelegate
	{
	public:
		virtual ~IRmluiCanvasDelegate() = default;
		virtual entt::id_type getRmluiCanvasDelegateType() const noexcept { return 0; };
		virtual void update(float deltaTime) { };
		virtual void onRmluiCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element) = 0;
		virtual bool loadRmluiScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine) = 0;
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeRmluiCanvasDelegate : public IRmluiCanvasDelegate
	{
	public:
		entt::id_type getRmluiCanvasDelegateType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class RmluiCanvasImpl;

	class DARMOK_EXPORT RmluiCanvas final
	{
	public:
		using Definition = protobuf::RmluiCanvas;

		using MousePositionMode = RmluiCanvasMousePositionMode;

		RmluiCanvas(const std::string& name = "", const std::optional<glm::uvec2>& size = std::nullopt) noexcept;
		RmluiCanvas(const Definition& def) noexcept;
		~RmluiCanvas() noexcept;

		const std::string& getName() const noexcept;

		RmluiCanvas& setCamera(const OptionalRef<Camera>& camera) noexcept;
		const OptionalRef<Camera>& getCamera() const noexcept;
		OptionalRef<Camera> getCurrentCamera() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		RmluiCanvas& setSize(const std::optional<glm::uvec2>& vp) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		const glm::vec3& getOffset() const noexcept;
		RmluiCanvas& setOffset(const glm::vec3& offset) noexcept;

		RmluiCanvas& setMousePositionMode(MousePositionMode mode) noexcept;
		MousePositionMode getMousePositionMode() const noexcept;

		RmluiCanvas& setVisible(bool visible) noexcept;
		bool isVisible() const noexcept;

		RmluiCanvas& setInputEnabled(bool enabled) noexcept;
		bool isInputEnabled() const noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		RmluiCanvas& setMousePosition(const glm::vec2& position) noexcept;
		const glm::vec2& getMousePosition() const noexcept;

		RmluiCanvas& applyViewportMousePositionDelta(const glm::vec2& delta) noexcept;
		RmluiCanvas& setViewportMousePosition(const glm::vec2& position) noexcept;
		glm::vec2 getViewportMousePosition() const noexcept;

		Rml::Context& getContext() noexcept;
		const Rml::Context& getContext() const noexcept;

		RmluiCanvasImpl& getImpl() noexcept;
		const RmluiCanvasImpl& getImpl() const noexcept;

		RmluiCanvas& setDelegate(IRmluiCanvasDelegate& dlg) noexcept;
		RmluiCanvas& setDelegate(std::unique_ptr<IRmluiCanvasDelegate>&& dlg) noexcept;
		OptionalRef<IRmluiCanvasDelegate> getDelegate() const noexcept;

		Rml::DataModelConstructor createDataModel(const std::string& name);
		Rml::DataModelConstructor getDataModel(const std::string& name) noexcept;
		bool removeDataModel(const std::string& name) noexcept;

		uint64_t getDefaultTextureFlags() const noexcept;
		RmluiCanvas& setDefaultTextureFlags(uint64_t flags) noexcept;
		uint64_t getTextureFlags(const std::string& source) const noexcept;
		RmluiCanvas& setTextureFlags(const std::string& source, uint64_t flags) noexcept;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;

	private:
		std::unique_ptr<RmluiCanvasImpl> _impl;
	};

	class RmluiSceneComponentImpl;

	class DARMOK_EXPORT RmluiSceneComponent : public ITypeSceneComponent<RmluiSceneComponent>
	{
	public:
		using Definition = protobuf::RmluiSceneComponent;

		RmluiSceneComponent() noexcept;
		~RmluiSceneComponent() noexcept;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;

		expected<void, std::string> init(Scene& scene, App& app) noexcept override;
		expected<void, std::string> shutdown() noexcept override;
		expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
		expected<void, std::string> update(float deltaTime) noexcept override;
	private:
		std::unique_ptr<RmluiSceneComponentImpl> _impl;
	};

	class RmluiRendererImpl;

	class DARMOK_EXPORT RmluiRenderer final : public ITypeCameraComponent<RmluiRenderer>
	{
	public:
		using Definition = protobuf::RmluiRenderer;

		RmluiRenderer() noexcept;
		~RmluiRenderer() noexcept;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;

		expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
		expected<void, std::string> shutdown() noexcept override;
		expected<void, std::string> render() noexcept override;
		expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
		expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
	private:
		std::unique_ptr<RmluiRendererImpl> _impl;
	};
}