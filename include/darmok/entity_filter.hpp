#pragma once

#include <bx/bx.h>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class BX_NO_VTABLE IEntityFilter
    {
    public:
        virtual ~IEntityFilter() = default;
        virtual void init(EntityRegistry& registry) { _registry = registry; };
        virtual void operator()(EntityRuntimeView& view) const = 0;
    private:
        OptionalRef<EntityRegistry> _registry;
    protected:
        EntityRegistry& getRegistry() const { return _registry.value(); }
    };

    template<typename T>
    class EntityComponentFilter final : public IEntityFilter
    {
    public:
        void operator()(EntityRuntimeView& view) const override
        {
            view.iterate(getRegistry().storage<T>());
        }
    };
}