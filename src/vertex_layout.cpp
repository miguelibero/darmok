#include "vertex_layout.hpp"
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/utils.hpp>
#include <sstream>
#include <map>

namespace darmok
{
    bgfx::Attrib::Enum VertexLayoutUtils::getBgfxAttrib(const std::string_view name) noexcept
	{
		auto sname = StringUtils::toLower(name);
		if (sname == "position" || sname == "pos")
		{
			return bgfx::Attrib::Position;
		}
		if (sname == "normal" || sname == "norm" || sname == "n")
		{
			return bgfx::Attrib::Normal;
		}
		if (sname == "tangent" || name == "tang" || sname == "t")
		{
			return bgfx::Attrib::Tangent;
		}
		if (sname == "bitangent" || sname == "bitang" || sname == "b")
		{
			return bgfx::Attrib::Bitangent;
		}
		if (sname == "indices" || sname == "index" || sname == "i")
		{
			return bgfx::Attrib::Indices;
		}
		if (sname == "joints" || sname == "joint" || sname == "j")
		{
			return bgfx::Attrib::Indices;
		}
		if (sname == "weight" || sname == "weights" || sname == "w")
		{
			return bgfx::Attrib::Weight;
		}
		auto count = StringUtils::getIntSuffix(sname, "color");
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
		}
		count = StringUtils::getIntSuffix(sname, "texcoord");
		if (count == std::nullopt)
		{
			count = StringUtils::getIntSuffix(sname, "tex_coord");
		}
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
		}

		return bgfx::Attrib::Count;
	}

	bgfx::AttribType::Enum VertexLayoutUtils::getBgfxAttribType(const std::string_view name) noexcept
	{
		auto sname = StringUtils::toLower(name);
		if (sname == "u8" || sname == "uint8")
		{
			return bgfx::AttribType::Uint8;
		}
		if (sname == "u10" || sname == "uint10")
		{
			return bgfx::AttribType::Uint10;
		}
		if (sname == "i" || sname == "int" || sname == "int16")
		{
			return bgfx::AttribType::Int16;
		}
		if (sname == "h" || sname == "half" || sname == "float8")
		{
			return bgfx::AttribType::Half;
		}
		if (sname == "f" || sname == "float" || sname == "float16")
		{
			return bgfx::AttribType::Float;
		}
		return bgfx::AttribType::Count;
	}

	std::string VertexLayoutUtils::getBgfxAttribName(bgfx::Attrib::Enum val) noexcept
	{
		switch (val)
		{
		case bgfx::Attrib::Position:
			return "position";
		case bgfx::Attrib::Normal:
			return "normal";
		case bgfx::Attrib::Tangent:
			return "tangent";
		case bgfx::Attrib::Bitangent:
			return "bitangent";
		case bgfx::Attrib::Indices:
			return "indices";
		case bgfx::Attrib::Weight:
			return "weight";
		}
		if (val >= bgfx::Attrib::Color0 && val <= bgfx::Attrib::Color3)
		{
			auto i = to_underlying(val) - to_underlying(bgfx::Attrib::Color0);
			return std::string("color") + std::to_string(i);
		}
		if (val >= bgfx::Attrib::TexCoord0 && val <= bgfx::Attrib::TexCoord7)
		{
			auto i = to_underlying(val) - to_underlying(bgfx::Attrib::TexCoord0);
			return std::string("texcoord") + std::to_string(i);
		}
		return "";
	}

	std::string VertexLayoutUtils::getBgfxAttribTypeName(bgfx::AttribType::Enum val) noexcept
	{
		switch (val)
		{
			case bgfx::AttribType::Uint8:
				return "uint8";
			case bgfx::AttribType::Uint10:
				return "uint10";
			case bgfx::AttribType::Int16:
				return "int";
			case bgfx::AttribType::Half:
				return "half";
			case bgfx::AttribType::Float:
				return "float";
		}
		return "";
	}


	void VertexLayoutUtils::readJson(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout) noexcept
	{
		layout.begin();
		for (auto& elm : json.items())
		{
			auto attrib = getBgfxAttrib(elm.key());
			if (attrib == bgfx::Attrib::Count)
			{
				continue;
			}
			auto type = bgfx::AttribType::Float;
			auto val = elm.value();
			if (val.contains("type"))
			{
				type = getBgfxAttribType(val["type"].get<std::string_view>());
			}
			if (type == bgfx::AttribType::Count)
			{
				continue;
			}
			uint8_t num = 1;
			if (val.contains("num"))
			{
				num = val["num"].get<uint8_t>();
			}
			auto normalize = false;
			if (val.contains("normalize"))
			{
				normalize = val["normalize"].get<bool>();
			}
			auto asInt = false;
			if (val.contains("int"))
			{
				asInt = val["int"].get<bool>();
			}
			layout.add(attrib, num, type, normalize, asInt);
		}
		layout.end();
	}

	void VertexLayoutUtils::writeJson(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout) noexcept
	{
		std::map<uint16_t, std::pair<std::string, nlohmann::json>> items;
		for (auto i = 0; i < bgfx::Attrib::Count; i++)
		{
			const auto attr = static_cast<bgfx::Attrib::Enum>(i);
			if (!layout.has(attr))
			{
				continue;
			}
			uint8_t num;
			bgfx::AttribType::Enum type;
			bool normalize;
			bool asInt;
			auto offset = layout.getOffset(attr);
			layout.decode(attr, num, type, normalize, asInt);

			nlohmann::json itemJson;
			auto typeName = getBgfxAttribTypeName(type);
			auto attrName = getBgfxAttribName(attr);
			if (!typeName.empty())
			{
				itemJson["type"] = typeName;
			}
			if (asInt)
			{
				itemJson["int"] = true;
			}
			if (normalize)
			{
				itemJson["normalize"] = true;
			}
			itemJson["num"] = num;
			items[offset] = std::make_pair(attrName, itemJson);
		}
		for (auto& elm : items)
		{
			json.emplace(elm.second.first, elm.second.second);
		}
	}

    void VertexLayoutUtils::readVaryingDef(std::string_view content, bgfx::VertexLayout& layout) noexcept
    {
        // TODO: parse varyingdef to generate VertexLayout
    }

	BinaryVertexLayoutLoader::BinaryVertexLayoutLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    bgfx::VertexLayout BinaryVertexLayoutLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        bgfx::VertexLayout layout;
        // TODO: deserialize VertexLayout
        return layout;
    }
}

std::string to_string(const bgfx::VertexLayout& layout) noexcept
{
	std::stringstream ss;
	ss << "VertexLayout:" << std::endl;
	nlohmann::ordered_json json;
	darmok::VertexLayoutUtils::writeJson(json, layout);
	ss << json.dump(2) << std::endl;
	return ss.str();
}

std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout)
{
	return out << to_string(layout);
}