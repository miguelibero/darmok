#include <darmok-editor/editor.hpp>

namespace darmok::editor
{
    ObjectEditorContainer::ObjectEditorContainer()
    {
    }

    bool ObjectEditorContainer::render(google::protobuf::Any& obj) const
    {
        auto itr = _editors.find(obj.type_url());
        if (itr == _editors.end())
        {
            return false;
        }
        for (auto& editor : itr->second)
        {
            if (editor->canRender(obj))
            {
                return editor->render(obj);
            }
        }
        return false;
    }

    void ObjectEditorContainer::add(std::unique_ptr<IObjectEditor>&& editor)
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