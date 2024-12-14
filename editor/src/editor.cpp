#include <darmok-editor/editor.hpp>

namespace darmok::editor
{
    ObjectEditorContainer::ObjectEditorContainer()
    {
    }

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
        if (_assets && _proj)
        {
            editor->init(_assets.value(), _proj.value(), *this);
        }
        auto& vec = _editors[type.hash()];
        vec.push_back(std::move(editor));
    }

    void ObjectEditorContainer::init(AssetContext& assets, EditorProject& proj)
    {
        for (auto& [type, editors] : _editors)
        {
            for (auto& editor : editors)
            {
                editor->init(assets, proj, *this);
            }
        }
        _assets = assets;
        _proj = proj;
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
        _assets.reset();
        _proj.reset();
    }
}