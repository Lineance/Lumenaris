#include "Renderer/Shader.hpp"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Renderer
{

    Shader::~Shader()
    {
        if (m_id != 0)
        {
            glDeleteProgram(m_id);
        }
    }

    void Shader::Load(const std::string &vertexPath, const std::string &fragmentPath)
    {
        // 读取文件
        std::ifstream vFile(vertexPath);
        std::ifstream fFile(fragmentPath);

        if (!vFile.is_open() || !fFile.is_open())
        {
            throw std::runtime_error("Failed to open shader files");
        }

        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();

        std::string vertexCode = vStream.str();
        std::string fragmentCode = fStream.str();

        // 编译
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        const char *vCode = vertexCode.c_str();
        glShaderSource(vertex, 1, &vCode, nullptr);
        glCompileShader(vertex);

        int success;
        char infoLog[512];
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
            throw std::runtime_error("Vertex Shader compilation failed: " + std::string(infoLog));
        }

        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fCode = fragmentCode.c_str();
        glShaderSource(fragment, 1, &fCode, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
            throw std::runtime_error("Fragment Shader compilation failed: " + std::string(infoLog));
        }

        // 链接
        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);

        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
            throw std::runtime_error("Shader Program linking failed: " + std::string(infoLog));
        }

        // 清理
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void Shader::Use() const
    {
        glUseProgram(m_id);
    }

    void Shader::SetMat4(const std::string &name, const Core::Mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::SetVec3(const std::string &name, const Core::Vec3 &vec) const
    {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &vec[0]);
    }

    void Shader::SetFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void Shader::SetInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

} // namespace Renderer