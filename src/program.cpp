#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include "generated/gui.program.h"
#include "generated/unlit.program.h"
#include "generated/forward.program.h"

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
                throw std::runtime_error("failed to compile shader: " + defName);
            }
            bgfx::setName(handle, defName.c_str());
            handles[defines] = handle;
        }
        return handles;
    }

    std::optional<StandardProgramType> Program::getStandardType(std::string_view val) noexcept
    {
        auto lower = StringUtils::toLower(val);
        if (lower == "gui")
        {
            return StandardProgramType::Gui;
        }
        if (lower == "unlit")
        {
            return StandardProgramType::Unlit;
        }
        if (lower == "forward")
        {
            return StandardProgramType::Forward;
        }
        return std::nullopt;
    }

    Program::Definition Program::getStandardDefinition(StandardProgramType type) noexcept
    {
        Definition def;
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
        }
        return def;
    }

    Program::Program(StandardProgramType type)
        : Program(getStandardDefinition(type))
    {
    }

	Program::Program(const Definition& def)
        : _vertexLayout(def.vertexLayout)
	{
        auto& profile = def.getCurrentProfile();
        _vertexHandles = createShaders(profile.vertexShaders, def.name + " vertex ");
        _fragmentHandles = createShaders(profile.fragmentShaders, def.name + " fragment ");

        for (auto& [vertDefines, vertHandle] : _vertexHandles)
        {
            for (auto& [fragDefines, fragHandle] : _fragmentHandles)
            {
                Defines defines = vertDefines;
                defines.insert(fragDefines.begin(), fragDefines.end());
                _handles[defines] = bgfx::createProgram(vertHandle, fragHandle);
                _allDefines.insert(defines.begin(), defines.end());
            }
        }
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

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader(dataLoader)
	{
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
        ProgramDefinition def;
        def.load(_dataLoader(name));
        if (def.name.empty())
        {
            def.name = name;
        }
		return std::make_shared<Program>(def);
	}
}