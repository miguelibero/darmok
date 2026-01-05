#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <darmok/texture.hpp>
#include <darmok/convert.hpp>
#include <memory>
#include <bx/bx.h>

struct ImGuiContext;
struct ImVec2;
struct ImVec4;
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

		ImguiTextureData(const TextureHandle& handle) noexcept;
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

	template<>
	struct Converter<ImVec2, glm::vec2>
	{
		static ImVec2 run(const glm::vec2& v) noexcept;
	};

	template<>
	struct Converter<ImVec2, glm::uvec2>
	{
		static ImVec2 run(const glm::uvec2& v) noexcept;
	};

	template<>
	struct Converter<glm::vec2, ImVec2>
	{
		static glm::vec2 run(const ImVec2& v) noexcept;
	};

	template<>
	struct Converter<glm::uvec2, ImVec2>
	{
		static glm::uvec2 run(const ImVec2& v) noexcept;
	};

	template<>
	struct Converter<ImVec4, glm::vec4>
	{
		static ImVec4 run(const glm::vec4& v) noexcept;
	};

	template<>
	struct Converter<ImVec4, glm::uvec4>
	{
		static ImVec4 run(const glm::uvec4& v) noexcept;
	};

	template<>
	struct Converter<glm::vec4, ImVec4>
	{
		static glm::vec4 run(const ImVec4& v) noexcept;
	};

	template<>
	struct Converter<glm::uvec4, ImVec4>
	{
		static glm::uvec4 run(const ImVec4& v) noexcept;
	};
}