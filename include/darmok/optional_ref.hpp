#pragma once

#include <optional>

namespace darmok
{
    template<typename T>
    class OptionalRef final
    {
    private:
        T* _value;
    public:
        OptionalRef() noexcept
        : _value(nullptr)
        {
        }

        OptionalRef(std::nullopt_t) noexcept
        : _value(nullptr)
        {
        }

        OptionalRef(T* value) noexcept
            : _value(value)
        {
        }

        OptionalRef(T& value) noexcept
            : _value(&value)
        {
        }

        OptionalRef(T&& value) noexcept
        : _value(&value)
        {
        }

        void reset() noexcept
        {
            _value = nullptr;
        }

        OptionalRef& operator=(std::nullopt_t) noexcept
        {
            reset();
            return *this;
        }

        OptionalRef& operator=(const OptionalRef& other) noexcept = default;

        bool operator==(T* other) noexcept
        {
            return _value == other;
        }

        bool operator!=(T* other) noexcept
        {
            return _value != other;
        }

        T* operator->() noexcept
        {
            return _value;
        }

        const T* operator->() const noexcept
        {
            return _value;
        }

        explicit operator bool() const noexcept
        {
            return hasValue();
        }

        bool hasValue() const noexcept
        {
            return _value != nullptr;
        }

        T& value() const
        {
            if (!hasValue())
            {
                throw std::bad_optional_access();
            }
            return *_value;
        }
    };
}