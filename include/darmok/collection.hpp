#pragma once

#include <iterator>
#include <stdexcept>
#include <unordered_map>

namespace darmok
{
    template<typename C, typename T>
    struct ReadOnlyIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        ReadOnlyIterator(C& collection, size_t pos = 0) noexcept
            : _collection(collection)
            , _pos(pos)
        {
        }

        reference operator*() const
        {
            return *operator->();
        }

        pointer operator->() const
        {
            return &_collection[_pos];
        }

        ReadOnlyIterator<C, T>& operator++() noexcept
        {
            _pos++;
            return *this;
        }

        ReadOnlyIterator<C, T> operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ReadOnlyIterator<C, T>& other) const noexcept
        {
            return &_collection == &other._collection && _pos == other._pos;
        }

        bool operator!=(const ReadOnlyIterator<C, T>& other) const noexcept
        {
            return !(operator==(other));
        }

    private:
        C& _collection;
        size_t _pos;
    };

    template<typename V>
    class ReadOnlyCollection
    {
    public:
        using Iterator = ReadOnlyIterator<ReadOnlyCollection<V>, V> ;
        using ConstIterator = ReadOnlyIterator<const ReadOnlyCollection<V>, const V>;

        [[nodiscard]] Iterator begin() noexcept
        {
            return Iterator(*this, 0);
        }

        [[nodiscard]] ConstIterator begin() const noexcept
        {
            return ConstIterator(*this, 0);
        }

        [[nodiscard]] Iterator end()
        {
            return Iterator(*this, size());
        }

        [[nodiscard]] ConstIterator end() const
        {
            return ConstIterator(*this, size());
        }

        [[nodiscard]] bool empty() const
        {
            return size() == 0;
        }

        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual const V& operator[](size_t pos) const = 0;
        [[nodiscard]] virtual V& operator[](size_t pos) = 0;
    };

    template<typename V>
    class ArrayCollection final : public ReadOnlyCollection<V>
    {
    public:
        ArrayCollection(V* ptr, size_t size)
            : _ptr(ptr)
        {
        }

        size_t size() const override
        {
            return _size;
        }

        const V& operator[](size_t pos) const override
        {
            return _ptr[pos];
        }

        V& operator[](size_t pos) override
        {
            return _ptr[pos];
        }

    private:
        V* _ptr;
        size_t _size;
    };

    template<typename T>
    class MemReadOnlyCollection : public ReadOnlyCollection<T>
    {
    public:
        [[nodiscard]] const T& operator[](size_t pos) const override
        {
            return get(pos);
        }

        [[nodiscard]] T& operator[](size_t pos) override
        {
            return get(pos);
        }

        [[nodiscard]] virtual size_t size() const = 0;

    private:
        mutable std::unordered_map<size_t, T> _elements;

        [[nodiscard]] T& get(size_t pos) const
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
            auto r = _elements.emplace(pos, create(pos));
            return r.first->second;
        }

        [[nodiscard]] virtual T create(size_t pos) const = 0;
    };
}