#include <darmok-editor/editor.hpp>
#include <darmok-editor/utils.hpp>

namespace darmok::editor
{
    ObjectEditorContainer::ObjectEditorContainer()
    {
    }

    bool ObjectEditorContainer::render(google::protobuf::Message& obj, bool withTitle) const
    {
		auto typeUrl = protobuf::getTypeUrl(obj);
        auto itr = _editors.find(typeUrl);
        if (itr == _editors.end())
        {
            return false;
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