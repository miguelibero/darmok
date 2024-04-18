
set(IMGUI_BASE ${BGFX_BASE}/3rdparty/dear-imgui)

add_library(
  imgui
  ${IMGUI_BASE}/imgui_draw.cpp
  ${IMGUI_BASE}/imgui_tables.cpp
  ${IMGUI_BASE}/imgui_widgets.cpp
  ${IMGUI_BASE}/imgui.cpp
)

target_include_directories(
    imgui
    PRIVATE
    ${BGFX_BASE}/3rdparty
)

set_property(TARGET imgui PROPERTY CXX_STANDARD 20)
target_link_libraries(imgui bx)