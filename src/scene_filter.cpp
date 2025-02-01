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

    EntityFilter::EntityFilter(Container elements, Operation op) noexcept
        : elements(std::move(elements))
        , op(op)
    {
    }

    EntityFilter& EntityFilter::operator|=(const Element& elm) noexcept
    {
        if (const auto* filter = std::get_if<EntityFilter>(&elm))
        {
            if (filter->empty())
            {
                return *this;
            }
        }
        if (op != Operation::Or)
        {
            if (op != Operation::Not && elements.size() > 1)
            {
                elements = { EntityFilter(*this) };
            }
            op = Operation::Or;
        }
        if (auto typeId = getTypeId(elm))
        {
            elements.emplace_back(typeId.value());
        }
        else
        {
            elements.push_back(elm);
        }
        return *this;
    }

    EntityFilter EntityFilter::operator|(const Element& elm) const noexcept
    {
        EntityFilter filter(*this);
        filter |= elm;
        return filter;
    }

    EntityFilter& EntityFilter::operator&=(const Element& elm) noexcept
    {
        if (const auto* filter = std::get_if<EntityFilter>(&elm))
        {
            if (filter->empty())
            {
                return *this;
            }
        }
        if (op != Operation::And)
        {
            if (op != Operation::Not && elements.size() > 1)
            {
                elements = { EntityFilter(*this) };
            }
            op = Operation::And;
        }
        if (auto typeId = getTypeId(elm))
        {
            elements.emplace_back(typeId.value());
        }
        else
        {
            elements.push_back(elm);
        }
        return *this;
    }

    EntityFilter EntityFilter::operator&(const Element& elm) const noexcept
    {
        EntityFilter filter(*this);
        filter &= elm;
        return filter;
    }

    bool EntityFilter::empty() const noexcept
    {
        for (const auto& elm : elements)
        {
            if (const auto* filter = std::get_if<EntityFilter>(&elm))
            {
                if (!filter->empty())
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    std::optional<EntityFilter::TypeId> EntityFilter::getTypeId(const Element& elm) noexcept
    {
        if (const auto* type = std::get_if<entt::id_type>(&elm))
        {
            return *type;
        }
        auto filter = std::get<EntityFilter>(elm);
        if (filter.op == Operation::Not)
        {
            return std::nullopt;
        }
        if (filter.elements.size() != 1)
        {
            return std::nullopt;
        }
        return getTypeId(filter.elements[0]);
    }

    EntityFilter EntityFilter::negate(const Element& elm) noexcept
    {
        if (const auto* type = std::get_if<entt::id_type>(&elm))
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
        for (auto& elm : elements)
        {
            if (auto* type = std::get_if<entt::id_type>(&elm))
            {
                elm = EntityFilter({ *type }, Operation::Not);
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
        std::stringstream stream;
        size_t i = 0;
        auto size = elements.size();
        for (const auto& elm : elements)
        {
            stream << opFront;
            if (const auto* typeId = std::get_if<TypeId>(&elm))
            {
                stream << *typeId;
            }
            else
            {
                const auto& sub = std::get<EntityFilter>(elm);
                auto par = sub.elements.size() > 1;
                if (par)
                {
                    stream << "( ";
                }
                stream << sub.toString();
                if (par)
                {
                    stream << " )";
                }
            }
            ++i;
            if (i < size)
            {
                stream << opBack;
            }
        }
        return stream.str();
    }

    EntityViewFilter::EntityViewFilter(const EntityRegistry& registry, const EntityFilter& filter) noexcept
        : _op(filter.op)
        , _invalid(false)
    {
        for (const auto& elm : filter.elements)
        {
            if (const auto* typeId = std::get_if<TypeId>(&elm))
            {
                if (const auto* set = registry.storage(*typeId))
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
                const auto& sub = std::get<EntityFilter>(elm);
                _elements.emplace_back(EntityViewFilter(registry, sub));
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
            return std::ranges::all_of(_elements, [entity](auto& elm)
            {
                return valid(entity, elm);
            });
        }
        if (_op == Operation::Or)
        {
			return std::ranges::any_of(_elements, [entity](auto& elm)
			{
				return valid(entity, elm);
			});
        }
        if (_op == Operation::Not)
        {
            return std::ranges::any_of(_elements, [entity](auto& elm)
            {
                return !valid(entity, elm);
            });
        }
        return false;
    }

    bool EntityViewFilter::valid(Entity entity, const Element& elm) noexcept
    {
        if (const auto* setRef = std::get_if<SetRef>(&elm))
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
            if (auto typeId = EntityFilter::getTypeId(elm))
            {
                baseFound = true;
                if (const auto* set = registry.storage(*typeId))
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
            const auto* store = registry.storage<Entity>();
            _elements.emplace_back(*store);
        }
    }

    bool EntityView::contains(Entity entity) const noexcept
    {
        return _filter.valid(entity);
    }

    bool EntityView::empty() const noexcept
    {
        return std::ranges::all_of(_elements, [](const auto& elm) {
            return elm.get().empty();
        });
    }

    EntityView::Iterator EntityView::begin() const
    {
        auto adapter = EntityViewIteratorAdapter(*this);
        if (_elements.empty())
        {
            return { adapter, adapter.beginContItr };
        }
        return { adapter, adapter.beginContItr, adapter.beginElmItr };
    }

    EntityView::Iterator EntityView::end() const
    {
        auto adapter = EntityViewIteratorAdapter(*this);
        if (_elements.empty())
        {
            return { adapter, adapter.endContItr };
        }
        return { adapter, adapter.endContItr, adapter.endElmItr };
    }

    EntityView::ReverseIterator EntityView::rbegin() const
    {
        auto adapter = EntityViewReverseIteratorAdapter(*this);
        if (_elements.empty())
        {
            return { adapter, adapter.beginContItr };
        }
        return { adapter, adapter.beginContItr, adapter.beginElmItr };
    }

    EntityView::ReverseIterator EntityView::rend() const
    {
        auto adapter = EntityViewReverseIteratorAdapter(*this);
        if (_elements.empty())
        {
            return { adapter, adapter.endContItr };
        }
        return { adapter, adapter.endContItr, adapter.endElmItr };
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