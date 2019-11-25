#include "shader.hpp"

#include <sstream>
#include <vector>

// namespace Shaders {
//     struct Mvm {
//         static constexpr std::string_view Vert = MvmVert;
//         static constexpr std::string_view Frag = MvmFrag;
//     };

//     struct LightMvm {
//         static constexpr std::string_view Vert = LightMvmVert;
//         static constexpr std::string_view Frag = LightMvmFrag;
//     };
// }

Shader::Shader(GLuint program) 
    : m_program(program)
{}

GLuint Shader::compile_shader(const char * shader_source, GLenum shader_type) {
    auto shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);

    auto status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE) {
        auto length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        std::vector<GLchar> log(length);
        glGetShaderInfoLog(shader, length, nullptr, log.data());

        throw std::runtime_error(const_cast<const char *>(log.data()));
    }

    return shader;
}

GLuint Shader::link_program(GLuint vs, GLuint fs) {
    auto program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    auto status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(status != GL_TRUE) {
        auto length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        std::vector<GLchar> log(length);
        glGetProgramInfoLog(program, length, nullptr, log.data());

        throw std::runtime_error(const_cast<const char *>(log.data()));
    }

    return program;
}

void Shader::use() {
    glUseProgram(m_program);
}

void Shader::set_bool(const char *name, bool value) const
{   
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform1i(variable, (int)value); 
}

void Shader::set_int(const char *name, int value) const
{ 
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform1i(variable, value); 
}

void Shader::set_float(const char *name, float value) const
{ 
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform1f(variable, value); 
}

void Shader::set_vec2(const char *name, const glm::vec2 &value) const
{ 
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform2fv(variable, 1, &value[0]); 
}

void Shader::set_vec3(const char *name, const glm::vec3 &value) const
{ 
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform3fv(variable, 1, &value[0]); 
}

void Shader::set_vec4(const char *name, const glm::vec4 &value) const
{ 
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniform4fv(variable, 1, &value[0]); 
}

void Shader::set_mat2(const char *name, const glm::mat2 &mat) const
{
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniformMatrix2fv(variable, 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat3(const char *name, const glm::mat3 &mat) const
{
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniformMatrix3fv(variable, 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_mat4(const char *name, const glm::mat4 &mat) const
{
    auto variable = glGetUniformLocation(m_program, name);

#ifdef __DEBUG__
    if(variable == -1) {
        std::stringstream error;
        error << "Unknown_variable: ";
        error << name;
        throw std::runtime_error(error.str().c_str());
    }
#endif

    glUniformMatrix4fv(variable, 1, GL_FALSE, &mat[0][0]);
}