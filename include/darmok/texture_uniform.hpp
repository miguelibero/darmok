
#pragma once

#include <darmok/asset.hpp>
#include <darmok/serialize.hpp>
#include <darmok/texture.hpp>

#include <string>
#include <unordered_map>

#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{	
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

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(name),
				CEREAL_NVP(stage)
			);
		}
	};

	class Texture;

	class TextureUniform final
	{
	public:
		using Key = TextureUniformKey;

		TextureUniform() noexcept;
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

		template<class Archive>
		void serialize(Archive& archive)
		{
			auto& assets = SerializeContextStack<AssetContext>::get();
			auto texDef = assets.getTextureLoader().getDefinition(_texture);
			archive(cereal::make_map_item(_key, texDef));
		}

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

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP_("uniforms", _uniforms)
			);
		}

	private:
		std::unordered_map<Key, TextureUniform, Key::Hash> _uniforms;
		bool _autoInit;
		bool _initialized;
	};	
}

