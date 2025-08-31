#include <darmok-editor/editor.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>

namespace darmok::editor
{
    void BaseObjectEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
		_container = container;
    }

    void BaseObjectEditor::shutdown()
    {
		_app.reset();
		_container.reset();
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

    BaseObjectEditor::RenderResult BaseObjectEditor::renderChild(google::protobuf::Message& msg) noexcept
    {
		return _container->render(msg, false);
    }

    std::optional<Entity> BaseObjectEditor::getEntity(const Any& anyComp) const noexcept
    {
		return getProject().getSceneDefinition().getEntity(anyComp);
    }

    std::optional<std::filesystem::path> BaseObjectEditor::getAssetPath(const Any& anyAsset) const noexcept
    {
        return getProject().getSceneDefinition().getAssetPath(anyAsset);
    }

    ObjectEditorContainer::ObjectEditorContainer()
    {
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
        return unexpected<std::string>{"could not find an editor to render"};
    }

    void ObjectEditorContainer::add(std::unique_ptr<IObjectEditor>&& editor) noexcept
    {
        auto typeUrl = editor->getObjectTypeUrl();
        if (_app)
        {
            editor->init(_app.value(), *this);
        }
        auto& vec = _editors[typeUrl];
        vec.push_back(std::move(editor));
    }

    void ObjectEditorContainer::init(EditorApp& app)
    {
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                editor->init(app, *this);
            }
        }
        _app = app;
    }

    void ObjectEditorContainer::shutdown()
    {
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                editor->shutdown();
            }
        }
        _app.reset();
    }
}