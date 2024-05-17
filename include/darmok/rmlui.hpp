#pragma once

#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Rml
{
	class Context;
}

namespace darmok
{
    class RmluiAppComponentImpl;
	class Texture;

    class RmluiAppComponent final : public AppComponent
    {
    public:
		DLLEXPORT RmluiAppComponent(const std::string& name) noexcept;
		DLLEXPORT RmluiAppComponent(const std::string& name, const glm::uvec2& size) noexcept;

		DLLEXPORT OptionalRef<Rml::Context> getContext() const noexcept;
		DLLEXPORT RmluiAppComponent& setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept;
		DLLEXPORT const std::vector<std::shared_ptr<Texture>>& getTargetTextures() noexcept;

		void init(App& app) override;
		void shutdown() noexcept override;
		bgfx::ViewId render(bgfx::ViewId viewId) const noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}