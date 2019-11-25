#pragma once

#include <string_view>

static constexpr std::string_view MvmVert = R"(
    #version 330 core
    layout (location = 0) in vec3 a_pos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_PointSize = 20.0;
        gl_Position = projection * view * model * vec4(a_pos, 1.0f);
    }
)";