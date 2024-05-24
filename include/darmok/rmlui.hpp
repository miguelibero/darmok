#pragma once

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

	class BX_NO_VTABLE IRmluiMouseDelegate
	{
	public:
		virtual ~IRmluiMouseDelegate() = default;
		virtual glm::vec2 onMousePositionChange(const glm::vec2& delta, const glm::vec2& position) = 0;
	};

    class RmluiAppComponent final : public AppComponent
    {
    public:
		DLLEXPORT RmluiAppComponent(const std::string& name) noexcept;
		DLLEXPORT RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept;

		DLLEXPORT OptionalRef<Rml::Context> getContext() const noexcept;
		
		DLLEXPORT RmluiAppComponent& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		DLLEXPORT const std::shared_ptr<Texture>& getTargetTexture() noexcept;

		DLLEXPORT RmluiAppComponent& setViewport(const std::optional<Viewport>& viewport) noexcept;
		DLLEXPORT const std::optional<Viewport>& getViewport() const noexcept;
		DLLEXPORT Viewport getCurrentViewport() const noexcept;

		DLLEXPORT RmluiAppComponent& setInputActive(bool active) noexcept;
		DLLEXPORT bool getInputActive() const noexcept;

		DLLEXPORT RmluiAppComponent& setMousePosition(const glm::vec2& position) noexcept;

		DLLEXPORT RmluiAppComponent& setMouseDelegate(IRmluiMouseDelegate& dlg) noexcept;
		DLLEXPORT RmluiAppComponent& resetMouseDelegate() noexcept;


		void init(App& app) override;
		void shutdown() noexcept override;
		bgfx::ViewId render(bgfx::ViewId viewId) const noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}