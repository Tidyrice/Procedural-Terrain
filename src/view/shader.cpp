#include <glad/glad.h>
#include "shader.h"
#include <GL/freeglut.h>
#include <iostream>
#include <sstream>
#include <fstream>

Shader::Shader(std::string vertex_shader_path, std::string fragment_shader_path)
{
    std::string vertex_shader_src = ReadFile(vertex_shader_path);
    std::string fragment_shader_src = ReadFile(fragment_shader_path);
    InitializeShaders(vertex_shader_src.c_str(), fragment_shader_src.c_str());
}

void
Shader::InitializeShaders(const char* vertex_shader_src, const char* fragment_shader_src)
{
    int success;
    char infoLog[512];

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        std::cerr << "[ERROR]: Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        std::cerr << "[ERROR]: Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glLinkProgram(shader_program_);
    glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program_, 512, NULL, infoLog);
        std::cerr << "[ERROR]: Shader program linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void
Shader::UseShader() const
{
    glUseProgram(shader_program_);
}

std::string
Shader::ReadFile(std::string filename) {
    std::cout<<filename<<std::endl;
    std::ifstream f(filename);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
