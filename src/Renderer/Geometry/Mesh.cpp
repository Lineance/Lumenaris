#include "Renderer/Geometry/Mesh.hpp"
#include "Core/Logger.hpp"
#include <stdexcept>

namespace Renderer
{

    // 无需定义静态成员，inline static 已在类内完成

    void MeshFactory::Register(const std::string &type,
                               std::function<std::unique_ptr<IMesh>()> creator)
    {
        s_registry[type] = creator;
    }

    std::unique_ptr<IMesh> MeshFactory::Create(const std::string &type)
    {
        auto it = s_registry.find(type);
        if (it != s_registry.end())
        {
            return it->second();
        }
        Core::Logger::GetInstance().Warning("Unknown mesh type requested: " + type);
        return nullptr;
    }

} // namespace Renderer