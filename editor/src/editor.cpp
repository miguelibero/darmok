#include <darmok-editor/editor.hpp>

namespace darmok::editor
{
    bool ObjectEditorContainer::render(entt::meta_any& obj) const
    {
        auto itr = _editors.find(obj.type().info().hash());
        if (itr == _editors.end())
        {
            return false;
        }
        for (auto& editor : itr->second)
        {
            if (editor->tryRender(obj))
            {
                return true;
            }
        }
        return false;
    }

    void ObjectEditorContainer::add(std::unique_ptr<IObjectEditor>&& editor)
    {
        auto type = editor->getObjectType();
        if (_app)
        {
            editor->init(_app.value(), *this);
        }
        auto& vec = _editors[type.hash()];
        vec.push_back(std::move(editor));
    }

    void ObjectEditorContainer::init(EditorAppDelegate& app)
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
        _app.reset();
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                editor->shutdown();
            }
        }
    }
}