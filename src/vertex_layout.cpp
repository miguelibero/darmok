#include <darmok/vertex_layout.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/utils.hpp>
#include <sstream>
#include <fstream>
#include <map>

namespace darmok
{

	bool VertexAttribute::inGroup(bgfx::Attrib::Enum attrib, ShaderAttributeGroup group) noexcept
	{
		return true;
	}

    bgfx::Attrib::Enum VertexAttribute::getBgfx(const std::string_view name) noexcept
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

	bgfx::AttribType::Enum VertexAttribute::getBgfxType(const std::string_view name) noexcept
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

	std::string VertexAttribute::getBgfxName(bgfx::Attrib::Enum val) noexcept
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

	std::string VertexAttribute::getBgfxTypeName(bgfx::AttribType::Enum val) noexcept
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


	void VertexAttribute::read(const std::string& key, const nlohmann::json& json)
	{
		attrib = getBgfx(key);
		if (attrib == bgfx::Attrib::Count)
		{
			throw std::invalid_argument("invalid key: " + key);
		}
		if (json.contains("type"))
		{
			auto typeStr = json["type"].get<std::string_view>();
			type = getBgfxType(typeStr);
			if (type == bgfx::AttribType::Count)
			{
				throw std::invalid_argument("invalid type: " + key);
			}
		}
		if (json.contains("num"))
		{
			num = json["num"].get<uint8_t>();
		}
		if (json.contains("normalize"))
		{
			normalize = json["normalize"].get<bool>();
		}
		if (json.contains("int"))
		{
			asInt = json["int"].get<bool>();
		}
	}

	std::string VertexAttribute::write(nlohmann::json& json) const noexcept
	{
		json["type"] = getBgfxTypeName(type);
		if (asInt)
		{
			json["int"] = true;
		}
		if (normalize)
		{
			json["normalize"] = true;
		}
		json["num"] = num;
		return getBgfxName(attrib);
	}

	void VertexAttribute::addTo(bgfx::VertexLayout& layout) const noexcept
	{
		layout.add(attrib, num, type, normalize, asInt);
	}

	bool VertexAttribute::inGroup(ShaderAttributeGroup group) const noexcept
	{
		return inGroup(attrib, group);
	}

	const std::unordered_map<std::string, bgfx::Attrib::Enum>& ProgramAttributes::getVaryingDefAttrs() noexcept
	{
		static const std::unordered_map<std::string, bgfx::Attrib::Enum> map =
		{
			{ "POSITION",		bgfx::Attrib::Position },
			{ "NORMAL",			bgfx::Attrib::Normal },
			{ "TANGENT",		bgfx::Attrib::Tangent },
			{ "BITANGENT",		bgfx::Attrib::Bitangent },
			{ "COLOR0",			bgfx::Attrib::Color0 },
			{ "COLOR1",			bgfx::Attrib::Color1 },
			{ "COLOR2",			bgfx::Attrib::Color2 },
			{ "COLOR3",			bgfx::Attrib::Color3 },
			{ "BLENDINDICES",	bgfx::Attrib::Indices },
			{ "BLENDWEIGHT",	bgfx::Attrib::Weight },
			{ "TEXCOORD0",		bgfx::Attrib::TexCoord0 },
			{ "TEXCOORD1",		bgfx::Attrib::TexCoord1 },
			{ "TEXCOORD2",		bgfx::Attrib::TexCoord2 },
			{ "TEXCOORD3",		bgfx::Attrib::TexCoord3 },
			{ "TEXCOORD4",		bgfx::Attrib::TexCoord4 },
			{ "TEXCOORD5",		bgfx::Attrib::TexCoord5 },
			{ "TEXCOORD6",		bgfx::Attrib::TexCoord6 },
			{ "TEXCOORD7",		bgfx::Attrib::TexCoord7 },
		};
		return map;
	}

	bgfx::VertexLayout ProgramAttributes::getVertexLayout(const std::vector<ShaderAttributeGroup>& disabledGroups)
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (auto& attr : vertex)
		{
			auto enabled = true;
			for (auto& group : disabledGroups)
			{
				if (attr.inGroup(group))
				{
					enabled = false;
				}
			}
			if (!enabled)
			{
				continue;
			}
			attr.addTo(layout);
		}
		layout.end();
		return layout;
	}

	void ProgramAttributes::read(const nlohmann::ordered_json& json) noexcept
	{

	}

	void ProgramAttributes::write(nlohmann::ordered_json& json) const noexcept
	{

	}

	void ProgramAttributes::readVaryingDef(std::istream& is)
	{
		const std::string vert_marker = "a_";
		const std::string frag_marker = "v_";
		const std::string instr_end = ";";
		const std::string comment = "//";
		std::string line;
		auto& varyingDefAttrs = getVaryingDefAttrs();
		while (std::getline(is, line))
		{
			StringUtils::trim(line);
			if (line.starts_with(comment))
			{
				continue;
			}
			if (line.ends_with(instr_end))
			{
				line = line.substr(0, line.size() - instr_end.size());
			}
			auto words = StringUtils::splitWords(line);
			if (words.size() < 4)
			{
				continue;
			}
			auto optNum = StringUtils::getIntSuffix(words[0], "vec");
			if (!optNum)
			{
				continue;
			}
			uint8_t num = optNum.value();
			auto itr = varyingDefAttrs.find(words[3]);
			if (itr == varyingDefAttrs.end())
			{
				continue;
			}
			auto attr = itr->second;
			auto& name = words[1];
			if (name.starts_with(vert_marker))
			{
				auto& elm = vertex.emplace_back(attr);
				elm.num = num;
				if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
				{
					elm.type = bgfx::AttribType::Uint8;
					elm.normalize = true;
				}
			}
			if (name.starts_with(frag_marker))
			{
				auto& elm = fragment.emplace_back(attr);
				elm.num = num;
				// TODO: read default value
			}
		}
	}

	void ProgramAttributes::writeVaryingDef(std::ostream& out)
	{

	}

	void VertexLayoutUtils::read(const std::filesystem::path& path, bgfx::VertexLayout& layout)
	{
		layout.begin().end();
		auto ext = path.extension();
		if (ext == ".json")
		{
			std::ifstream ifs(path);
			auto json = nlohmann::ordered_json::parse(ifs);
			read(json, layout);
			return;
		}
		auto fullExt = StringUtils::getFileExt(path.filename().string());
		if (ext == ".varyingdef" || fullExt == "varying.def.sc")
		{
			std::ifstream ifs(path);
			ProgramAttributes progAttrs;
			progAttrs.readVaryingDef(ifs);
			layout = progAttrs.getVertexLayout();
			return;
		}
		std::ifstream ifs(path, std::ios::binary);
		cereal::BinaryInputArchive archive(ifs);
		archive(layout);
	}

	void VertexLayoutUtils::write(const std::filesystem::path& path, const bgfx::VertexLayout& layout)
	{
		auto ext = path.extension();
		if (ext == ".json")
		{
			nlohmann::ordered_json json;
			VertexLayoutUtils::write(json, layout);
			std::ofstream os(path);
			os << json.dump(2) << std::endl;
		}
		else
		{
			std::ofstream os(path, std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(layout);
		}
	}

	void VertexLayoutUtils::read(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout)
	{
		layout.begin();
		for (auto& elm : json.items())
		{
			VertexAttribute attr;
			attr.read(elm.key(), elm.value());
			attr.addTo(layout);
		}
		layout.end();
	}

	void VertexLayoutUtils::write(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout)
	{
		std::map<uint16_t, VertexAttribute> items;
		for (auto i = 0; i < bgfx::Attrib::Count; i++)
		{
			const auto attr = static_cast<bgfx::Attrib::Enum>(i);
			if (!layout.has(attr))
			{
				continue;
			}
			auto offset = layout.getOffset(attr);
			VertexAttribute val;
			layout.decode(val.attrib, val.num, val.type, val.normalize, val.asInt);
			items.emplace(offset, val);
		}
		for (auto& [offset, val] : items)
		{
			nlohmann::json itemJson;
			auto attrName = val.write(itemJson);
			json.emplace(attrName, itemJson);
		}
	}
}

std::string to_string(const bgfx::VertexLayout& layout) noexcept
{
	nlohmann::ordered_json json;
	darmok::VertexLayoutUtils::write(json, layout);
	std::stringstream ss;
	ss << "VertexLayout:" << std::endl;
	ss << json.dump(2) << std::endl;
	return ss.str();
}

std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept
{
	return out << to_string(layout);
}