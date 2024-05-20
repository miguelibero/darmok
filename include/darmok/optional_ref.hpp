#pragma once

#include <optional>
#include <functional>

namespace darmok
{
    /*
     * easier to use std::optional<std::reference_wrapper<T>>
     */
#ifndef DARMOK_INHERIT_OPTIONAL_REF

    template<typename T>
    class OptionalRef final
    {
    private:
        T* _value;
    public:
        using std_t = std::optional<std::reference_wrapper<T>>;

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

        OptionalRef(const std_t& value) noexcept
            : _value(value ? &value.value().get() : nullptr)
        {
        }

        OptionalRef<T>& operator=(std::nullopt_t) noexcept
        {
            reset();
            return *this;
        }

        void reset() noexcept
        {
            _value = nullptr;
        }

        [[nodiscard]] T* operator->() const
        {
            if (_value == nullptr)
            {
                throw std::bad_optional_access();
            }
            return _value;
        }

        [[nodiscard]] T& operator*() const
        {
            if (_value == nullptr)
            {
                throw std::bad_optional_access();
            }
            return *_value;
        }

        explicit operator bool() const noexcept
        {
            return _value != nullptr;
        }

        [[nodiscard]] T& value() const
        {
            if (!*this)
            {
                throw std::bad_optional_access();
            }
            return *_value;
        }

        [[nodiscard]] T* ptr() const noexcept
        {
            return _value;
        }

        operator std_t() const noexcept
        { 
            if (_value == nullptr)
            {
                return std::nullopt;
            }
            return *_value;
        }

        operator OptionalRef<const T>() const noexcept
        {
            return _value;
        }

        [[nodiscard]] bool operator==(const OptionalRef<T>& other) const noexcept
        {
            return _value == other._value;
        }

        [[nodiscard]] bool operator!=(const OptionalRef<T>& other) const noexcept
        {
            return !operator==(other);
        }
    };

#else

    template<typename T>
    class OptionalRef final : public std::optional<std::reference_wrapper<T>>
    {
    public:
        OptionalRef() noexcept
        {
        }

        OptionalRef(std::nullopt_t) noexcept
        {
        }

        OptionalRef(T* value) noexcept
            : std::optional<std::reference_wrapper<T>>(*value)
        {
            if (value == nullptr)
            {
                reset();
            }
        }

        OptionalRef(T& value) noexcept
            : std::optional<std::reference_wrapper<T>>(value)
        {
        }

        OptionalRef(std::reference_wrapper<T> value) noexcept
            : std::optional<std::reference_wrapper<T>>(value)
        {
        }

        OptionalRef(const OptionalRef<std::remove_const_t<T>>& other) noexcept
            : std::optional<std::reference_wrapper<T>>(other)
        {
        }

        [[nodiscard]] T* ptr() const noexcept
        {
            if (!*this)
            {
                return nullptr;
            }
            return &value();
        }

        [[nodiscard]] T* operator->() const
        {
            return &value();
        }

        [[nodiscard]] T& operator*() const
        {
            return value();
        }

        [[nodiscard]] T& value() const
        {
            return std::optional<std::reference_wrapper<T>>::value().get();
        }

        [[nodiscard]] bool operator==(const OptionalRef<T>& other) const noexcept
        {
            return ptr() == other.ptr();
        }

        [[nodiscard]] bool operator!=(const OptionalRef<T>& other) const noexcept
        {
            return !operator==(other);
        }
    };

#endif
}

template<typename T> struct std::hash<darmok::OptionalRef<T>>
{
    std::size_t operator()(const darmok::OptionalRef<T>& key) const noexcept
    {
        return (std::size_t)key.ptr();
    }
};