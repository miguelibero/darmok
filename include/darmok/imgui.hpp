#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <memory>
#include <bx/bx.h>

struct ImGuiContext;

namespace darmok
{
	class DARMOK_EXPORT BX_NO_VTABLE IImguiRenderer
	{
	public:
		virtual ~IImguiRenderer() = default;

		virtual void imguiRender() = 0;
	};

	class ImguiAppComponentImpl;

	class DARMOK_EXPORT ImguiAppComponent final : public IAppComponent
    {
    public:
		ImguiAppComponent(IImguiRenderer& renderer, float fontSize = 18.0f) noexcept;
		~ImguiAppComponent();

		void init(App& app) override;
		void shutdown() noexcept override;
		void renderReset() noexcept override;
		void update(float dt) noexcept override;
		ImGuiContext* getContext() noexcept;
		bool getInputEnabled() const noexcept;
		ImguiAppComponent& setInputEnabled(bool enabled) noexcept;
	private:
		std::unique_ptr<ImguiAppComponentImpl> _impl;
    };
}