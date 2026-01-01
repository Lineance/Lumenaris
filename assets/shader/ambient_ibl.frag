#version 330 core

// 来自顶点着色器的输入
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 InstanceColor;
in vec3 WorldPos;

// 输出颜色
out vec4 FragColor;

// ========================================
// 光源结构体（与C++类对应）
// ========================================

struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

// ========================================
// 光源数组
// ========================================

#define NR_DIR_LIGHTS 4
#define NR_POINT_LIGHTS 48
#define NR_SPOT_LIGHTS 8

uniform DirectionalLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform int nrDirLights;
uniform int nrPointLights;
uniform int nrSpotLights;

// ========================================
// 环境光照设置
// ========================================

uniform float ambientIntensity;  // 环境光强度
uniform int ambientMode;          // 0=固定颜色, 1=天空盒采样, 2=半球光照

// 天空盒环境光
uniform samplerCube ambientSkybox;  // 纹理单元 10（TextureUnit::AMBIENT_SKYBOX）

// 半球光照
uniform vec3 skyColor;      // 天空颜色
uniform vec3 groundColor;   // 地面颜色

// ========================================
// 材质属性
// ========================================

uniform vec3 objectColor;
uniform float shininess;
uniform bool useInstanceColor;
uniform bool useTexture;

uniform sampler2D textureSampler;  // 纹理单元 1（TextureUnit::MATERIAL_DIFFUSE）

// ========================================
// 视点位置
// ========================================

uniform vec3 viewPos;

// ========================================
// 函数声明
// ========================================

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcAmbientLight(vec3 normal);

void main()
{
    // 归一化法线
    vec3 norm = normalize(Normal);

    // 视线方向
    vec3 viewDir = normalize(viewPos - FragPos);

    // 获取基础颜色
    vec3 baseColor;
    if (useTexture)
    {
        vec4 texColor = texture(textureSampler, TexCoord);
        baseColor = texColor.rgb;
    }
    else if (useInstanceColor)
    {
        baseColor = InstanceColor;
    }
    else
    {
        baseColor = objectColor;
    }

    // ========================================
    // 计算环境光（轻量级IBL）- 不包含baseColor
    // ========================================
    vec3 ambient = CalcAmbientLight(norm);

    // ========================================
    // 计算直接光源的贡献
    // ========================================
    vec3 directLighting = vec3(0.0);

    // 平行光
    for (int i = 0; i < nrDirLights; ++i)
    {
        directLighting += CalcDirectionalLight(dirLights[i], norm, viewDir);
    }

    // 点光源
    for (int i = 0; i < nrPointLights; ++i)
    {
        directLighting += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // 聚光灯
    for (int i = 0; i < nrSpotLights; ++i)
    {
        directLighting += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }

    // ========================================
    // 合并结果（与multi_light.frag一致）
    // ========================================

    // 环境光 + 直接光，然后应用材质颜色
    vec3 result = (ambient + directLighting) * baseColor;

    // Gamma校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}

// ========================================
// 环境光计算（轻量级IBL）- 不包含baseColor
// ========================================

vec3 CalcAmbientLight(vec3 normal)
{
    vec3 ambient = vec3(0.0);

    if (ambientMode == 0)
    {
        // 模式0: 传统固定颜色环境光
        ambient = vec3(ambientIntensity);
    }
    else if (ambientMode == 1)
    {
        // 模式1: 从天空盒采样环境光
        vec3 skyboxColor = texture(ambientSkybox, normal).rgb;
        ambient = skyboxColor * ambientIntensity;
    }
    else if (ambientMode == 2)
    {
        // 模式2: 半球光照
        float hemiFactor = normal.y * 0.5 + 0.5;
        vec3 hemiColor = mix(groundColor, skyColor, hemiFactor);
        ambient = hemiColor * ambientIntensity;
    }

    return ambient;
}

// ========================================
// 光照计算函数（与multi_light.frag一致）
// ========================================

// 计算平行光贡献
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // 合并结果（包含环境光分量，但会被CalcAmbientLight覆盖）
    vec3 ambient = light.ambient * light.color;
    vec3 diffuse = light.diffuse * diff * light.color;
    vec3 specular = light.specular * spec * light.color;

    return (ambient + diffuse + specular);
}

// 计算点光源贡献
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 合并结果
    vec3 ambient = light.ambient * light.color;
    vec3 diffuse = light.diffuse * diff * light.color;
    vec3 specular = light.specular * spec * light.color;

    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// 计算聚光灯贡献
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 聚光灯强度（边缘柔化）
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // 合并结果
    vec3 ambient = light.ambient * light.color;
    vec3 diffuse = light.diffuse * diff * light.color;
    vec3 specular = light.specular * spec * light.color;

    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
