#pragma once

#include <bx/bx.h>

namespace darmok::editor
{
    template<typename T>
    class BX_NO_VTABLE IComponentEditor
    {
    public:
        virtual ~IComponentEditor() = default;
        virtual void render(T& comp) = 0;
    };
}