#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include "generated/gui.program.h"
#include "generated/unlit.program.h"
#include "generated/forward.program.h"

namespace darmok
{    
    Program::ShaderHandles Program::createShaders(const ProgramProfileDefinition::Map& defMap, const std::string& name)
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

    Program::Program(StandardProgramType type)
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
        load(def);
    }

	Program::Program(const Definition& def)
	{
        load(def);
	}

    void Program::load(const Definition& def)
    {
        _vertexLayout = def.vertexLayout;
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
            }
        }
    }

    Program::Program(const Handles& handles, const VertexLayout& layout) noexcept
		: _handles(handles)
		, _vertexLayout(layout)
	{
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
        auto itr = _handles.find(defines);
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