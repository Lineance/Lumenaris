#pragma once
#include <string>
#include "Core/GLM.hpp"

namespace Renderer
{

    class Shader
    {
        unsigned int m_id = 0;

    public:
        Shader() = default;
        ~Shader();

        void Load(const std::string &vertexPath, const std::string &fragmentPath);
        void Use() const;

        // Uniform 设置
        void SetMat4(const std::string &name, const glm::mat4 &mat) const;
        void SetVec3(const std::string &name, const glm::vec3 &vec) const;
        void SetFloat(const std::string &name, float value) const;
        void SetInt(const std::string &name, int value) const;
        void SetBool(const std::string &name, bool value) const;

        // 新增：获取OpenGL程序ID
        unsigned int GetID() const { return m_id; }
    };

} // namespace Renderer