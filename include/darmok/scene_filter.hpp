#pragma once

#include <darmok/export.h>
#include <darmok/scene_fwd.hpp>

#include <unordered_set>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

namespace darmok
{
    enum class EntityFilterOperation
    {
        And,
        Or,
        Not
    };

    struct EntityFilter final
    {
        using TypeId = entt::id_type;
        using Operation = EntityFilterOperation;
        using Element = std::variant<TypeId, EntityFilter>;
        using Container = std::vector<Element>;

        EntityFilter(TypeId typeId) noexcept;
        EntityFilter(Container elements = {}, Operation op = Operation::And) noexcept;

        EntityFilter& operator|=(const Element& elm) noexcept;
        EntityFilter operator|(const Element& elm) const noexcept;

        EntityFilter& operator&=(const Element& elm) noexcept;
        EntityFilter operator&(const Element& elm) const noexcept;

        bool empty() const noexcept;
        std::string toString() const noexcept;
        EntityFilter& negate() noexcept;
        EntityFilter& negateElements() noexcept;
        static EntityFilter negate(const Element& elm) noexcept;
        static std::optional<TypeId> getTypeId(const Element& elm) noexcept;

        template<typename T>
        static EntityFilter create() noexcept
        {
            return EntityFilter(entt::type_hash<T>::value());
        }

        Container elements;
        Operation op;
    };

    class EntityViewFilter final
    {
    public:
        EntityViewFilter(const EntityRegistry& registry, const EntityFilter& filter) noexcept;
        bool valid(Entity entity) const noexcept;
    private:
        using TypeId = entt::id_type;
        using SetRef = std::reference_wrapper<const EntitySparseSet>;
        using Element = std::variant<SetRef, EntityViewFilter>;
        using Container = std::vector<Element>;
        using Operation = EntityFilterOperation;

        Container _elements;
        EntityFilterOperation _op;
        bool _invalid;

        static bool valid(Entity entity, const Element& elm) noexcept;
    };

    template<typename Adapter>
    struct BaseEntityViewIterator;

    struct EntityViewIteratorAdapter;
    struct EntityViewReverseIteratorAdapter;

    class EntityView final
    {
    public:
        using SetRef = std::reference_wrapper<const EntitySparseSet>;
        using Container = std::vector<SetRef>;
        using Iterator = BaseEntityViewIterator<EntityViewIteratorAdapter>;
        using ReverseIterator = BaseEntityViewIterator<EntityViewReverseIteratorAdapter>;

        EntityView(const EntityRegistry& registry, const EntityFilter& filter) noexcept;

        bool contains(Entity entity) const noexcept;
        bool empty() const noexcept;

        Iterator begin() const;
        Iterator end() const;
        ReverseIterator rbegin() const;
        ReverseIterator rend() const;
    private:
        Container _elements;
        EntityViewFilter _filter;

        friend struct EntityViewIteratorAdapter;
        friend struct EntityViewReverseIteratorAdapter;
    };

    struct EntityViewIteratorAdapter final
    {
        using ElementIterator = EntitySparseSet::const_iterator;
        using ContainerIterator = EntityView::Container::const_iterator;

        EntityViewIteratorAdapter(const EntityView& view) noexcept;

        ElementIterator beginElmItr;
        ElementIterator endElmItr;
        ContainerIterator beginContItr;
        ContainerIterator endContItr;
        std::reference_wrapper<const EntityView> view;

        ElementIterator containerBegin(const ContainerIterator& itr) const;
        ElementIterator containerEnd(const ContainerIterator& itr) const;

    };

    struct EntityViewReverseIteratorAdapter final
    {
        using ElementIterator = EntitySparseSet::const_reverse_iterator;
        using ContainerIterator = EntityView::Container::const_reverse_iterator;

        EntityViewReverseIteratorAdapter(const EntityView& view) noexcept;

        ElementIterator beginElmItr;
        ElementIterator endElmItr;
        ContainerIterator beginContItr;
        ContainerIterator endContItr;
        std::reference_wrapper<const EntityView> view;

        ElementIterator containerBegin(const ContainerIterator& itr) const;
        ElementIterator containerEnd(const ContainerIterator& itr) const;
    };

    template<typename Adapter>
    struct BaseEntityViewIterator final
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Adapter::ElementIterator::value_type;
        using pointer = Adapter::ElementIterator::pointer;
        using ElementIterator = Adapter::ElementIterator;
        using ContainerIterator = Adapter::ContainerIterator;

        BaseEntityViewIterator(const Adapter& adapter, ContainerIterator contItr, ElementIterator elmItr = {}) noexcept
            : _contItr(contItr)
            , _elmItr(elmItr)
            , _adapter(adapter)
        {
            tryNextContainer();
            if (_elmItr != adapter.endElmItr && !contains(*_elmItr))
            {
                operator++();
            }
        }

        value_type operator*() const
        {
            return _elmItr.operator*();
        }

        pointer operator->() const
        {
            return _elmItr.operator->();
        }

        BaseEntityViewIterator& operator++() noexcept
        {
            do
            {
                tryNextContainer();
                ++_elmItr;
                tryNextContainer();
            } while (_elmItr != _adapter.endElmItr && !contains(*_elmItr));
            return *this;
        }

        BaseEntityViewIterator operator++(int) noexcept
        {
            auto orig = *this;
            return ++(*this), orig;
        }

        BaseEntityViewIterator& operator--() noexcept
        {
            do
            {
                tryPrevContainer();
                --_elmItr;
                tryPrevContainer();
            } while (_elmItr != _adapter.beginElmItr && !contains(*_elmItr));
            return *this;
        }

        BaseEntityViewIterator operator--(int) noexcept
        {
            auto orig = *this;
            return operator--(), orig;
        }

        bool operator==(const BaseEntityViewIterator& other) const noexcept
        {
            return _contItr == other._contItr && _elmItr == other._elmItr;
        }

        bool operator!=(const BaseEntityViewIterator& other) const noexcept
        {
            return !operator==(other);
        }

    private:
        ContainerIterator _contItr;
        ElementIterator _elmItr;
        Adapter _adapter;

        bool contains(Entity entity) const noexcept
        {
            return _adapter.view.get().contains(entity);
        }

        bool tryNextContainer() noexcept
        {
            if (_contItr == _adapter.endContItr)
            {
                return false;
            }
            if (_elmItr != _adapter.containerEnd(_contItr))
            {
                return false;
            }
            do
            {
                ++_contItr;
            } while (_contItr != _adapter.endContItr && _contItr->get().empty());
            if (_contItr != _adapter.endContItr)
            {
                _elmItr = _adapter.containerBegin(_contItr);
                return true;
            }
            return false;
        }

        bool tryPrevContainer() noexcept
        {
            if (_contItr == _adapter.endContItr)
            {
                --_contItr;
            }
            if (_contItr == _adapter.beginContItr)
            {
                return false;
            }
            if (_elmItr != _adapter.containerBegin(_contItr))
            {
                return false;
            }
            do
            {
                --_contItr;
            } while (_contItr != _adapter.beginContItr && _contItr->get().empty());
            _elmItr = _adapter.containerEnd(_contItr);
            if (!_contItr->get().empty())
            {
                --_elmItr;
            }
            return true;
        }
    };
}