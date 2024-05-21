#pragma once

#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <string>
#include <optional>

namespace Rml
{
	class Context;
}

namespace darmok
{
    class RmluiAppComponentImpl;
	class Texture;
	class Camera;
	class Transform;

    class RmluiAppComponent final : public AppComponent
    {
    public:
		DLLEXPORT RmluiAppComponent(const std::string& name) noexcept;
		DLLEXPORT RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept;

		DLLEXPORT OptionalRef<Rml::Context> getContext() const noexcept;
		DLLEXPORT RmluiAppComponent& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		DLLEXPORT const std::shared_ptr<Texture>& getTargetTexture() noexcept;
		DLLEXPORT RmluiAppComponent& setSize(const std::optional<glm::uvec2>& size) noexcept;
		DLLEXPORT const std::optional<glm::uvec2>& getSize() noexcept;

		RmluiAppComponent& setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;

		RmluiAppComponent& setMouseTransform(const Camera& cam, const Transform& trans) noexcept;
		RmluiAppComponent& resetMouseTransform() noexcept;

		void init(App& app) override;
		void shutdown() noexcept override;
		bgfx::ViewId render(bgfx::ViewId viewId) const noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}