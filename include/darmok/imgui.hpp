#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/texture.hpp>
#include <memory>
#include <bx/bx.h>

struct ImGuiContext;
typedef unsigned long long ImTextureID;

namespace darmok
{
	class DARMOK_EXPORT BX_NO_VTABLE IImguiRenderer
	{
	public:
		virtual ~IImguiRenderer() = default;

		virtual expected<void, std::string> imguiSetup() noexcept { return {}; };
		virtual expected<void, std::string> imguiRender() noexcept = 0;
	};

	class ImguiAppComponentImpl;

	struct ImguiTextureData final
	{
		bgfx::TextureHandle handle;
		bool alphaBlend;
		uint8_t mip;

		ImguiTextureData(const bgfx::TextureHandle& handle) noexcept;
		ImguiTextureData(const ImguiTextureData& other) = default;
		ImguiTextureData(const ImTextureID& id) noexcept;
		operator ImTextureID() const noexcept;
	};

	union ImguiTextureId { ImTextureID ptr; ImguiTextureData s; };

	class DARMOK_EXPORT ImguiAppComponent final : public ITypeAppComponent<ImguiAppComponent>
    {
    public:
		ImguiAppComponent(IImguiRenderer& renderer, float fontSize = 18.0f) noexcept;
		~ImguiAppComponent();

		expected<void, std::string> init(App& app) noexcept override;
		expected<void, std::string> shutdown() noexcept override;
		expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
		expected<void, std::string> render() noexcept override;
		expected<void, std::string> update(float dt) noexcept override;
		ImGuiContext* getContext() noexcept;
		bool getInputEnabled() const noexcept;
		ImguiAppComponent& setInputEnabled(bool enabled) noexcept;
		expected<void, std::string> updateFonts() noexcept;

	private:
		std::unique_ptr<ImguiAppComponentImpl> _impl;
    };
}