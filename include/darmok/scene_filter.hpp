#pragma once
#include <darmok/scene_fwd.hpp>
#include <unordered_set>
#include <vector>
#include <memory>
#include <variant>

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
        EntityFilter(const Container& elements = {}, Operation op = Operation::And) noexcept;

        EntityFilter& operator|=(const Element& elm) noexcept;
        EntityFilter operator|(const Element& elm) const noexcept;

        EntityFilter& operator&=(const Element& elm) noexcept;
        EntityFilter operator&(const Element& elm) const noexcept;

        std::string toString() const noexcept;
        EntityFilter& negate() noexcept;
        EntityFilter& negateElements() noexcept;
        static EntityFilter negate(const Element& elm) noexcept;

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

    struct EntityViewIterator;
    struct EntityViewReverseIterator;

    class EntityView final
    {
    public:
        using SetRef = std::reference_wrapper<const EntitySparseSet>;
        using Container = std::vector<SetRef>;
        using Iterator = EntityViewIterator;
        using ReverseIterator = EntityViewReverseIterator;

        EntityView(const EntityRegistry& registry, const EntityFilter& filter) noexcept;

        bool contains(Entity entity) const noexcept;

        Iterator begin() const;
        Iterator end() const;
        ReverseIterator rbegin() const;
        ReverseIterator rend() const;
    private:
        Container _elements;
        EntityViewFilter _filter;

        EntitySparseSet::const_iterator getElementBegin() const;
        EntitySparseSet::const_iterator getElementEnd() const;
        EntitySparseSet::const_reverse_iterator getElementReverseBegin() const;
        EntitySparseSet::const_reverse_iterator getElementReverseEnd() const;

        friend struct EntityViewIterator;
        friend struct EntityViewReverseIterator;
    };

    struct EntityViewIterator final
    {
        using ContainerIterator = EntityView::Container::const_iterator;
        using ElementIterator = EntitySparseSet::const_iterator;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ElementIterator::value_type;
        using pointer = ElementIterator::pointer;

        EntityViewIterator(const EntityView& view, ContainerIterator contItr, ElementIterator elmItr = {}) noexcept;

        value_type operator*() const;
        pointer operator->() const;
        EntityViewIterator& operator++() noexcept;
        EntityViewIterator operator++(int) noexcept; 
        EntityViewIterator& operator--() noexcept;
        EntityViewIterator operator--(int) noexcept;

        bool operator==(const EntityViewIterator& other) const noexcept;
        bool operator!=(const EntityViewIterator& other) const noexcept;

    private:
        ContainerIterator _contItr;
        ElementIterator _elmItr;
        ElementIterator _beginElmItr;
        ElementIterator _endElmItr;
        std::reference_wrapper<const EntityView> _view;

        bool tryNextContainer() noexcept;
        bool tryPrevContainer() noexcept;
    };

    struct EntityViewReverseIterator final
    {
        using ContainerIterator = EntityView::Container::const_reverse_iterator;
        using ElementIterator = EntitySparseSet::const_reverse_iterator;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ElementIterator::value_type;
        using pointer = ElementIterator::pointer;

        EntityViewReverseIterator(const EntityView& view, ContainerIterator contItr, ElementIterator elmItr = {}) noexcept;

        value_type operator*() const;
        pointer operator->() const;
        EntityViewReverseIterator& operator++() noexcept;
        EntityViewReverseIterator operator++(int) noexcept;
        EntityViewReverseIterator& operator--() noexcept;
        EntityViewReverseIterator operator--(int) noexcept;

        bool operator==(const EntityViewReverseIterator& other) const noexcept;
        bool operator!=(const EntityViewReverseIterator& other) const noexcept;

    private:
        ContainerIterator _contItr;
        ElementIterator _elmItr;
        ElementIterator _beginElmItr;
        ElementIterator _endElmItr;
        std::reference_wrapper<const EntityView> _view;

        bool tryNextContainer() noexcept;
        bool tryPrevContainer() noexcept;
    };

}