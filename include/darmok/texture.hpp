
#pragma once

#include <darmok/export.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/image.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <bx/bx.h>

#include <string>
#include <memory>

namespace darmok
{
	struct DARMOK_EXPORT TextureConfig final
	{
		glm::uvec2 size = glm::uvec2(1);
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
		TextureType type = TextureType::Texture2D;
		uint16_t depth = 0;
		bool mips = false;
		uint16_t layers = 1;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(size, format, type, depth, mips, layers);
		}

		[[nodiscard]] static const TextureConfig& getEmpty() noexcept;

		[[nodiscard]] std::string to_string() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getInfo() const noexcept;
	};

	class Data;
	class DataView;

	class DARMOK_EXPORT Texture final
	{
	public:
		using Config = TextureConfig;

		// TODO: remove bgfx flags param and move options to texture config struct

		Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		void update(const DataView& data, uint8_t mip = 0);
		void update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint8_t mip = 0, uint16_t layer = 0, uint8_t side = 0);
		void update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin = glm::uvec3(0), uint8_t mip = 0);
		uint32_t read(Data& data) noexcept;

		[[nodiscard]] std::string to_string() const noexcept;
		[[nodiscard]] const bgfx::TextureHandle& getHandle() const noexcept;
		[[nodiscard]] TextureType getType() const noexcept;
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] bgfx::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] uint16_t getDepth() const noexcept;
		[[nodiscard]] bool hasMips() const noexcept;
		[[nodiscard]] uint32_t getStorageSize() const noexcept;
		[[nodiscard]] uint8_t getMipsCount() const noexcept;
		[[nodiscard]] uint8_t getBitsPerPixel() const noexcept;

		Texture& setName(std::string_view name) noexcept;

	private:
		bgfx::TextureHandle _handle;
		Config _config;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureLoader
	{
	public:
		using result_type = std::shared_ptr<Texture>;
		virtual ~ITextureLoader() = default;
		[[nodiscard]] virtual result_type operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) = 0;
	};

	class IImageLoader;

	class DARMOK_EXPORT ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader) noexcept;
		[[nodiscard]] result_type operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) noexcept override;
	private:
		IImageLoader& _imgLoader;
	};

	struct TextureUniformKey final
	{
		std::string name;
		uint8_t stage;

		bool operator==(const TextureUniformKey& other) const noexcept;
		bool operator!=(const TextureUniformKey& other) const noexcept;

		size_t hash() const noexcept;

		struct Hash final
		{
			size_t operator()(const TextureUniformKey& key) const noexcept
			{
				return key.hash();
			}
		};
	};

	class TextureUniform final
	{
	public:
		using Key = TextureUniformKey;

		TextureUniform(const std::string& name, uint8_t stage, bool autoInit = true) noexcept;
		TextureUniform(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& tex, bool autoInit = true) noexcept;
		TextureUniform(const Key& key, bool autoInit = true) noexcept;
		TextureUniform(const Key& key, const std::shared_ptr<Texture>& tex, bool autoInit = true) noexcept;
		~TextureUniform() noexcept;
		TextureUniform(const TextureUniform& other) noexcept;
		TextureUniform& operator=(const TextureUniform& other) noexcept;
		TextureUniform(TextureUniform&& other) noexcept;
		TextureUniform& operator=(TextureUniform&& other) noexcept;

		void init() noexcept;
		void shutdown() noexcept;

		bool operator==(const TextureUniform& other) const noexcept;
		bool operator!=(const TextureUniform& other) const noexcept;

		TextureUniform& operator=(const std::shared_ptr<Texture>& tex) noexcept;
		operator const std::shared_ptr<Texture>& () const noexcept;

		TextureUniform& set(const std::shared_ptr<Texture>& texture) noexcept;
		const std::shared_ptr<Texture>& get() const noexcept;

		const TextureUniform& configure(bgfx::Encoder& encoder) const;
		TextureUniform& configure(bgfx::Encoder& encoder) noexcept;
	private:
		Key _key;
		std::shared_ptr<Texture> _texture;
		bgfx::UniformHandle _handle;

		void doConfigure(bgfx::Encoder& encoder) const noexcept;
	};

	class TextureUniformContainer final
	{
	public:
		using Key = TextureUniformKey;
		TextureUniformContainer(bool autoInit = true) noexcept;
		~TextureUniformContainer() noexcept;

		void init() noexcept;
		void shutdown() noexcept;

		TextureUniformContainer& set(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept;
		const TextureUniformContainer& configure(bgfx::Encoder& encoder) const;
		TextureUniformContainer& configure(bgfx::Encoder& encoder) noexcept;
	private:
		std::unordered_map<Key, TextureUniform, Key::Hash> _uniforms;
		bool _autoInit;
		bool _initialized;
	};
}

