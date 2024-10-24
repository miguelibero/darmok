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
        // TODO: only add storages in the filter
        auto store = registry.storage<Entity>();
        _elements.emplace_back(*store);

        /*
        EntityFilter ffilter(filter);
        if (ffilter.op == EntityFilterOperation::Not)
        {
            ffilter.negate();
            ffilter.negateElements();
        }

        if (ffilter.op == EntityFilterOperation::Or)
        {
            for (auto& elm : ffilter.elements)
            {
                if (auto typeId = std::get_if<EntityFilter::TypeId>(&elm))
                {
                    if (auto set = registry.storage(*typeId))
                    {
                        _elements.emplace_back(set);
                    }
                }
            }
        }
        */
    }

    bool EntityView::contains(Entity entity) const noexcept
    {
        return _filter.valid(entity);
    }

    EntitySparseSet::const_iterator EntityView::getElementBegin() const
    {
        return _elements.front().get().begin();
    }

    EntitySparseSet::const_iterator EntityView::getElementEnd() const
    {
        return _elements.back().get().end();
    }

    EntityView::Iterator EntityView::begin() const
    {
        auto cbegin = _elements.begin();
        if (_elements.empty())
        {
            return Iterator(*this, cbegin);
        }
        return Iterator(*this, cbegin, getElementBegin());
    }

    EntityView::Iterator EntityView::end() const
    {
        auto cend = _elements.end();
        if (_elements.empty())
        {
            return Iterator(*this, cend);
        }
        return Iterator(*this, cend, getElementEnd());
    }

    EntitySparseSet::const_reverse_iterator EntityView::getElementReverseBegin() const
    {
        return _elements.back().get().rbegin();
    }

    EntitySparseSet::const_reverse_iterator EntityView::getElementReverseEnd() const
    {
        return _elements.front().get().rend();
    }

    EntityView::ReverseIterator EntityView::rbegin() const
    {
        auto cbegin = _elements.rbegin();
        if (_elements.empty())
        {
            return ReverseIterator(*this, cbegin);
        }
        return ReverseIterator(*this, cbegin, getElementReverseBegin());
    }

    EntityView::ReverseIterator EntityView::rend() const
    {
        auto cend = _elements.rend();
        if (_elements.empty())
        {
            return ReverseIterator(*this, cend);
        }
        return ReverseIterator(*this, cend, getElementReverseEnd());
    }

    EntityViewIterator::EntityViewIterator(const EntityView& view, ContainerIterator contItr, ElementIterator elmItr) noexcept
        : _contItr(contItr)
        , _elmItr(elmItr)
        , _beginElmItr(view.getElementBegin())
        , _endElmItr(view.getElementEnd())
        , _view(view)
    {
        if (_elmItr != _endElmItr && !view.contains(*_elmItr))
        {
            operator++();
        }
    }

    EntityViewIterator::value_type EntityViewIterator::operator*() const
    {
        return _elmItr.operator*();
    }

    EntityViewIterator::pointer EntityViewIterator::operator->() const
    {
        return _elmItr.operator->();
    }

    EntityViewIterator& EntityViewIterator::operator++() noexcept
    {
        do
        {
            if (!tryNextContainer())
            {
                ++_elmItr;
                tryNextContainer();
            }
        }
        while (_elmItr != _endElmItr && !_view.get().contains(*_elmItr));

        return *this;
    }

    EntityViewIterator EntityViewIterator::operator++(int) noexcept
    {
        EntityViewIterator orig = *this;
        return ++(*this), orig;
    }

    EntityViewIterator& EntityViewIterator::operator--() noexcept
    {
        do
        {
            if (!tryPrevContainer())
            {
                --_elmItr;
                tryPrevContainer();
            }
        } while (_elmItr != _beginElmItr && !_view.get().contains(*_elmItr));
        return *this;
    }

    EntityViewIterator EntityViewIterator::operator--(int) noexcept
    {
        EntityViewIterator orig = *this;
        return operator--(), orig;
    }

    bool EntityViewIterator::tryNextContainer() noexcept
    {
        if (_elmItr == _contItr->get().end())
        {
            _contItr++;
            if (_elmItr != _endElmItr)
            {
                _elmItr = _contItr->get().begin();
            }
            return true;
        }
        return false;
    }

    bool EntityViewIterator::tryPrevContainer() noexcept
    {
        if (_elmItr == _contItr->get().begin())
        {
            _contItr--;
            if (_elmItr != _beginElmItr)
            {
                _elmItr = --_contItr->get().end();
            }
            return true;
        }
        return false;
    }

    bool EntityViewIterator::operator==(const EntityViewIterator& other) const noexcept
    {
        return _contItr == other._contItr && _elmItr == other._elmItr;
    }

    bool EntityViewIterator::operator!=(const EntityViewIterator& other) const noexcept
    {
        return !operator==(other);
    }

    EntityViewReverseIterator::EntityViewReverseIterator(const EntityView& view, ContainerIterator contItr, ElementIterator elmItr) noexcept
        : _contItr(contItr)
        , _elmItr(elmItr)
        , _beginElmItr(view.getElementReverseBegin())
        , _endElmItr(view.getElementReverseEnd())
        , _view(view)
    {
        if (_elmItr != _endElmItr && !view.contains(*_elmItr))
        {
            operator++();
        }
    }

    EntityViewReverseIterator::value_type EntityViewReverseIterator::operator*() const
    {
        return _elmItr.operator*();
    }

    EntityViewReverseIterator::pointer EntityViewReverseIterator::operator->() const
    {
        return _elmItr.operator->();
    }

    EntityViewReverseIterator& EntityViewReverseIterator::operator++() noexcept
    {
        do
        {
            if (!tryNextContainer())
            {
                ++_elmItr;
                tryNextContainer();
            }
        } while (_elmItr != _endElmItr && !_view.get().contains(*_elmItr));

        return *this;
    }

    EntityViewReverseIterator EntityViewReverseIterator::operator++(int) noexcept
    {
        EntityViewReverseIterator orig = *this;
        return ++(*this), orig;
    }

    EntityViewReverseIterator& EntityViewReverseIterator::operator--() noexcept
    {
        do
        {
            if (!tryPrevContainer())
            {
                --_elmItr;
                tryPrevContainer();
            }
        } while (_elmItr != _beginElmItr && !_view.get().contains(*_elmItr));
        return *this;
    }

    EntityViewReverseIterator EntityViewReverseIterator::operator--(int) noexcept
    {
        EntityViewReverseIterator orig = *this;
        return operator--(), orig;
    }

    bool EntityViewReverseIterator::tryNextContainer() noexcept
    {
        if (_elmItr == _contItr->get().rend())
        {
            _contItr++;
            if (_elmItr != _endElmItr)
            {
                _elmItr = _contItr->get().rbegin();
            }
            return true;
        }
        return false;
    }

    bool EntityViewReverseIterator::tryPrevContainer() noexcept
    {
        if (_elmItr == _contItr->get().rbegin())
        {
            _contItr--;
            if (_elmItr != _beginElmItr)
            {
                _elmItr = --_contItr->get().rend();
            }
            return true;
        }
        return false;
    }

    bool EntityViewReverseIterator::operator==(const EntityViewReverseIterator& other) const noexcept
    {
        return _contItr == other._contItr && _elmItr == other._elmItr;
    }

    bool EntityViewReverseIterator::operator!=(const EntityViewReverseIterator& other) const noexcept
    {
        return !operator==(other);
    }
}