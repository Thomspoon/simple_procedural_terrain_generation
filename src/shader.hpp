#pragma once

#include "glad/glad.h"
#include "glm/mat4x4.hpp"

#include "shaders/mvm.vert"
#include "shaders/mvm.frag"
#include "shaders/light_mvm.vert"
#include "shaders/light_mvm.frag"
#include "shaders/terrain.vert"
#include "shaders/terrain.frag"

namespace Shaders {
    struct Mvm {
        static constexpr std::string_view Vert = MvmVert;
        static constexpr std::string_view Frag = MvmFrag;
    };

    struct LightMvm {
        static constexpr std::string_view Vert = LightMvmVert;
        static constexpr std::string_view Frag = LightMvmFrag;
    };

    struct Terrain {
        static constexpr std::string_view Vert = TerrainVert;
        static constexpr std::string_view Frag = TerrainFrag;
    };
}

class Shader {
public:

    template<typename CustomShader>
    static Shader create() {
        auto vs = Shader::compile_shader(CustomShader::Vert.data(), GL_VERTEX_SHADER);
        auto fs = Shader::compile_shader(CustomShader::Frag.data(), GL_FRAGMENT_SHADER);

        auto program = Shader::link_program(vs, fs);

        glDeleteShader(fs);
        glDeleteShader(vs);

        return Shader(program);
    }

    void use();

    void set_bool(const char *name, bool value) const;
    void set_int(const char *name, int value) const;
    void set_float(const char *name, float value) const;
    void set_vec2(const char *name, const glm::vec2 &value) const;
    void set_vec3(const char *name, const glm::vec3 &value) const;
    void set_vec4(const char *name, const glm::vec4 &value) const;
    void set_mat2(const char *name, const glm::mat2 &mat) const;
    void set_mat3(const char *name, const glm::mat3 &mat) const;
    void set_mat4(const char *name, const glm::mat4 &mat) const;

private:
    Shader(GLuint);

    static GLuint compile_shader(const char *shader, GLenum shader_type);
    static GLuint link_program(GLuint vs, GLuint fs);

    GLuint m_program;
};