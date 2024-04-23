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

        OptionalRef(const std::optional<T>& value) noexcept
            : _value(value.has_value() ? &value.value() : nullptr)
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

        T* operator->() const
        {
            if (!hasValue())
            {
                throw std::bad_optional_access();
            }
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

        T* ptr() const noexcept
        {
            return _value;
        }

        operator OptionalRef<const T>() const noexcept
        { 
            return OptionalRef<const T>(_value);
        }

        [[nodiscard]] bool operator==(const OptionalRef<T>& other) const noexcept
        {
            if (_value == nullptr)
            {
                return other._value == nullptr;
            }
            return _value == other._value;
        }

        [[nodiscard]] bool operator!=(const OptionalRef<T>& other) const noexcept
        {
            return !operator==(other);
        }
    };
}

template<typename T> struct std::hash<darmok::OptionalRef<T>>
{
    std::size_t operator()(const darmok::OptionalRef<T>& key) const noexcept
    {
        return (std::size_t)key.ptr();
    }
};