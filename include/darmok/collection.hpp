#pragma once

#include <darmok/export.h>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <darmok/optional_ref.hpp>
#include <darmok/utils.hpp>


namespace darmok
{
    struct DARMOK_EXPORT CollectionUtils
    {
        template<typename T>
        static std::vector<std::unordered_set<T>> combinations(const std::unordered_set<T>& set) noexcept
        {
            std::vector<T> elms(set.begin(), set.end());
            std::vector<std::unordered_set<T>> combs;
            size_t n = elms.size();
            size_t size = std::pow(2, n);

            for (size_t i = 0; i < size; ++i)
            {
                auto& comb = combs.emplace_back();
                for (size_t j = 0; j < n; ++j)
                {
                    if (i & ((uint32_t)1 << j))
                    {
                        comb.insert(elms[j]);
                    }
                }
            }
            return combs;
        }
    };


    template<typename C, typename T>
    struct RefIterator final
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        RefIterator(C& collection, size_t pos = 0) noexcept
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

        RefIterator<C, T>& operator++() noexcept
        {
            _pos++;
            return *this;
        }

        RefIterator<C, T> operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const RefIterator<C, T>& other) const noexcept
        {
            return &_collection == &other._collection && _pos == other._pos;
        }

        bool operator!=(const RefIterator<C, T>& other) const noexcept
        {
            return !(operator==(other));
        }

    private:
        C& _collection;
        size_t _pos;
    };

    template<typename V>
    class BX_NO_VTABLE ConstRefCollection
    {
    public:
        using Iterator = RefIterator<const ConstRefCollection<V>, const V> ;

        virtual ~ConstRefCollection() = default;

        [[nodiscard]] Iterator begin() const noexcept
        {
            return Iterator(*this, 0);
        }

        [[nodiscard]] Iterator end() const
        {
            return Iterator(*this, size());
        }

        [[nodiscard]] bool empty() const
        {
            return size() == 0;
        }

        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual const V& operator[](size_t pos) const = 0;
    };

    template<typename V>
    class BX_NO_VTABLE RefCollection
    {
    public:
        using Iterator = RefIterator<RefCollection<V>, V>;
        using ConstIterator = RefIterator<const RefCollection<V>, const V>;

        virtual ~RefCollection() = default;

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

    template<typename C, typename T>
    struct ValIterator final
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;

        ValIterator(C& collection, size_t pos = 0) noexcept
            : _collection(collection)
            , _pos(pos)
        {
        }

        value_type operator*() const
        {
            return operator->();
        }

        value_type operator->() const
        {
            return _collection[_pos];
        }

        ValIterator<C, T>& operator++() noexcept
        {
            _pos++;
            return *this;
        }

        ValIterator<C, T> operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const ValIterator<C, T>& other) const noexcept
        {
            return &_collection == &other._collection && _pos == other._pos;
        }

        bool operator!=(const ValIterator<C, T>& other) const noexcept
        {
            return !(operator==(other));
        }

    private:
        C& _collection;
        size_t _pos;
    };

    template<typename V>
    class BX_NO_VTABLE ValCollection
    {
    public:
        using Iterator = ValIterator<const ValCollection<V>, V>;

        virtual ~ValCollection() = default;

        [[nodiscard]] Iterator begin() const noexcept
        {
            return Iterator(*this, 0);
        }

        [[nodiscard]] Iterator end() const
        {
            return Iterator(*this, size());
        }

        [[nodiscard]] bool empty() const
        {
            return size() == 0;
        }

        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual V operator[](size_t pos) const = 0;
    };
}

namespace std
{
    template<typename T>
    struct hash;
}

template<typename T> struct std::hash<std::unordered_set<T>>
{
    std::size_t operator()(const std::unordered_set<T>& key) const noexcept
    {
        size_t hash = 0;
        for (auto& elm : key)
        {
            darmok::hashCombine(hash, elm);
        }
        return hash;
    }
};