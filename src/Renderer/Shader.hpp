#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Renderer
{
    class Shader
    {
    public:
        Shader() = default;
        ~Shader();

        void Load(const std::string &vertexPath, const std::string &fragmentPath);
        void Use() const;

        // Uniform setters
        void SetBool(const std::string &name, bool value) const;
        void SetInt(const std::string &name, int value) const;
        void SetFloat(const std::string &name, float value) const;
        void SetVec3(const std::string &name, const glm::vec3 &vec) const;
        void SetMat4(const std::string &name, const glm::mat4 &mat) const;

    private:
        unsigned int m_id = 0;
    };
} // namespace Renderer
