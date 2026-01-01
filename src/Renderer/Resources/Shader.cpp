#include "Renderer/Resources/Shader.hpp"
#include "Core/Logger.hpp"
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
        Core::Logger::GetInstance().Info("Loading shader program from: " + vertexPath + " and " + fragmentPath);

        // 读取文件
        std::ifstream vFile(vertexPath);
        std::ifstream fFile(fragmentPath);

        if (!vFile.is_open())
        {
            Core::Logger::GetInstance().Error("Failed to open vertex shader file: " + vertexPath);
            throw std::runtime_error("Failed to open vertex shader file: " + vertexPath);
        }

        if (!fFile.is_open())
        {
            Core::Logger::GetInstance().Error("Failed to open fragment shader file: " + fragmentPath);
            throw std::runtime_error("Failed to open fragment shader file: " + fragmentPath);
        }

        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();

        std::string vertexCode = vStream.str();
        std::string fragmentCode = fStream.str();

        // 编译顶点着色器
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
            Core::Logger::GetInstance().Error("Vertex shader compilation failed: " + std::string(infoLog));
            glDeleteShader(vertex);
            throw std::runtime_error("Vertex Shader compilation failed: " + std::string(infoLog));
        }

        // 编译片段着色器
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fCode = fragmentCode.c_str();
        glShaderSource(fragment, 1, &fCode, nullptr);
        glCompileShader(fragment);

        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
            Core::Logger::GetInstance().Error("Fragment shader compilation failed: " + std::string(infoLog));
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            throw std::runtime_error("Fragment Shader compilation failed: " + std::string(infoLog));
        }

        // 链接着色器程序
        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);

        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
            Core::Logger::GetInstance().Error("Shader program linking failed: " + std::string(infoLog));
            glDeleteProgram(m_id);
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            m_id = 0;
            throw std::runtime_error("Shader Program linking failed: " + std::string(infoLog));
        }

        Core::Logger::GetInstance().Info("Shader program linked successfully, ID: " + std::to_string(m_id));

        // 清理
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // 对于OpenGL的封装
    void Shader::Use() const
    {
        glUseProgram(m_id);
        Core::Logger::GetInstance().LogShaderActivation(m_id);
    }

    // 传递矩阵、向量、整数与浮点数到着色器
    void Shader::SetMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::SetVec3(const std::string &name, const glm::vec3 &vec) const
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

    void Shader::SetBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
    }

} // namespace Renderer