#pragma once

#include <darmok-editor/app.hpp>

namespace darmok::editor
{
    class SkeletonEditorAppComponent final : public IEditorAppComponent
    {
        expected<void, std::string> renderMainMenu(MainMenuSection section) noexcept override;
        expected<void, std::string> init(EditorApp& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;

        OptionalRef<EditorApp> _app;
    };
}