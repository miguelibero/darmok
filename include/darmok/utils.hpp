#pragma once

#include <bx/error.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>

#include <type_traits>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <unordered_map>


namespace darmok
{
    template <typename E>
    constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);

    template<typename V>
    class BX_NO_VTABLE BaseContainer
    {
    public:
        template<typename C, typename T>
        struct BaseIterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = value_type*;
            using reference = value_type&;

            BaseIterator(C& container, size_t pos = 0)
                : _container(container)
                , _pos(pos)
            {
            }

            reference operator*() const
            {
                return *operator->();
            }

            pointer operator->() const
            {
                return &_container[_pos];
            }

            BaseIterator<C, T>& operator++()
            {
                _pos++;
                return *this;
            }

            BaseIterator<C, T> operator++(int)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const BaseIterator<C, T>& other) const
            {
                return &_container == &other._container && _pos == other._pos;
            }

            bool operator!=(const BaseIterator<C, T>& other) const
            {
                return !(operator==(other));
            }

        private:
            C& _container;
            size_t _pos;
        };

        typedef BaseIterator<BaseContainer<V>, V> Iterator;
        typedef BaseIterator<const BaseContainer<V>, const V> ConstIterator;

        Iterator begin()
        {
            return Iterator(*this, 0);
        }

        ConstIterator begin() const
        {
            return ConstIterator(*this, 0);
        }

        Iterator end()
        {
            return Iterator(*this, size());
        }

        ConstIterator end() const
        {
            return ConstIterator(*this, size());
        }

        bool empty() const
        {
            return size() == 0;
        }

        virtual size_t size() const = 0;
        virtual const V& operator[](size_t pos) const = 0;
        virtual V& operator[](size_t pos) = 0;
    };

    template<typename T>
    class BX_NO_VTABLE MemContainer : public BaseContainer<T>
    {
    public:
        const T& operator[](size_t pos) const override
        {
            return get(pos);
        }

        T& operator[](size_t pos) override
        {
            return get(pos);
        }

        virtual size_t size() const = 0;

    private:
        mutable std::unordered_map<size_t, T> _elements;

        T& get(size_t pos) const
        {
            if (pos < 0 || pos >= size())
            {
                throw std::out_of_range("out of container size");
            }
            auto itr = _elements.find(pos);
            if (itr != _elements.end())
            {
                return itr->second;
            }
            auto r = _elements.emplace(std::make_pair(pos, create(pos)));
            return r.first->second;
        }

        virtual T create(size_t pos) const = 0;
    };
}