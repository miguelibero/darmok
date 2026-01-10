#include <darmok-editor/editor.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>

namespace darmok::editor
{
    expected<void, std::string> BaseObjectEditor::init(EditorApp& app, ObjectEditorContainer& container) noexcept
    {
        _app = app;
		_container = container;
        return {};
    }

    expected<void, std::string> BaseObjectEditor::shutdown() noexcept
    {
		_app.reset();
		_container.reset();
        return {};
    }

    EditorProject& BaseObjectEditor::getProject() noexcept
    {
		return _app->getProject();
    }

    const EditorProject& BaseObjectEditor::getProject() const noexcept
    {
        return _app->getProject();
    }

    Scene& BaseObjectEditor::getScene() noexcept
    {
		return *getProject().getScene();
    }

    const Scene& BaseObjectEditor::getScene() const noexcept
    {
        return *getProject().getScene();
    }

    SceneDefinitionWrapper& BaseObjectEditor::getSceneDefinition() noexcept
    {
        return getProject().getSceneDefinition();
    }

    const SceneDefinitionWrapper& BaseObjectEditor::getSceneDefinition() const noexcept
    {
        return getProject().getSceneDefinition();
    }

    EditorApp& BaseObjectEditor::getApp() noexcept
    {
        return *_app;
    }

    const EditorApp& BaseObjectEditor::getApp() const noexcept
    {
        return *_app;
    }

    IComponentLoadContext& BaseObjectEditor::getComponentLoadContext() noexcept
    {
		return getProject().getComponentLoadContext();
    }

    expected<void, std::string> BaseObjectEditor::reloadAsset(const std::filesystem::path& path) noexcept
    {
        return getProject().getAssets().reloadAsset(path);
    }

    expected<void, std::string> BaseObjectEditor::removeAsset(const std::filesystem::path& path) noexcept
    {
        return getProject().getAssets().removeAsset(path);
    }

    BaseObjectEditor::RenderResult BaseObjectEditor::renderChild(google::protobuf::Message& msg, bool withTitle) noexcept
    {
		return _container->render(msg, withTitle);
    }

    EntityId BaseObjectEditor::getEntityId(const Any& anyComp) const noexcept
    {
		return getProject().getSceneDefinition().getEntity(anyComp);
    }

    Entity BaseObjectEditor::getEntity(EntityId entityId) const noexcept
    {
        return getProject().getComponentLoadContext().getEntity(entityId);

    }

    std::optional<std::filesystem::path> BaseObjectEditor::getAssetPath(const Any& anyAsset) const noexcept
    {
        return getProject().getSceneDefinition().getAssetPath(anyAsset);
    }

    ObjectEditorContainer::RenderResult ObjectEditorContainer::render(google::protobuf::Message& obj, bool withTitle) const noexcept
    {
		auto typeUrl = protobuf::getTypeUrl(obj);
        auto itr = _editors.find(typeUrl);
        if (itr == _editors.end())
        {
            return unexpected<std::string>{"could not find an editor for the type"};
        }
        for (auto& editor : itr->second)
        {
            if (editor->canRender(obj))
            {
                if (withTitle && !ImguiUtils::beginFrame(editor->getTitle().c_str()))
                {
                    ImguiUtils::endFrame();
                    return false;
                }
                auto result = editor->render(obj);
                if (withTitle)
                {
                    ImguiUtils::endFrame();
                }
                return result;
            }
        }
        ImGui::Text(fmt::format("[{}]", typeUrl).c_str());
        return unexpected<std::string>{"could not find an editor to render"};
    }

    expected<void, std::string> ObjectEditorContainer::add(std::unique_ptr<IObjectEditor> editor) noexcept
    {
        auto typeUrl = editor->getObjectTypeUrl();
        if (_app)
        {
            auto result = editor->init(_app.value(), *this);
            if (!result)
            {
                return result;
            }
        }
        auto& vec = _editors[typeUrl];
        vec.push_back(std::move(editor));
        return {};
    }

    expected<void, std::string> ObjectEditorContainer::init(EditorApp& app) noexcept
    {
        std::vector<std::string> errors;
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                auto result = editor->init(app, *this);
                if (!result)
                {
					errors.push_back(type + ": " + result.error());
                }
            }
        }
        _app = app;
		return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> ObjectEditorContainer::shutdown() noexcept
    {
        std::vector<std::string> errors;
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                auto result = editor->shutdown();
                if (!result)
                {
                    errors.push_back(type + ": " + result.error());
                }
            }
        }
        _app.reset();
        return StringUtils::joinExpectedErrors(errors);
    }
}