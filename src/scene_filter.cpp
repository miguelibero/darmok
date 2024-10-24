#include "scene_filter.hpp"
#include <darmok/scene_filter.hpp>
#include <sstream>

namespace darmok
{
    EntityFilter::EntityFilter(TypeId typeId) noexcept
        : elements{ typeId }
        , op(Operation::And)
    {
    }

    EntityFilter::EntityFilter(const Container& elements, Operation op) noexcept
        : elements(elements)
        , op(op)
    {
    }

    EntityFilter& EntityFilter::operator|=(const Element& elm) noexcept
    {
        if (op != Operation::Or)
        {
            elements = { EntityFilter(*this) };
            op = Operation::Or;
        }
        elements.push_back(elm);
        return *this;
    }

    EntityFilter EntityFilter::operator|(const Element& elm) const noexcept
    {
        EntityFilter r(*this);
        r |= elm;
        return r;
    }

    EntityFilter& EntityFilter::operator&=(const Element& elm) noexcept
    {
        if (op != Operation::And)
        {
            elements = { EntityFilter(*this) };
            op = Operation::And;
        }
        elements.push_back(elm);
        return *this;
    }

    EntityFilter EntityFilter::operator&(const Element& elm) const noexcept
    {
        EntityFilter r(*this);
        r &= elm;
        return r;
    }

    EntityFilter EntityFilter::negate(const Element& elm) noexcept
    {
        if (auto type = std::get_if<entt::id_type>(&elm))
        {
            return EntityFilter({ *type }, Operation::Not);
        }
        auto copy = std::get<EntityFilter>(elm);
        copy.negate();
        return copy;
    }

    EntityFilter& EntityFilter::negate() noexcept
    {
        if (op == Operation::Not)
        {
            op = Operation::And;
        }
        else
        {
            elements = { EntityFilter(*this) };
            op = Operation::Not;
        }
        return *this;
    }

    EntityFilter& EntityFilter::negateElements() noexcept
    {
        for (size_t i = 0; i < elements.size(); ++i)
        {
            auto& elm = elements[i];
            if (auto type = std::get_if<entt::id_type>(&elm))
            {
                elements[i] = EntityFilter({ *type }, Operation::Not);
            }
            else
            {
                std::get<EntityFilter>(elm).negate();
            }
        }
        return *this;
    }

    std::string EntityFilter::toString() const noexcept
    {
        std::string opFront;
        std::string opBack;
        switch (op)
        {
        case Operation::And:
            opBack = " && ";
            break;
        case Operation::Or:
            opBack = " || ";
            break;
        case Operation::Not:
            opFront = "!";
            opBack = " || ";
            break;
        }
        std::stringstream ss;
        size_t i = 0;
        auto size = elements.size();
        for (auto& elm : elements)
        {
            ss << opFront;
            if (auto typeId = std::get_if<TypeId>(&elm))
            {
                ss << *typeId;
            }
            else
            {
                auto& sub = std::get<EntityFilter>(elm);
                auto par = sub.elements.size() > 1;
                if (par)
                {
                    ss << "( ";
                }
                ss << sub.toString();
                if (par)
                {
                    ss << " )";
                }
            }
            ++i;
            if (i < size)
            {
                ss << opBack;
            }
        }
        return ss.str();
    }

    EntityViewFilter::EntityViewFilter(const EntityRegistry& registry, const EntityFilter& filter) noexcept
        : _op(filter.op)
        , _invalid(false)
    {
        for (auto& elm : filter.elements)
        {
            if (auto typeId = std::get_if<TypeId>(&elm))
            {
                if (auto set = registry.storage(*typeId))
                {
                    _elements.emplace_back(*set);
                }
                else if(_op == Operation::And)
                {
                    // and with nonexistent type will always fail
                    _elements.clear();
                    _invalid = true;
                    break;
                }
            }
            else
            {
                auto& sub = std::get<EntityFilter>(elm);
                _elements.push_back(EntityViewFilter(registry, sub));
            }
        }
    }

    bool EntityViewFilter::valid(Entity entity) const noexcept
    {
        if (_invalid)
        {
            return false;
        }
        if (_elements.empty())
        {
            return true;
        }
        if (_op == Operation::And)
        {
            for (auto& elm : _elements)
            {
                if (!valid(entity, elm))
                {
                    return false;
                }
            }
            return true;
        }
        if (_op == Operation::Or)
        {
            for (auto& elm : _elements)
            {
                if (valid(entity, elm))
                {
                    return true;
                }
            }
            return false;
        }
        if (_op == Operation::Not)
        {
            for (auto& elm : _elements)
            {
                if (!valid(entity, elm))
                {
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    bool EntityViewFilter::valid(Entity entity, const Element& elm) noexcept
    {
        if (auto setRef = std::get_if<SetRef>(&elm))
        {
            return setRef->get().contains(entity);
        }
        return std::get<EntityViewFilter>(elm).valid(entity);
    }

    EntityView::EntityView(const EntityRegistry& registry, const EntityFilter& filter) noexcept
        : _filter(registry, filter)
    {
        EntityFilter ffilter(filter);
        if (ffilter.op == EntityFilterOperation::Not)
        {
            ffilter.negate();
            ffilter.negateElements();
        }

        auto isAnd = ffilter.op == EntityFilterOperation::And;
        auto baseFound = false;
        for (auto& elm : ffilter.elements)
        {
            if (auto typeId = std::get_if<EntityFilter::TypeId>(&elm))
            {
                baseFound = true;
                if (auto set = registry.storage(*typeId))
                {
                    _elements.emplace_back(*set);
                }
                if (isAnd)
                {
                    break;
                }
            }
        }
        if (_elements.empty() && (!baseFound || !isAnd))
        {
            auto store = registry.storage<Entity>();
            _elements.emplace_back(*store);
        }
    }

    bool EntityView::contains(Entity entity) const noexcept
    {
        return _filter.valid(entity);
    }

    bool EntityView::empty() const noexcept
    {
        if (_elements.empty())
        {
            return true;
        }
        for (auto& elm : _elements)
        {
            if (!elm.get().empty())
            {
                return false;
            }
        }
        return true;
    }

    EntityView::Iterator EntityView::begin() const
    {
        auto adapter = EntityViewIteratorAdapter(*this);
        if (_elements.empty())
        {
            return Iterator(adapter, adapter.beginContItr);
        }
        return Iterator(adapter, adapter.beginContItr, adapter.beginElmItr);
    }

    EntityView::Iterator EntityView::end() const
    {
        auto adapter = EntityViewIteratorAdapter(*this);
        if (_elements.empty())
        {
            return Iterator(adapter, adapter.endContItr);
        }
        return Iterator(adapter, adapter.endContItr, adapter.endElmItr);
    }

    EntityView::ReverseIterator EntityView::rbegin() const
    {
        auto adapter = EntityViewReverseIteratorAdapter(*this);
        if (_elements.empty())
        {
            return ReverseIterator(adapter, adapter.beginContItr);
        }
        return ReverseIterator(adapter, adapter.beginContItr, adapter.beginElmItr);
    }

    EntityView::ReverseIterator EntityView::rend() const
    {
        auto adapter = EntityViewReverseIteratorAdapter(*this);
        if (_elements.empty())
        {
            return ReverseIterator(adapter, adapter.endContItr);
        }
        return ReverseIterator(adapter, adapter.endContItr, adapter.endElmItr);
    }

    EntityViewIteratorAdapter::EntityViewIteratorAdapter(const EntityView& view) noexcept
        : beginContItr(view._elements.begin())
        , endContItr(view._elements.end())
        , view(view)
    {
        if (!view._elements.empty())
        {
            beginElmItr = view._elements.front().get().begin();
            endElmItr = view._elements.back().get().end();
        }
    }

    EntityViewIteratorAdapter::ElementIterator EntityViewIteratorAdapter::containerBegin(const ContainerIterator& itr) const
    {
        return itr->get().begin();
    }

    EntityViewIteratorAdapter::ElementIterator EntityViewIteratorAdapter::containerEnd(const ContainerIterator& itr) const
    {
        return itr->get().end();
    }

    EntityViewReverseIteratorAdapter::EntityViewReverseIteratorAdapter(const EntityView& view) noexcept
        : beginContItr(view._elements.rbegin())
        , endContItr(view._elements.rend())
        , view(view)
    {
        if (!view._elements.empty())
        {
            beginElmItr = view._elements.back().get().rbegin();
            endElmItr = view._elements.front().get().rend();
        }
    }

    EntityViewReverseIteratorAdapter::ElementIterator EntityViewReverseIteratorAdapter::containerBegin(const ContainerIterator& itr) const
    {
        return itr->get().rbegin();
    }

    EntityViewReverseIteratorAdapter::ElementIterator EntityViewReverseIteratorAdapter::containerEnd(const ContainerIterator& itr) const
    {
        return itr->get().rend();
    }
}