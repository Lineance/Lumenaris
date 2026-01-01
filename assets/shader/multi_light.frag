#version 330 core

// 来自顶点着色器的输入
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 InstanceColor;

// 输出颜色
out vec4 FragColor;

// ========================================
// 光源结构体（与C++类对应）
// ========================================

// 平行光
struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
};

// 点光源
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

// 聚光灯
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
#define NR_POINT_LIGHTS 16
#define NR_SPOT_LIGHTS 8

uniform DirectionalLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

uniform int nrDirLights;
uniform int nrPointLights;
uniform int nrSpotLights;

// ========================================
// 材质属性
// ========================================

uniform vec3 objectColor;
uniform float shininess;
uniform bool useInstanceColor;
uniform bool useTexture;

uniform sampler2D textureSampler;

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
    // 计算所有光源的贡献
    // ========================================

    vec3 result = vec3(0.0);

    // 平行光
    for (int i = 0; i < nrDirLights; ++i)
    {
        result += CalcDirectionalLight(dirLights[i], norm, viewDir);
    }

    // 点光源
    for (int i = 0; i < nrPointLights; ++i)
    {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // 聚光灯
    for (int i = 0; i < nrSpotLights; ++i)
    {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }

    // 应用材质颜色
    result *= baseColor;

    // HDR色调映射（简单版本）
    // result = result / (result + vec3(1.0));

    // Gamma校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}

// ========================================
// 光照计算函数
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

    // 合并结果
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

    ambient *= attenuation;
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

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
