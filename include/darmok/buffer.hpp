#include <bgfx/bgfx.h>
#include <darmok/data.hpp>
#include <darmok/program_def.hpp>
#include <glm/glm.hpp>

namespace darmok
{
    class BufferDataWriter final
    {
    public:
        BufferDataWriter(const ProgramBufferDefinition& def, bx::AllocatorI* alloc = nullptr) noexcept;
        bool load(Data&& data) noexcept;

        template<typename T>
        BufferDataWriter& set(std::string_view name, uint32_t index, const T& input) noexcept
        {
            return set(name, index, &input, 1);
        }

        template<typename T>
        BufferDataWriter& set(std::string_view name, uint32_t index, const std::vector<T>& input) noexcept
        {
            return set(name, index, &input.front(), input.size());
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        BufferDataWriter& set(std::string_view name, uint32_t index, const const glm::vec<L, T, Q>& input) noexcept
        {
            return set(name, index, glm::value_ptr(input), L);
        }

        template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
        BufferDataWriter& set(std::string_view name, uint32_t index, const const glm::mat<L1, L2, T, Q>& input) noexcept
        {
            return set(name, index, glm::value_ptr(input), L1*L2);
        }

        template<typename T>
        BufferDataWriter& set(std::string_view name, uint32_t index, const T* ptr, size_t size) noexcept
        {
            
            return set(name, index, ptr, sizeof(T), size);
        }

        BufferDataWriter& set(std::string_view name, uint32_t index, const void* ptr, size_t elementSize, size_t num) noexcept;
        Data&& finish() noexcept;

    private:
        const ProgramBufferDefinition& _def;
        Data _data;
        bx::AllocatorI* _alloc;
    };
}