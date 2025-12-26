#include "Renderer/Geometry.hpp"
#include <stdexcept>

namespace Renderer
{

    // 无需定义静态成员，inline static 已在类内完成

    void GeometryFactory::Register(const std::string &type,
                                   std::function<std::unique_ptr<IGeometry>()> creator)
    {
        s_registry[type] = creator;
    }

    std::unique_ptr<IGeometry> GeometryFactory::Create(const std::string &type)
    {
        auto it = s_registry.find(type);
        if (it != s_registry.end())
        {
            return it->second();
        }
        return nullptr;
    }

} // namespace Renderer