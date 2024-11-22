#pragma once

#include <wx/wx.h>

namespace darmok::editor
{
    class App : public wxApp
    {
    public:
        bool OnInit() override;
    };
}