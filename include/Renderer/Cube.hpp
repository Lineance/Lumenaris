#pragma once
#include "Renderer/Geometry.hpp"

namespace Renderer
{

    class Cube : public IGeometry
    {
        GLuint m_vao = 0, m_vbo = 0;

    public:
        void Create() override;
        void Draw() const override;
    };

    // ✅ 编译期注册（C++17 保证初始化顺序）
    namespace
    {
        struct CubeRegistrar
        {
            CubeRegistrar()
            {
                GeometryFactory::Register("CUBE", []()
                                          { return std::make_unique<Cube>(); });
            }
        };
        inline const CubeRegistrar g_cubeRegistrar; // inline 避免重复定义
    }

} // namespace Renderer