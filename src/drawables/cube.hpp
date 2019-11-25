#pragma once

#include "../drawable.hpp"

namespace {
    static float vertex_attributes[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
}

class Cube : public Drawable<Cube> {
public:
    explicit Cube(VertexArrayObject&& t_vao, VertexBufferObject&& t_vbo) : Drawable(std::move(t_vao)), vbo(std::move(t_vbo)) {}

    static std::shared_ptr<Cube> create_impl() {
        auto cube_vao = VertexArrayObject();
        auto cube_vbo = VertexBufferObject(VertexBufferType::ARRAY);

        cube_vao.bind();
        cube_vbo.bind();

        cube_vbo.send_data(vertex_attributes, VertexDrawType::STATIC);

        cube_vbo.enable_attribute_pointer(0, 3, VertexDataType::FLOAT, 3, 0);
        
        cube_vao.unbind();

        return std::make_shared<Cube>(std::move(cube_vao), std::move(cube_vbo));
    }

    DrawType draw_impl() const {
        vao.bind();

        auto draw_type = DrawArrays {
            VertexPrimitive::TRIANGLES,
            0,
            36
        };

        return DrawType(draw_type);
    }

private:
    VertexBufferObject vbo;
};