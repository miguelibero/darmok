#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene.hpp>
#include <darmok/rmlui_fwd.hpp>
#include <darmok/collection.hpp>
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
		virtual entt::id_type getType() const noexcept { return 0; };
		virtual void update(float deltaTime) { };
		virtual void onRmluiCustomEvent(Rml::Event& event, const std::string& value, Rml::Element& element) = 0;
		virtual bool loadRmluiScript(Rml::ElementDocument& doc, std::string_view content, std::string_view sourcePath, int sourceLine) = 0;
	};

	class RmluiCanvasImpl;

	class DARMOK_EXPORT RmluiCanvas final
	{
	public:
		using MousePositionMode = RmluiCanvasMousePositionMode;

		RmluiCanvas(const std::string& name, const std::optional<glm::uvec2>& size = std::nullopt) noexcept;
		~RmluiCanvas() noexcept;

		const std::string& getName() const noexcept;

		RmluiCanvas& setMainCamera(Camera& camera) noexcept;
		OptionalRef<Camera> getMainCamera() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		RmluiCanvas& setSize(const std::optional<glm::uvec2>& vp) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		const glm::vec3& getOffset() const noexcept;
		RmluiCanvas& setOffset(const glm::vec3& offset) noexcept;

		RmluiCanvas& setMousePositionMode(MousePositionMode mode) noexcept;
		MousePositionMode getMousePositionMode() const noexcept;

		RmluiCanvas& setVisible(bool visible) noexcept;
		bool isVisible() const noexcept;

		RmluiCanvas& setInputActive(bool active) noexcept;
		bool isInputActive() const noexcept;

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

	private:
		std::unique_ptr<RmluiCanvasImpl> _impl;
	};

	class RmluiSceneComponentImpl;

	class DARMOK_EXPORT RmluiSceneComponent : public ISceneComponent
	{
	public:
		RmluiSceneComponent() noexcept;
		~RmluiSceneComponent() noexcept;

		void init(Scene& scene, App& app) noexcept override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;
		void update(float deltaTime) noexcept override;
	private:
		std::unique_ptr<RmluiSceneComponentImpl> _impl;
	};

	class RmluiRendererImpl;

	class DARMOK_EXPORT RmluiRenderer final : public ICameraComponent
	{
	public:
		RmluiRenderer() noexcept;
		~RmluiRenderer() noexcept;

		void init(Camera& cam, Scene& scene, App& app) noexcept override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;
		void beforeRenderView(IRenderGraphContext& context) noexcept override;
	private:
		std::unique_ptr<RmluiRendererImpl> _impl;
	};
}