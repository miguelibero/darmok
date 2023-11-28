#pragma once

#include <bgfx/bgfx.h>
#include <darmok/input.hpp>
#include <darmok/window.hpp>

#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

namespace darmok
{
    void imguiCreate(float fontSize = 18.0f);
    void imguiDestroy();

    void imguiBeginFrame(WindowHandle window, Utf8Char inputChar = {}, bgfx::ViewId viewId = 255);
    void imguiEndFrame();
}