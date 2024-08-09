#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_graph.hpp>
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

	class RmluiViewImpl;

	class DARMOK_EXPORT RmluiView final
	{
	public:
		RmluiView(std::unique_ptr<RmluiViewImpl>&& impl) noexcept;
		~RmluiView() noexcept;

		std::string getName() const noexcept;

		bool getFullscreen() const noexcept;
		RmluiView& setFullscreen(bool enabled) noexcept;

		RmluiView& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		std::shared_ptr<Texture> getTargetTexture() noexcept;

		RmluiView& setViewport(const Viewport& vp) noexcept;
		const Viewport& getViewport() const noexcept;

		RmluiView& setEnabled(bool enabled) noexcept;
		bool getEnabled() const noexcept;

		RmluiView& setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;

		void setScrollBehavior(Rml::ScrollBehavior behaviour, float speedFactor) noexcept;

		RmluiView& setMousePosition(const glm::vec2& position) noexcept;
		const glm::vec2& getMousePosition() const noexcept;

		Rml::Context& getContext() noexcept;
		const Rml::Context& getContext() const noexcept;

		RmluiViewImpl& getImpl() noexcept;
		const RmluiViewImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<RmluiViewImpl> _impl;
	};

	class RmluiAppComponentImpl;

	class DARMOK_EXPORT RmluiAppComponent final : public IAppComponent
    {
    public:
		RmluiAppComponent() noexcept;
		~RmluiAppComponent() noexcept;

		RmluiAppComponentImpl& getImpl() noexcept;
		const RmluiAppComponentImpl& getImpl() const noexcept;

		OptionalRef<const RmluiView> getView(const std::string& name = "") const noexcept;
		RmluiView& getView(const std::string& name = "");
		RmluiView& addView(const std::string& name = "", int priority = 0);

		bool hasView(const std::string& name) const noexcept;
		bool removeView(const std::string& name);
		
		void init(App& app) override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;
		void update(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}