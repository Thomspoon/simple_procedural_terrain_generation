// Shader adapted from the following tutorials:
// https://learnopengl.com/Lighting/Basic-Lighting

#pragma once

#include <string_view>

static constexpr std::string_view TerrainVert = R"(
    // Model view matrix vertex shader

    #version 330 core
    layout (location = 0) in vec3 a_pos;
    layout (location = 1) in vec3 a_normal;
    layout (location = 2) in vec3 a_color;
    //layout (location = 2) in vec2 a_tex_coord;

    out vec3 fragment_pos;
    out vec3 surface_normal;
    out vec3 fragment_color;
    //out vec2 tex_coord;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        fragment_pos = vec3(model * vec4(a_pos, 1.0));
        surface_normal = mat3(transpose(inverse(model))) * a_normal;
        fragment_color = a_color;
        //tex_coord = a_tex_coord;

        gl_Position = projection * view * model * vec4(a_pos, 1.0f);
    }
)";
