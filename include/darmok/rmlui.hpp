#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <bx/bx.h>
#include <string>
#include <optional>
#include <RmlUi/Core/ScrollTypes.h>

namespace Rml
{
	class Context;
}

namespace darmok
{
	class RmluiAppComponent;
	class Texture;
	class Camera;
	class Transform;
	struct Viewport;

	class RmluiCanvasImpl;

	class DARMOK_EXPORT RmluiCanvas final
	{
	public:
		RmluiCanvas(const std::string& name, std::optional<glm::vec2> size = std::nullopt) noexcept;
		~RmluiCanvas() noexcept;

		std::string getName() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		RmluiCanvas& setSize(const std::optional<glm::uvec2>& size) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		RmluiCanvas& setEnabled(bool enabled) noexcept;
		bool getEnabled() const noexcept;

		RmluiCanvas& setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		RmluiCanvas& setMousePosition(const glm::vec2& position) noexcept;
		const glm::vec2& getMousePosition() const noexcept;

		Rml::Context& getContext() noexcept;
		const Rml::Context& getContext() const noexcept;

		RmluiCanvasImpl& getImpl() noexcept;
		const RmluiCanvasImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<RmluiCanvasImpl> _impl;
	};

	class RmluiComponentImpl;

	class DARMOK_EXPORT RmluiComponent final : public ICameraComponent
	{
	public:
		RmluiComponent() noexcept;
		~RmluiComponent() noexcept;

		void init(Camera& cam, Scene& scene, App& app) override;
		void update(float deltaTime) noexcept override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;
		void beforeRenderView(IRenderGraphContext& context) override;

		RmluiComponentImpl& getImpl() noexcept;
		const RmluiComponentImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<RmluiComponentImpl> _impl;
	};
}