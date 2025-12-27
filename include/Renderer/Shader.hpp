#pragma once
#include <string>
#include <Core/MathTypes.hpp>

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
        void SetMat4(const std::string &name, const Core::Mat4 &mat) const;
        void SetVec3(const std::string &name, const Core::Vec3 &vec) const;
        void SetFloat(const std::string &name, float value) const;
        void SetInt(const std::string &name, int value) const;
    };

} // namespace Renderer