#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include <darmok/collection.hpp>
#include "generated/shaders/gui.program.h"
#include "generated/shaders/unlit.program.h"
#include "generated/shaders/forward.program.h"
#include "generated/shaders/forward_basic.program.h"
#include "generated/shaders/tonemap.program.h"

namespace darmok
{    
    Program::ShaderHandles Program::createShaders(const DefinesDataMap& defMap, const std::string& name)
    {
        ShaderHandles handles;
        for (auto& [defines, data] : defMap)
        {
            auto handle = bgfx::createShader(data.copyMem());
            auto defName = name + StringUtils::join(" ", defines.begin(), defines.end());
            if (!isValid(handle))
            {
                throw std::runtime_error("failed to create shader: " + defName);
            }
            bgfx::setName(handle, defName.c_str());
            handles[defines] = handle;
        }
        return handles;
    }

    bgfx::ShaderHandle Program::findBestShader(const Defines& defines, const ShaderHandles& handles) noexcept
    {
        size_t count = 0;
        bgfx::ShaderHandle handle = { bgfx::kInvalidHandle };
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
        : _vertexLayout(def.vertexLayout)
	{
        auto& profile = def.getCurrentProfile();
        _vertexHandles = createShaders(profile.vertexShaders, def.name + " vertex ");
        _fragmentHandles = createShaders(profile.fragmentShaders, def.name + " fragment ");

        for (auto& [defines, vertHandle] : _vertexHandles)
        {
            _allDefines.insert(defines.begin(), defines.end());
            auto fragHandle = findBestShader(defines, _fragmentHandles);
            createHandle(defines, vertHandle, fragHandle);
        }
        for (auto& [defines, fragHandle] : _fragmentHandles)
        {
            _allDefines.insert(defines.begin(), defines.end());
            auto vertHandle = findBestShader(defines, _vertexHandles);
            createHandle(defines, vertHandle, fragHandle);
        }
    }

    void Program::createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle)
    {
        if (_handles.contains(defines))
        {
            return;
        }
        auto handle = bgfx::createProgram(vertHandle, fragHandle);
        if (!isValid(handle))
        {
            auto definesStr = StringUtils::join(", ", defines.begin(), defines.end());
            throw std::runtime_error("failed to create program: " + definesStr);
        }
        _handles[defines] = handle;
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

	const VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _vertexLayout;
	}

    void StandardProgramLoader::loadDefinition(ProgramDefinition& def, StandardProgramType type)
    {
        switch (type)
        {
        case StandardProgramType::Gui:
            def.loadStaticMem(gui_program);
            break;
        case StandardProgramType::Unlit:
            def.loadStaticMem(unlit_program);
            break;
        case StandardProgramType::Forward:
            def.loadStaticMem(forward_program);
            break;
        case StandardProgramType::ForwardBasic:
            def.loadStaticMem(forward_basic_program);
            break;
        case StandardProgramType::Tonemap:
            def.loadStaticMem(tonemap_program);
            break;
        }
    }

    std::shared_ptr<ProgramDefinition> StandardProgramLoader::loadDefinition(StandardProgramType type)
    {
        auto itr = _defCache.find(type);
        if (itr != _defCache.end())
        {
            if (auto def = itr->second.lock())
            {
                return def;
            }
        }

        auto def = std::make_shared<ProgramDefinition>();
        loadDefinition(*def, type);
        _defCache[type] = def;
        return def;
    }

    std::shared_ptr<Program> StandardProgramLoader::load(StandardProgramType type)
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

    std::optional<StandardProgramType> StandardProgramLoader::getType(const std::shared_ptr<Program>& prog) noexcept
    {
        auto itr = std::find_if(_cache.begin(), _cache.end(),
            [prog](auto& elm) { return elm.second.lock() == prog; });
        if (itr != _cache.end())
        {
            return itr->first;
        }
        return std::nullopt;
    }

    std::optional<StandardProgramType> StandardProgramLoader::getType(const std::shared_ptr<ProgramDefinition>& def) noexcept
    {
        auto itr = std::find_if(_defCache.begin(), _defCache.end(),
            [def](auto& elm) { return elm.second.lock() == def; });
        if (itr != _defCache.end())
        {
            return itr->first;
        }
        return std::nullopt;
    }

    std::unordered_map<StandardProgramType, std::weak_ptr<ProgramDefinition>> StandardProgramLoader::_defCache;
    std::unordered_map<StandardProgramType, std::weak_ptr<Program>> StandardProgramLoader::_cache;
}