#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include <darmok/collection.hpp>
#include <darmok/protobuf.hpp>
#include "generated/shaders/gui.program.h"
#include "generated/shaders/unlit.program.h"
#include "generated/shaders/forward.program.h"
#include "generated/shaders/forward_basic.program.h"
#include "generated/shaders/tonemap.program.h"

namespace darmok
{    
    expected<protobuf::Varying, std::string> Program::loadRefVarying(const Ref& ref, OptionalRef<IProgramSourceLoader> loader) noexcept
    {
        if (ref.has_standard())
        {
            auto def = StandardProgramLoader::loadDefinition(ref.standard());
            if (!def)
            {
                return unexpected{ "standard program error: " + std::move(def).error() };
            }
            return def.value()->varying();
        }
        if (ref.has_path())
        {
            if (!loader)
            {
                return unexpected{ "no program definition loader provided" };
            }
            auto result = (*loader)(ref.path());
            if (!result)
            {
                return unexpected{ "program " + ref.path() + ": " + result.error()};
            }
            auto src = result.value();
            if (!src)
            {
                return unexpected{ "empty program source" };
            }
            return src->varying();
        }
        return unexpected{ "empty program ref" };
    }

    ILoader<Program::Definition>::Result Program::loadRefDefinition(const Ref& ref, OptionalRef<IProgramDefinitionLoader> loader) noexcept
    {
        if (ref.has_standard())
        {
            return StandardProgramLoader::loadDefinition(ref.standard());
        }
        if (ref.has_path())
        {
            if (!loader)
            {
				return unexpected{ "no program definition loader provided" };
            }
            auto result = (*loader)(ref.path());
            if (!result)
            {
                return unexpected{ "program " + ref.path() + ": " + result.error() };
            }
            return result.value();
        }
        return nullptr;
    }

    ILoader<Program>::Result Program::loadRef(const Ref& ref, OptionalRef<IProgramLoader> loader) noexcept
    {
        if (ref.has_standard())
        {
            return StandardProgramLoader::load(ref.standard());
        }
        if (ref.has_path())
        {
            if (!loader)
            {
                return unexpected{ "no program loader provided" };
            }
            auto result = (*loader)(ref.path());
            if (!result)
            {
                return unexpected{ "program " + ref.path() + ": " + result.error() };
            }
            return result.value();
        }
        return nullptr;
    }

    expected<Program::ShaderHandles, std::string> Program::createShaders(const google::protobuf::RepeatedPtrField<protobuf::Shader>& shaders, const std::string& name) noexcept
    {		
        ShaderHandles handles;

        for (auto& shader : shaders)
        {
            Defines defines{ shader.defines().begin(), shader.defines().end() };
            auto shaderName = name + StringUtils::join(" ", defines.begin(), defines.end());
            
			if (shader.data().empty())
			{
				return unexpected{ "shader is empty: " + shaderName };
			}
            auto handle = bgfx::createShader(protobuf::copyMem(shader.data()));
            if (!isValid(handle))
            {
                return unexpected{ "failed to create shader: " + shaderName };
            }
            
            bgfx::setName(handle, shaderName.c_str());
            handles[defines] = handle;
        }
        
        return handles;
    }

    bgfx::ShaderHandle Program::findBestShader(const Defines& defines, const ShaderHandles& handles) noexcept
    {
        size_t count = 0;
        bgfx::ShaderHandle handle{ bgfx::kInvalidHandle };
        for (auto combDefines : CollectionUtils::combinations(defines))
        {
            auto itr = handles.find(combDefines);
            if (itr != handles.end() && count <= combDefines.size())
            {
                handle = itr->second;
                count = combDefines.size();
            }
        }
        return handle;
    }

    Program::Program(bgfx::VertexLayout layout, Handles handles) noexcept
        : _vertexLayout{ std::move(layout) }
        , _fragmentHandles{ std::move(handles).fragmentHandles }
        , _vertexHandles{ std::move(handles).vertexHandles }
        , _handles{ std::move(handles).programHandles }
    {
        auto addDefines = [this](const Defines& defines)
        {
            _allDefines.insert(defines.begin(), defines.end());
        };

        for (auto& elm : _fragmentHandles)
        {
            addDefines(elm.first);
        }
        for (auto& elm : _vertexHandles)
        {
            addDefines(elm.first);
        }
        for (auto& elm : _handles)
        {
            addDefines(elm.first);
        }
    }

	expected<Program, std::string> Program::load(const Definition& def) noexcept
	{
        auto profileResult = ConstProgramDefinitionWrapper{ def }.getCurrentProfile();
		if (!profileResult)
		{
            return unexpected{ std::move(profileResult).error() };
		}

        auto& profile = profileResult.value().get();

        Handles handles;
		
        auto fragResult = createShaders(profile.fragment_shaders(), def.name());
        if (!fragResult)
        {
            return unexpected{ std::move(fragResult).error()};
        }
        handles.fragmentHandles = std::move(fragResult.value());
        auto vertResult = createShaders(profile.vertex_shaders(), def.name());
        if (!vertResult)
        {
            return unexpected{ std::move(vertResult).error() };
        }
        handles.vertexHandles = std::move(vertResult.value());

        for (auto& [defines, vertHandle] : handles.vertexHandles)
        {
            auto fragHandle = findBestShader(defines, handles.fragmentHandles);
            auto result = createHandle(defines, vertHandle, fragHandle);
            if(!result)
            {
                return unexpected{ std::move(result).error() };
			}
            handles.programHandles[defines] = result.value();
        }
        for (auto& [defines, fragHandle] : handles.fragmentHandles)
        {
            auto vertHandle = findBestShader(defines, handles.vertexHandles);
            auto result = createHandle(defines, vertHandle, fragHandle);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            handles.programHandles[defines] = result.value();
        }
        auto vertexLayout = ConstVertexLayoutWrapper{ def.varying().vertex() }.getBgfx();
        return Program{ std::move(vertexLayout), std::move(handles) };
    }

    expected<bgfx::ProgramHandle, std::string> Program::createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle) noexcept
    {
        auto handle = bgfx::createProgram(vertHandle, fragHandle);
        if (!isValid(handle))
        {
            auto definesStr = StringUtils::join(", ", defines.begin(), defines.end());
            return unexpected{ "failed to create program: " + definesStr };
        }
        return handle;
    }

    Program::~Program() noexcept
    {
        for (auto& [defines, handle] : _handles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
        for (auto& [defines, handle] : _vertexHandles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
        for (auto& [defines, handle] : _fragmentHandles)
        {
            if (isValid(handle))
            {
                bgfx::destroy(handle);
            }
        }
    }

    Program::Source Program::createSource() noexcept
    {
        Program::Source src;
        return src;
    }

	bgfx::ProgramHandle Program::getHandle(const Defines& defines) const noexcept
	{
        Defines existingDefines;
        for (auto& define : defines)
        {
            if (_allDefines.contains(define))
            {
                existingDefines.insert(define);
            }
        }
        auto itr = _handles.find(existingDefines);
        if (itr != _handles.end())
        {
            return itr->second;
        }
        return { bgfx::kInvalidHandle };
	}

	const bgfx::VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _vertexLayout;
	}

    expected<void, std::string> StandardProgramLoader::loadDefinition(Definition& def, Type type)
    {
        switch (type)
        {
        case protobuf::StandardProgram::Gui:
            return protobuf::readStaticMem(def, gui_program);
        case protobuf::StandardProgram::Unlit:
            return protobuf::readStaticMem(def, unlit_program);
        case protobuf::StandardProgram::Forward:
            return protobuf::readStaticMem(def, forward_program);
        case protobuf::StandardProgram::ForwardBasic:
            return protobuf::readStaticMem(def, forward_basic_program);
        case protobuf::StandardProgram::Tonemap:
            return protobuf::readStaticMem(def, tonemap_program);
        default:
            break;
        }
        return unexpected{"undefined standar program type"};
    }

    expected<std::shared_ptr<Program::Definition>, std::string> StandardProgramLoader::loadDefinition(Type type) noexcept
    {
        auto itr = _defCache.find(type);
        if (itr != _defCache.end())
        {
            if (auto def = itr->second.lock())
            {
                return def;
            }
        }

        auto def = std::make_shared<Program::Definition>();
        auto result = loadDefinition(*def, type);
        if(!result)
        {
            return unexpected{ std::move(result).error() };
		}
        _defCache[type] = def;
        return def;
    }

    expected<std::shared_ptr<Program>, std::string> StandardProgramLoader::load(Type type) noexcept
    {
        auto itr = _cache.find(type);
        if (itr != _cache.end())
        {
            if (auto prog = itr->second.lock())
            {
                return prog;
            }
        }
        auto defResult = loadDefinition(type);
        if (!defResult)
        {
            return unexpected{ std::move(defResult).error() };
        }
        auto progResult = Program::load(*defResult.value());
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        auto prog = std::make_shared<Program>(std::move(progResult).value());
        _cache[type] = prog;
        return prog;
    }

    std::optional<StandardProgramLoader::Type> StandardProgramLoader::getType(const std::shared_ptr<Program>& prog) noexcept
    {
        auto itr = std::find_if(_cache.begin(), _cache.end(),
            [prog](auto& elm) { return elm.second.lock() == prog; });
        if (itr != _cache.end())
        {
            return itr->first;
        }
        return std::nullopt;
    }

    std::optional<StandardProgramLoader::Type> StandardProgramLoader::getType(const std::shared_ptr<Definition>& def) noexcept
    {
        auto itr = std::find_if(_defCache.begin(), _defCache.end(),
            [def](auto& elm) { return elm.second.lock() == def; });
        if (itr != _defCache.end())
        {
            return itr->first;
        }
        return std::nullopt;
    }

    std::unordered_map<StandardProgramLoader::Type, std::weak_ptr<Program::Definition>> StandardProgramLoader::_defCache;
    std::unordered_map<StandardProgramLoader::Type, std::weak_ptr<Program>> StandardProgramLoader::_cache;
}
