#include "program.hpp"
#include <unordered_map>
#include <optional>
#include <charconv>

#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/utils.hpp>

#include "embedded_shader.hpp"
#include "generated/shaders/gui.vertex.h"
#include "generated/shaders/gui.fragment.h"
#include "generated/shaders/gui_image.vertex.h"
#include "generated/shaders/gui_image.fragment.h"
#include "generated/shaders/gui.layout.h"
#include "generated/shaders/unlit.vertex.h"
#include "generated/shaders/unlit.fragment.h"
#include "generated/shaders/unlit.layout.h"
#include "generated/shaders/forward_phong.vertex.h"
#include "generated/shaders/forward_phong.fragment.h"
#include "generated/shaders/forward_phong.layout.h"
#include "generated/shaders/forward_pbr.vertex.h"
#include "generated/shaders/forward_pbr.fragment.h"
#include "generated/shaders/forward_pbr.layout.h"

namespace darmok
{
	Program::Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, std::string_view layoutJson)
		: _handle{ bgfx::kInvalidHandle }
	{
		auto renderer = bgfx::getRendererType();
		_handle = bgfx::createProgram(
			bgfx::createEmbeddedShader(embeddedShaders, renderer, (name + "_vertex").c_str()),
			bgfx::createEmbeddedShader(embeddedShaders, renderer, (name + "_fragment").c_str()),
			true
		);
		rapidjson::Document doc;
		doc.Parse(layoutJson.data(), layoutJson.size());
		Program::readVertexLayoutJson(doc, _layout);
	}


    Program::Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept
		: _handle(handle)
		, _layout(layout)
	{
	}

    Program::~Program() noexcept
    {
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
    }

	const bgfx::ProgramHandle& Program::getHandle() const noexcept
	{
		return _handle;
	}

	const bgfx::VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _layout;
	}

	static std::optional<int> getNameSuffixCounter(const std::string_view name, const std::string_view prefix) noexcept
	{
		if (!StringUtils::startsWith(name, prefix))
		{
			return std::nullopt;
		}
		int v;
		auto r = std::from_chars(name.data() + prefix.size(), name.data() + name.size(), v);
		if (r.ptr == nullptr)
		{
			return std::nullopt;
		}
		return v;
	}

	bgfx::Attrib::Enum Program::getBgfxAttrib(const std::string_view name) noexcept
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
		if (sname == "weight" || sname == "w")
		{
			return bgfx::Attrib::Weight;
		}
		auto count = getNameSuffixCounter(sname, "color");
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
		}
		count = getNameSuffixCounter(sname, "texcoord");
		if (count == std::nullopt)
		{
			count = getNameSuffixCounter(sname, "tex_coord");
		}
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
		}

		return bgfx::Attrib::Count;
	}

	bgfx::AttribType::Enum Program::getBgfxAttribType(const std::string_view name) noexcept
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

	static std::string_view getStringView(const rapidjson::Value& v) noexcept
	{
		return std::string_view(v.GetString(), v.GetStringLength());
	}

	void Program::readVertexLayoutJson(const rapidjson::Document& doc, bgfx::VertexLayout& layout) noexcept
	{
		layout.begin();
		for (auto& elm : doc.GetObject())
		{
			auto attrib = getBgfxAttrib(getStringView(elm.name));
			if (attrib == bgfx::Attrib::Count)
			{
				continue;
			}
			auto type = bgfx::AttribType::Float;
			if (elm.value.HasMember("type"))
			{
				type = getBgfxAttribType(getStringView(elm.value["type"]));
			}
			if (type == bgfx::AttribType::Count)
			{
				continue;
			}
			uint8_t num = 1;
			if (elm.value.HasMember("num"))
			{
				num = elm.value["num"].GetUint();
			}
			auto normalize = false;
			if (elm.value.HasMember("normalize"))
			{
				normalize = elm.value["normalize"].GetBool();
			}
			auto asInt = false;
			if (elm.value.HasMember("int"))
			{
				asInt = elm.value["int"].GetBool();
			}
			layout.add(attrib, num, type, normalize, asInt);
		}
		layout.end();
	}

	const DataProgramLoader::Suffixes DataProgramLoader::defaultSuffixes = Suffixes{ "_vertex", "_fragment", "_vertex_layout" };

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes) noexcept
		: _dataLoader(dataLoader)
		, _vertexLayoutLoader(vertexLayoutLoader)
		, _suffixes(suffixes)
	{
	}

	static std::string getShaderExt()
	{
		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
			return "";
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			return ".dx11";
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:
			return ".pssl";
		case bgfx::RendererType::Metal:
			return ".metal";
		case bgfx::RendererType::Nvn:
			return ".nvn";
		case bgfx::RendererType::OpenGL:
			return ".glsl";
		case bgfx::RendererType::OpenGLES:
			return ".essl";
		case bgfx::RendererType::Vulkan:
			return ".spv";
		}
		throw std::runtime_error("unknown renderer type");
	}

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& filePath)
	{
		std::string dataName = filePath + getShaderExt() + ".bin";
		auto data = _dataLoader(dataName);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data.makeRef());
		bgfx::setName(handle, filePath.c_str());
		return handle;
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
		std::string nameStr(name);
		const bgfx::ShaderHandle vsh = loadShader(nameStr + _suffixes.vertex);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!name.empty())
		{
			fsh = loadShader(nameStr + _suffixes.fragment);
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		auto layout = _vertexLayoutLoader(nameStr + _suffixes.vertexLayout);
		return std::make_shared<Program>(handle, layout);
	}

	StandardProgramLoader::StandardProgramLoader() noexcept
		: _impl(std::make_unique<StandardProgramLoaderImpl>())
	{
	}

	StandardProgramLoader::~StandardProgramLoader() noexcept
	{
	}

	StandardProgramLoader::result_type StandardProgramLoader::operator()(StandardProgramType type) noexcept
	{
		return (*_impl)(type);
	}

	const bgfx::EmbeddedShader StandardProgramLoaderImpl::_embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(gui_vertex),
		BGFX_EMBEDDED_SHADER(gui_fragment),
		BGFX_EMBEDDED_SHADER(unlit_vertex),
		BGFX_EMBEDDED_SHADER(unlit_fragment),
		BGFX_EMBEDDED_SHADER(forward_phong_vertex),
		BGFX_EMBEDDED_SHADER(forward_phong_fragment),
		BGFX_EMBEDDED_SHADER(forward_pbr_vertex),
		BGFX_EMBEDDED_SHADER(forward_pbr_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	const std::unordered_map<StandardProgramType, std::string> StandardProgramLoaderImpl::_embeddedShaderNames
	{
		{StandardProgramType::Gui, "gui"},
		{StandardProgramType::Unlit, "unlit"},
		{StandardProgramType::ForwardPhong, "forward_phong"},
		{StandardProgramType::ForwardPbr, "forward_pbr"},
	};

	const std::unordered_map<StandardProgramType, std::string_view> StandardProgramLoaderImpl::_embeddedShaderVertexLayouts
	{
		{StandardProgramType::Gui, gui_layout},
		{StandardProgramType::Unlit, unlit_layout},
		{StandardProgramType::ForwardPhong, forward_phong_layout},
		{StandardProgramType::ForwardPbr, forward_pbr_layout},
	};

	std::shared_ptr<Program> StandardProgramLoaderImpl::operator()(StandardProgramType type) const noexcept
	{
		auto itr = _embeddedShaderNames.find(type);
		if (itr == _embeddedShaderNames.end())
		{
			return nullptr;
		}
		auto itr2 = _embeddedShaderVertexLayouts.find(type);
		if (itr2 == _embeddedShaderVertexLayouts.end())
		{
			return nullptr;
		};
		return std::make_shared<Program>(itr->second, _embeddedShaders, itr2->second);
	}
}