#pragma once

#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    template<typename C, typename T>
    struct ReadOnlyIterator final
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
    class BX_NO_VTABLE ReadOnlyCollection
    {
    public:
        using Iterator = ReadOnlyIterator<ReadOnlyCollection<V>, V> ;
        using ConstIterator = ReadOnlyIterator<const ReadOnlyCollection<V>, const V>;

        virtual ~ReadOnlyCollection() = default;

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
    class FixedReadOnlyCollection : public ReadOnlyCollection<V>
    {
    public:
        FixedReadOnlyCollection(const V* ptr, size_t size)
            : _ptr(ptr)
        {
        }

        size_t size() const noexcept override
        {
            return _size;
        }

        const V& operator[](size_t pos) const override
        {
            return get(pos);
        }

        V& operator[](size_t pos) override
        {
            return get(pos);
        }

    private:
        const V* _ptr;
        size_t _size;

        [[nodiscard]] V& get(size_t pos) const
        {
            if (pos < 0 || pos >= size())
            {
                throw std::out_of_range("out of container size");
            }
            return _ptr[pos];
        }
    };

    template<typename V>
    class AllocatedReadOnlyCollection : public ReadOnlyCollection<V>
    {
    public:
        AllocatedReadOnlyCollection(size_t size, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept
            : _ptr(nullptr)
            , _size(size)
            , _alloc(alloc)
        {
        }

        AllocatedReadOnlyCollection(OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept
            : AllocatedReadOnlyCollection(0, alloc)
        {
        }

        size_t size() const noexcept override
        {
            return _size;
        }

        const V& operator[](size_t pos) const override
        {
            return get(pos);
        }

        V& operator[](size_t pos) override
        {
            return get(pos);
        }

    private:
        V* _ptr;
        size_t _size;
        OptionalRef<bx::AllocatorI> _alloc;

        V& get(size_t pos) const
        {
            if (pos < 0 || pos >= size())
            {
                throw std::out_of_range("out of container size");
            }
            return _ptr[pos];
        }

    protected:
        void allocCollection(size_t size, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept
        {
            _alloc = alloc;
            _size = size;
            size *= sizeof(V);
            void* ptr;
            if (_alloc)
            {
                ptr = bx::alloc(_alloc.ptr(), size);
            }
            else
            {
                ptr = malloc(size);
            }
            _ptr = static_cast<V*>(ptr);
        }

        void freeCollection() noexcept
        {
            if (_alloc)
            {
                bx::free(_alloc, _ptr);
            }
            else
            {
                std::free(_ptr);
            }
        }
    };

    template<typename V>
    class VectorReadOnlyCollection : public ReadOnlyCollection<V>
    {
    public:
        VectorReadOnlyCollection() = default;

        VectorReadOnlyCollection(size_t size) noexcept
            : _elements(size)
        {
        }

        VectorReadOnlyCollection(const std::vector<V>& elements) noexcept
            : _elements(elements)
        {
        }

        size_t size() const noexcept override
        {
            return _elements.size();
        }

        const V& operator[](size_t pos) const override
        {
            return _elements[pos];
        }

        V& operator[](size_t pos) override
        {
            return _elements[pos];
        }
    protected:
        std::vector<V> _elements;
    };

    template<typename V>
    class DynamicReadOnlyCollection : public ReadOnlyCollection<V>
    {
    public:
        [[nodiscard]] const V& operator[](size_t pos) const override
        {
            return get(pos);
        }

        [[nodiscard]] V& operator[](size_t pos) override
        {
            return get(pos);
        }

        [[nodiscard]] virtual size_t size() const = 0;

    private:
        mutable std::unordered_map<size_t, V> _elements;

        [[nodiscard]] V& get(size_t pos) const
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
            auto r = _elements.insert(pos, std::move(create(pos)));
            return r.first->second;
        }

        [[nodiscard]] virtual V&& create(size_t pos) const = 0;
    };
}