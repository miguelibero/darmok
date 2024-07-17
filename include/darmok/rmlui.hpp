#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/render_graph.hpp>
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

		RmluiView& setTargetTexture(const std::shared_ptr<Texture>& texture) noexcept;
		std::shared_ptr<Texture> getTargetTexture() noexcept;

		const Viewport& getViewport() const noexcept;
		void setViewport(const Viewport& vp) noexcept;

		RmluiView& setInputActive(bool active) noexcept;
		bool getInputActive() const noexcept;

		RmluiView& setMousePosition(const glm::vec2& position) noexcept;

		Rml::Context& getContext() noexcept;
		const Rml::Context& getContext() const noexcept;

		RmluiViewImpl& getImpl() noexcept;
		const RmluiViewImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<RmluiViewImpl> _impl;
	};

	class RmluiAppComponentImpl;

	class DARMOK_EXPORT RmluiAppComponent final : public AppComponent
    {
    public:
		RmluiAppComponent() noexcept;
		~RmluiAppComponent() noexcept;

		const RmluiView& getDefaultView() const noexcept;
		RmluiView& getDefaultView() noexcept;
		OptionalRef<const RmluiView> getView(const std::string& name) const noexcept;
		RmluiView& getView(const std::string& name);
		bool hasView(const std::string& name) const noexcept;
		bool removeView(const std::string& name);
		
		void init(App& app) override;
		void shutdown() noexcept override;
		void updateLogic(float dt) noexcept override;
	private:
		std::unique_ptr<RmluiAppComponentImpl> _impl;
    };
}