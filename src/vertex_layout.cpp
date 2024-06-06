#include <darmok/vertex_layout.hpp>
#include <darmok/utils.hpp>
#include <sstream>

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
			return bgfx::Attrib::Normal;
		}
		if (sname == "bitangent" || sname == "bitang" || sname == "b")
		{
			return bgfx::Attrib::Normal;
		}
		if (sname == "bitangent" || sname == "bitang" || sname == "b")
		{
			return bgfx::Attrib::Normal;
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

    void VertexLayoutUtils::readVaryingDef(std::string_view content, bgfx::VertexLayout& layout) noexcept
    {
        
    }
}

std::string to_string(const bgfx::VertexLayout& layout) noexcept
{
	std::stringstream ss;
	ss << "VertexLayout" << std::endl;
	// TODO
	return ss.str();
}

std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout)
{
	return out << to_string(layout);
}