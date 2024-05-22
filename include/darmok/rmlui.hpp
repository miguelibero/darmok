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



	class BX_NO_VTABLE IRmluiMouseDelegate
	{
	public:
		virtual ~IRmluiMouseDelegate() = default;
		virtual glm::ivec2 onMousePositionChange(const glm::vec2& delta, const glm::vec2& position) = 0;
	};

    class RmluiAppComponent final : public AppComponent
    {
    public:
		DLLEXPORT RmluiAppComponent(const std::string& name) noexcept;
		DLLEXPORT RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept;

		DLLEXPORT OptionalRef<Rml::Context> getContext() const noexcept;
		
		DLLEXPORT RmluiAppComponent& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		DLLEXPORT const std::shared_ptr<Texture>& getTargetTexture() noexcept;

		DLLEXPORT RmluiAppComponent& setFixedSize(const std::optional<glm::uvec2>& size) noexcept;
		DLLEXPORT const std::optional<glm::uvec2>& getFixedSize() noexcept;

		DLLEXPORT glm::uvec2 getCurrentSize() const noexcept;

		DLLEXPORT RmluiAppComponent& setInputActive(bool active) noexcept;
		DLLEXPORT bool getInputActive() const noexcept;

		DLLEXPORT RmluiAppComponent& setMouseDelegate(IRmluiMouseDelegate& dlg) noexcept;
		DLLEXPORT RmluiAppComponent& resetMouseDelegate() noexcept;
		DLLEXPORT glm::ivec2 screenProject(const glm::vec3& position, const glm::mat4& model = glm::mat4(1)) noexcept;

		void init(App& app) override;
		void shutdown() noexcept override;
		bgfx::ViewId render(bgfx::ViewId viewId) const noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}