#pragma once

#include <variant>

namespace darmok
{
    // replacement for C++20 std::expected


    template<typename E>
    struct Unexpected final
    {
        E value;
        Unexpected(E&& value)
            : value(std::forward<E>(value))
        {
        }
    };

    template<typename T, typename E>
    struct Expected final
    {
        using Unexpected = Unexpected<E>;

        Expected(const T& value)
            : _value(value)
        {
        }

        Expected(T&& value)
            : _value(std::forward<T>(value))
        {
        }

        Expected(Unexpected&& err)
            : _value(std::forward<E>(err.value))
        {
        }

        operator bool() const noexcept
        {
            return hasValue();
        }

        bool hasValue() const noexcept
        {
            return std::holds_alternative<T>(_value);
        }

        T& value() noexcept
        {
            return std::get<T>(_value);
        }

        const T& value() const noexcept
        {
            return std::get<T>(_value);
        }

        E& error() noexcept
        {
            return std::get<E>(_value);
        }

        const E& error() const noexcept
        {
            return std::get<E>(_value);
        }

    private:
        std::variant<T, E> _value;
    };
}

