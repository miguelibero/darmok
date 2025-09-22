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
    expected<protobuf::Varying, std::string> Program::loadRefVarying(const Ref& ref, OptionalRef<IProgramSourceLoader> loader)
    {
        if (ref.has_standard())
        {
            auto def = StandardProgramLoader::loadDefinition(ref.standard());
            if (!def)
            {
                return unexpected{ "empty standard program" };
            }
            return def->varying();
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

    ILoader<Program::Definition>::Result Program::loadRefDefinition(const Ref& ref, OptionalRef<IProgramDefinitionLoader> loader)
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

    ILoader<Program>::Result Program::loadRef(const Ref& ref, OptionalRef<IProgramLoader> loader)
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

    expected<Program::ShaderHandles, std::string> Program::createShaders(const google::protobuf::RepeatedPtrField<protobuf::Shader>& shaders, const std::string& name)
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

	Program::Program(const Definition& def)
        : _vertexLayout{ ConstVertexLayoutWrapper{ def.varying().vertex() }.getBgfx() }
	{
        auto profileResult = ConstProgramDefinitionWrapper{ def }.getCurrentProfile();
		if (!profileResult)
		{
			throw std::runtime_error{ profileResult.error() };
		}

        auto& profile = profileResult.value().get();
		
        auto fragResult = createShaders(profile.fragment_shaders(), def.name());
        if (!fragResult)
        {
            throw std::runtime_error{ fragResult.error() };
        }
		_fragmentHandles = std::move(fragResult.value());
        auto vertResult = createShaders(profile.vertex_shaders(), def.name());
        if (!vertResult)
        {
            throw std::runtime_error{ vertResult.error() };
        }
        _vertexHandles = std::move(vertResult.value());

        for (auto& [defines, vertHandle] : _vertexHandles)
        {
            _allDefines.insert(defines.begin(), defines.end());
            auto fragHandle = findBestShader(defines, _fragmentHandles);
            auto result = createHandle(defines, vertHandle, fragHandle);
            if(!result)
            {
                throw std::runtime_error{ result.error() };
			}
        }
        for (auto& [defines, fragHandle] : _fragmentHandles)
        {
            _allDefines.insert(defines.begin(), defines.end());
            auto vertHandle = findBestShader(defines, _vertexHandles);
            auto result = createHandle(defines, vertHandle, fragHandle);
            if (!result)
            {
                throw std::runtime_error{ result.error() };
            }
        }
    }

    expected<bgfx::ProgramHandle, std::string> Program::createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle)
    {
        auto itr = _handles.find(defines);
        if (itr != _handles.end())
        {
            return itr->second;
        }
        auto handle = bgfx::createProgram(vertHandle, fragHandle);
        if (!isValid(handle))
        {
            auto definesStr = StringUtils::join(", ", defines.begin(), defines.end());
            return unexpected{ "failed to create program: " + definesStr };
        }
        _handles[defines] = handle;
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
        }
        return unexpected{"undefined standar program type"};
    }

    std::shared_ptr<Program::Definition> StandardProgramLoader::loadDefinition(Type type)
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
            return nullptr;
		}
        _defCache[type] = def;
        return def;
    }

    std::shared_ptr<Program> StandardProgramLoader::load(Type type)
    {
        auto itr = _cache.find(type);
        if (itr != _cache.end())
        {
            if (auto prog = itr->second.lock())
            {
                return prog;
            }
        }
        auto def = loadDefinition(type);
        if (!def)
        {
            return nullptr;
        }
        auto prog = std::make_shared<Program>(*def);
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