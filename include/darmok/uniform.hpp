#include <bgfx/bgfx.h>
#include <darmok/data.hpp>
#include <darmok/program_def.hpp>
#include <glm/glm.hpp>

namespace darmok
{
    class UniformDataWriter final
    {
    public:
        UniformDataWriter(const ProgramUniformDefinition& def, bx::AllocatorI* alloc = nullptr) noexcept;
        bool load(Data&& data) noexcept;

        template<typename T>
        UniformDataWriter& set(std::string_view name, const T& input) noexcept
        {
            return set(name, &input, 1);
        }

        template<typename T>
        UniformDataWriter& set(std::string_view name, const std::vector<T>& input) noexcept
        {
            return set(name, &input.front(), input.size());
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        UniformDataWriter& set(std::string_view name, const const glm::vec<L, T, Q>& input) noexcept
        {
            return set(name, glm::value_ptr(input), L);
        }

        template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
        UniformDataWriter& set(std::string_view name, const const glm::mat<L1, L2, T, Q>& input) noexcept
        {
            return set(name, glm::value_ptr(input), L1*L2);
        }

        template<typename T>
        UniformDataWriter& set(std::string_view name, const T* ptr, size_t size) noexcept
        {
            
            return set(name, ptr, sizeof(T), size);
        }

        UniformDataWriter& set(std::string_view name, const void* ptr, size_t elementSize, size_t num) noexcept;
        Data&& finish() noexcept;

    private:
        const ProgramUniformDefinition& _def;
        Data _data;
        bx::AllocatorI* _alloc;
    };
}