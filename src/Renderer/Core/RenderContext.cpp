#include "Renderer/Core/RenderContext.hpp"
#include "Core/Logger.hpp"
#include <sstream>

namespace Renderer
{
    namespace Core
    {

        void RenderContext::Clear()
        {
            // 清空所有光源
            m_lightManager.ClearAll();

            // 重置环境光照
            m_ambientLighting.SetMode(AmbientLighting::Mode::SOLID_COLOR);
            m_ambientLighting.SetIntensity(0.1f);

            // 天空盒保持不变（不主动清空，用户可以手动调用Skybox的方法）
        }

        std::string RenderContext::GetStatistics() const
        {
            std::ostringstream oss;
            oss << "RenderContext Statistics:\n";
            oss << "----------------------------------------\n";
            oss << m_lightManager.GetStatistics() << "\n";
            oss << "Ambient Lighting Mode: ";

            switch (m_ambientLighting.GetMode())
            {
            case AmbientLighting::Mode::SOLID_COLOR:
                oss << "Solid Color";
                break;
            case AmbientLighting::Mode::SKYBOX_SAMPLE:
                oss << "Skybox Sample (IBL)";
                break;
            case AmbientLighting::Mode::HEMISPHERE:
                oss << "Hemisphere";
                break;
            }

            oss << "\n";
            oss << "Ambient Intensity: " << m_ambientLighting.GetIntensity() << "\n";
            oss << "----------------------------------------";

            return oss.str();
        }

    } // namespace Core
} // namespace Renderer
