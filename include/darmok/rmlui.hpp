#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/rmlui_fwd.hpp>
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

	class RmluiCanvasImpl;

	class DARMOK_EXPORT RmluiCanvas final
	{
	public:
		using MousePositionMode = RmluiCanvasMousePositionMode;

		RmluiCanvas(const std::string& name, const std::optional<glm::uvec2>& size = std::nullopt) noexcept;
		~RmluiCanvas() noexcept;

		std::string getName() const noexcept;

		const std::optional<glm::uvec2>& getSize() const noexcept;
		RmluiCanvas& setSize(const std::optional<glm::uvec2>& vp) noexcept;
		glm::uvec2 getCurrentSize() const noexcept;

		const glm::vec3& getOffset() const noexcept;
		RmluiCanvas& setOffset(const glm::vec3& offset) noexcept;

		RmluiCanvas& setMousePositionMode(MousePositionMode mode) noexcept;
		MousePositionMode getMousePositionMode() const noexcept;

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

	class RmluiCameraComponentImpl;

	class DARMOK_EXPORT RmluiCameraComponent final : public ICameraComponent
	{
	public:
		RmluiCameraComponent() noexcept;
		~RmluiCameraComponent() noexcept;

		void init(Camera& cam, Scene& scene, App& app) override;
		void update(float deltaTime) noexcept override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;

		RmluiCameraComponentImpl& getImpl() noexcept;
		const RmluiCameraComponentImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<RmluiCameraComponentImpl> _impl;
	};
}