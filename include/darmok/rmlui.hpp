#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <string>
#include <optional>

namespace Rml
{
	class Context;
}

namespace darmok
{
	class RmluiAppComponent;
	class RmluiAppComponentImpl;
	class Texture;
	class Camera;
	class Transform;
	struct Viewport;

	class DARMOK_EXPORT BX_NO_VTABLE IRmluiMouseDelegate
	{
	public:
		virtual ~IRmluiMouseDelegate() = default;
		virtual glm::vec2 onMousePositionChange(const glm::vec2& delta, const glm::vec2& position) = 0;
	};

	class DARMOK_EXPORT RmluiAppComponent final : public AppComponent
    {
    public:
		RmluiAppComponent(const std::string& name) noexcept;
		RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept;
		~RmluiAppComponent() noexcept;

		OptionalRef<Rml::Context> getContext() const noexcept;
		
		RmluiAppComponent& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		std::shared_ptr<Texture> getTargetTexture() noexcept;

		RmluiAppComponent& setViewport(const std::optional<Viewport>& viewport) noexcept;
		const std::optional<Viewport>& getViewport() const noexcept;
		Viewport getCurrentViewport() const noexcept;

		RmluiAppComponent& setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;

		RmluiAppComponent& setMousePosition(const glm::vec2& position) noexcept;

		RmluiAppComponent& setMouseDelegate(IRmluiMouseDelegate& dlg) noexcept;
		RmluiAppComponent& resetMouseDelegate() noexcept;


		void init(App& app) override;
		void shutdown() noexcept override;
		bgfx::ViewId render(bgfx::ViewId viewId) const noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}