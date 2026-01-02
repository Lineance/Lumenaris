#pragma once

#include "Core/GLM.hpp"
#include "Renderer/Resources/Shader.hpp"
#include <string>
#include <memory>
#include <cstdint>
#include <cstdio>

namespace Renderer
{
    namespace Lighting
    {

        /**
         * 光源类型枚举
         */
        enum class LightType
        {
            DIRECTIONAL,  // 平行光（方向光，如太阳光）
            POINT,        // 点光源（从一个点向所有方向发光，如灯泡）
            SPOT          // 聚光灯（从一个点向特定方向锥形发光）
        };

        /**
         * LightHandle - 光源句柄（修复索引失效问题）
         *
         * 设计要点：
         * - 使用稳定的 generation ID 而非容器索引
         * - 禁用拷贝，仅可移动
         * - 类型安全，包含光源类型标签
         */
        class LightHandle
        {
        public:
            LightHandle() : m_id(0), m_generation(0), m_type(LightType::DIRECTIONAL) {}

            LightHandle(size_t id, size_t generation, LightType type)
                : m_id(id), m_generation(generation), m_type(type) {}

            // 禁用拷贝
            LightHandle(const LightHandle&) = delete;
            LightHandle& operator=(const LightHandle&) = delete;

            // 允许移动
            LightHandle(LightHandle&&) noexcept = default;
            LightHandle& operator=(LightHandle&&) noexcept = default;

            // 访问器
            size_t GetId() const { return m_id; }
            size_t GetGeneration() const { return m_generation; }
            LightType GetType() const { return m_type; }

            // 有效性检查
            bool IsValid() const { return m_generation > 0; }

        private:
            size_t m_id;          // 稳定的 ID
            size_t m_generation;  // 代数标记（用于检测失效句柄）
            LightType m_type;     // 光源类型
        };

        /**
         * Light 类 - 光照系统基类
         *
         * 定义了所有光源的通用属性：
         * - 颜色
         * - 强度
         * - 开关状态
         * - 环境光、漫反射、镜面反射分量
         */
        class Light
        {
        protected:
            // ✅ 性能优化（2026-01-02）：线程局部格式化缓冲区
            // 避免每帧构造大量字符串（48 光源 × 10 uniform = 480 次字符串分配）
            // 使用 snprintf 直接格式化到栈上缓冲区，零堆分配
            struct UniformFormatter
            {
                char buffer[64];

                const char* formatUniform(const char* lightType, int index, const char* property)
                {
                    snprintf(buffer, sizeof(buffer), "%s[%d].%s", lightType, index, property);
                    return buffer;
                }
            };

            // 线程局部存储（每个线程独享一份，无锁竞争）
            static thread_local UniformFormatter s_formatter;
        public:
            // 构造函数
            Light(
                const glm::vec3 &color = glm::vec3(1.0f),
                float intensity = 1.0f,
                float ambient = 0.1f,
                float diffuse = 0.8f,
                float specular = 0.5f);

            // 虚析构函数
            virtual ~Light() = default;

            // ========================================
            // 通用属性访问
            // ========================================

            // 颜色
            const glm::vec3 &GetColor() const { return m_color; }
            void SetColor(const glm::vec3 &color) { m_color = color; }

            // 强度
            float GetIntensity() const { return m_intensity; }
            void SetIntensity(float intensity) { m_intensity = intensity; }

            // 状态开关
            bool IsEnabled() const { return m_enabled; }
            void SetEnabled(bool enabled) { m_enabled = enabled; }
            void Toggle() { m_enabled = !m_enabled; }

            // 光照分量
            float GetAmbient() const { return m_ambient; }
            void SetAmbient(float ambient) { m_ambient = ambient; }

            float GetDiffuse() const { return m_diffuse; }
            void SetDiffuse(float diffuse) { m_diffuse = diffuse; }

            float GetSpecular() const { return m_specular; }
            void SetSpecular(float specular) { m_specular = specular; }

            // ========================================
            // 虚函数接口（派生类实现）
            // ========================================

            /**
             * 获取光源类型
             */
            virtual LightType GetType() const = 0;

            /**
             * 将光源数据传递给着色器
             * @param shader 着色器对象
             * @param index 光源索引（用于数组uniform）
             */
            virtual void ApplyToShader(Shader &shader, int index = 0) const = 0;

            /**
             * 获取光源描述（用于调试）
             */
            virtual std::string GetDescription() const = 0;

        protected:
            // 光照属性
            glm::vec3 m_color;
            float m_intensity;
            bool m_enabled;

            // 光照分量（Phong光照模型）
            float m_ambient;   // 环境光分量
            float m_diffuse;   // 漫反射分量
            float m_specular;  // 镜面反射分量
        };

        /**
         * DirectionalLight 类 - 平行光（方向光）
         *
         * 特点：
         * - 来自一个方向，没有位置
         * - 所有光线平行
         * - 不随距离衰减
         * - 适用于：太阳光、月光
         */
        class DirectionalLight : public Light
        {
        public:
            DirectionalLight(
                const glm::vec3 &direction = glm::vec3(-0.2f, -1.0f, -0.3f),
                const glm::vec3 &color = glm::vec3(1.0f),
                float intensity = 1.0f,
                float ambient = 0.1f,
                float diffuse = 0.8f,
                float specular = 0.5f);

            // Light接口实现
            LightType GetType() const override { return LightType::DIRECTIONAL; }
            void ApplyToShader(class Shader &shader, int index = 0) const override;
            std::string GetDescription() const override;

            // 方向属性
            const glm::vec3 &GetDirection() const { return m_direction; }
            void SetDirection(const glm::vec3 &direction) { m_direction = direction; }

        private:
            glm::vec3 m_direction;
        };

        /**
         * LightWithAttenuation 类 - 带衰减的光源基类 ⭐ NEW
         *
         * 设计目标：
         * - 消除 PointLight 和 SpotLight 之间的代码重复
         * - 提供位置和衰减参数的公共实现
         * - 支持多态计算有效距离
         *
         * 架构优势：
         * - ✅ 遵循 DRY 原则（Don't Repeat Yourself）
         * - ✅ 统一衰减参数管理
         * - ✅ 支持虚函数多态调用
         */
        class LightWithAttenuation : public Light
        {
        public:
            /**
             * 衰减参数
             * @param constant 常数项（通常为1.0）
             * @param linear 线性项
             * @param quadratic 二次项
             */
            struct Attenuation
            {
                float constant;
                float linear;
                float quadratic;

                // 预设的衰减配置
                static Attenuation Range7()   { return {1.0f, 0.7f, 1.8f}; }   // 7米
                static Attenuation Range13()  { return {1.0f, 0.35f, 0.44f}; }  // 13米
                static Attenuation Range20()  { return {1.0f, 0.22f, 0.20f}; }  // 20米
                static Attenuation Range32()  { return {1.0f, 0.14f, 0.07f}; }  // 32米
                static Attenuation Range50()  { return {1.0f, 0.09f, 0.032f}; } // 50米
                static Attenuation Range65()  { return {1.0f, 0.07f, 0.017f}; } // 65米
                static Attenuation Range100() { return {1.0f, 0.045f, 0.0075f}; } // 100米
            };

            // ========================================
            // 位置属性
            // ========================================

            const glm::vec3 &GetPosition() const { return m_position; }
            void SetPosition(const glm::vec3 &position) { m_position = position; }

            // ========================================
            // 衰减属性
            // ========================================

            const Attenuation &GetAttenuation() const { return m_attenuation; }
            void SetAttenuation(const Attenuation &attenuation) { m_attenuation = attenuation; }

            // ========================================
            // 虚函数：计算有效距离（支持多态）
            // ========================================

            /**
             * 计算有效光照距离（近似值）
             *
             * ⭐ 虚函数设计：支持多态调用
             * - PointLight: 纯距离衰减
             * - SpotLight: 距离衰减 + 角度衰减
             *
             * @return 有效距离（米）
             */
            virtual float GetEffectiveRange() const;

        protected:
            // 保护构造函数，仅允许派生类构造
            LightWithAttenuation(
                const glm::vec3 &color,
                float intensity,
                float ambient,
                float diffuse,
                float specular,
                const glm::vec3 &position,
                const Attenuation &attenuation)
                : Light(color, intensity, ambient, diffuse, specular)
                , m_position(position)
                , m_attenuation(attenuation)
            {}

            glm::vec3 m_position;
            Attenuation m_attenuation;
        };

        /**
         * PointLight 类 - 点光源（重构：继承 LightWithAttenuation）
         *
         * 特点：
         * - 从一个点向所有方向发光
         * - 有位置信息
         * - 随距离衰减
         * - 适用于：灯泡、蜡烛、火焰
         *
         * ⭐ 架构优化：继承 LightWithAttenuation 基类
         */
        class PointLight : public LightWithAttenuation
        {
        public:
            using Attenuation = LightWithAttenuation::Attenuation;  // 类型别名

            PointLight(
                const glm::vec3 &position = glm::vec3(0.0f),
                const glm::vec3 &color = glm::vec3(1.0f),
                float intensity = 1.0f,
                float ambient = 0.1f,
                float diffuse = 0.8f,
                float specular = 0.5f,
                const Attenuation &attenuation = Attenuation::Range20());

            // Light接口实现
            LightType GetType() const override { return LightType::POINT; }
            void ApplyToShader(class Shader &shader, int index = 0) const override;
            std::string GetDescription() const override;

            // ⭐ 重写基类虚函数（支持多态）
            float GetEffectiveRange() const override;
        };

        /**
         * SpotLight 类 - 聚光灯（⭐ 重构：继承 LightWithAttenuation）
         *
         * 特点：
         * - 从一个点向特定方向锥形发光
         * - 有位置和方向
         * - 有截止角度（内锥和外锥）
         * - 随距离衰减
         * - 适用于：手电筒、台灯、舞台灯光
         *
         * ⭐ 架构优化：
         * - 继承 LightWithAttenuation（消除代码重复）
         * - 避免违反 Liskov 原则
         * - 支持多态调用 GetEffectiveRange()
         */
        class SpotLight : public LightWithAttenuation
        {
        public:
            using Attenuation = LightWithAttenuation::Attenuation;  // 类型别名

            SpotLight(
                const glm::vec3 &position = glm::vec3(0.0f),
                const glm::vec3 &direction = glm::vec3(0.0f, -1.0f, 0.0f),
                const glm::vec3 &color = glm::vec3(1.0f),
                float intensity = 1.0f,
                float ambient = 0.1f,
                float diffuse = 0.8f,
                float specular = 0.5f,
                const Attenuation &attenuation = LightWithAttenuation::Attenuation::Range20(),
                float cutOff = glm::radians(12.5f),
                float outerCutOff = glm::radians(15.0f));

            // Light接口实现
            LightType GetType() const override { return LightType::SPOT; }
            void ApplyToShader(class Shader &shader, int index = 0) const override;
            std::string GetDescription() const override;

            // ========================================
            // 位置和衰减（继承自 LightWithAttenuation）
            // ========================================

            // ⭐ 不再需要重复实现，直接使用基类的：
            // - GetPosition() / SetPosition()
            // - GetAttenuation() / SetAttenuation()

            // ========================================
            // 方向属性（SpotLight 特有）
            // ========================================

            const glm::vec3 &GetDirection() const { return m_direction; }
            void SetDirection(const glm::vec3 &direction) { m_direction = direction; }

            // ========================================
            // 截止角度（SpotLight 特有）
            // ========================================

            float GetCutOff() const { return m_cutOff; }
            void SetCutOff(float cutOff) { m_cutOff = cutOff; }

            float GetOuterCutOff() const { return m_outerCutOff; }
            void SetOuterCutOff(float outerCutOff) { m_outerCutOff = outerCutOff; }

            // 获取截止角度的度数
            float GetCutOffDegrees() const { return glm::degrees(m_cutOff); }
            float GetOuterCutOffDegrees() const { return glm::degrees(m_outerCutOff); }

            // 设置截止角度（度数）
            void SetCutOffDegrees(float degrees) { m_cutOff = glm::radians(degrees); }
            void SetOuterCutOffDegrees(float degrees) { m_outerCutOff = glm::radians(degrees); }

            // ========================================
            // ⭐ 重写虚函数（支持多态）
            // ========================================

            /**
             * 计算聚光灯有效距离
             *
             * 考虑因素：
             * - 距离衰减（继承自 LightWithAttenuation）
             * - 角度衰减（SpotLight 特有）
             */
            float GetEffectiveRange() const override;

        private:
            glm::vec3 m_direction;
            float m_cutOff;        // 内锥角度（弧度）
            float m_outerCutOff;   // 外锥角度（弧度，用于边缘柔化）
        };

        // 类型别名
        using LightPtr = std::shared_ptr<Light>;
        using DirectionalLightPtr = std::shared_ptr<DirectionalLight>;
        using PointLightPtr = std::shared_ptr<PointLight>;
        using SpotLightPtr = std::shared_ptr<SpotLight>;

    } // namespace Lighting
} // namespace Renderer
