#pragma once

#include <darmok-editor/scene.hpp>

namespace darmok
{
    class Transform;
}

namespace darmok::editor
{
    class TransformEditor final : IComponentEditor<Transform>
    {
    public:
        void render(Transform& trans) noexcept override;
    };
}