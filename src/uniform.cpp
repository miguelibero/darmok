#include <darmok/uniform.hpp>

namespace darmok
{
    UniformDataWriter::UniformDataWriter(const ProgramUniformDefinition& def, bx::AllocatorI* alloc) noexcept
        : _def(def)
        , _alloc(alloc)
    {
    }

    bool UniformDataWriter::load(Data&& data) noexcept
    {
        return true;
    }

    UniformDataWriter& UniformDataWriter::set(std::string_view name, const void* ptr, size_t elementSize, size_t num) noexcept
    {
        for (auto& pack : _def.packs)
        {

        }
        return *this;
    }

    Data&& UniformDataWriter::finish() noexcept
    {
        return std::move(_data);
    }
}