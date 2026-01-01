#include "Renderer/Environment/AmbientLighting.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    AmbientLighting::AmbientLighting()
        : m_skyboxTextureID(0)
        , m_intensity(0.3f)
        , m_enabled(true)
        , m_mode(Mode::SOLID_COLOR)
        , m_skyColor(0.5f, 0.7f, 1.0f)    // 默认蓝天
        , m_groundColor(0.1f, 0.1f, 0.1f) // 默认深灰地面
    {
    }

    AmbientLighting::~AmbientLighting()
    {
        // 不删除纹理，由Skybox类管理
    }

    AmbientLighting::AmbientLighting(AmbientLighting&& other) noexcept
        : m_skyboxTextureID(other.m_skyboxTextureID)
        , m_intensity(other.m_intensity)
        , m_enabled(other.m_enabled)
        , m_mode(other.m_mode)
        , m_skyColor(other.m_skyColor)
        , m_groundColor(other.m_groundColor)
    {
        other.m_skyboxTextureID = 0;
    }

    AmbientLighting& AmbientLighting::operator=(AmbientLighting&& other) noexcept
    {
        if (this != &other)
        {
            m_skyboxTextureID = other.m_skyboxTextureID;
            m_intensity = other.m_intensity;
            m_enabled = other.m_enabled;
            m_mode = other.m_mode;
            m_skyColor = other.m_skyColor;
            m_groundColor = other.m_groundColor;

            other.m_skyboxTextureID = 0;
        }
        return *this;
    }

    bool AmbientLighting::Initialize()
    {
        Core::Logger::GetInstance().Info("Ambient lighting system initialized");
        return true;
    }

    bool AmbientLighting::LoadFromSkybox(unsigned int skyboxTextureID, float intensity)
    {
        if (skyboxTextureID == 0)
        {
            Core::Logger::GetInstance().Error("Invalid skybox texture ID, cannot create ambient lighting");
            return false;
        }

        m_skyboxTextureID = skyboxTextureID;
        m_intensity = intensity;
        m_mode = Mode::SKYBOX_SAMPLE;

        Core::Logger::GetInstance().Info("Ambient lighting loaded from skybox, intensity: " +
                                         std::to_string(intensity));
        return true;
    }

    void AmbientLighting::BindTexture(unsigned int textureUnit) const
    {
        if (m_skyboxTextureID != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTextureID);
        }
    }

    void AmbientLighting::ApplyToShader(Shader& shader) const
    {
        if (!m_enabled)
        {
            shader.SetFloat("ambientIntensity", 0.0f);
            return;
        }

        // 设置环境光强度
        shader.SetFloat("ambientIntensity", m_intensity);

        // 设置环境光模式
        shader.SetInt("ambientMode", static_cast<int>(m_mode));

        // 根据模式设置不同的参数
        switch (m_mode)
        {
            case Mode::SOLID_COLOR:
                // 使用传统固定颜色环境光
                break;

            case Mode::SKYBOX_SAMPLE:
                // 绑定天空盒纹理
                if (m_skyboxTextureID != 0)
                {
                    BindTexture(10);
                    shader.SetInt("ambientSkybox", 10);
                }
                break;

            case Mode::HEMISPHERE:
                // 设置半球光照颜色
                shader.SetVec3("skyColor", m_skyColor);
                shader.SetVec3("groundColor", m_groundColor);
                break;
        }
    }

} // namespace Renderer
