project (imgui)

set (imgui_headers imconfig.h imgui.h imgui_impl_glfw.h imgui_impl_opengl3.h imgui_internal.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h)
set (imgui_sources imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp imgui_widgets.cpp)

add_library(imgui ${imgui_sources} ${imgui_headers})

target_include_directories(imgui PUBLIC ./)
target_link_libraries(imgui glfw glad)
