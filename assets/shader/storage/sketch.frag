#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

// 简单的噪声函数
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// 创建交叉线条图案
float sketchPattern(vec2 uv, float intensity) {
    // 水平线条
    float horizontal = abs(fract(uv.y * 50.0) - 0.5) * 2.0;
    horizontal = smoothstep(0.4, 0.6, horizontal);

    // 垂直线条
    float vertical = abs(fract(uv.x * 50.0) - 0.5) * 2.0;
    vertical = smoothstep(0.4, 0.6, vertical);

    // 对角线条 (45度)
    float diagonal1 = abs(fract((uv.x + uv.y) * 35.0) - 0.5) * 2.0;
    diagonal1 = smoothstep(0.4, 0.6, diagonal1);

    // 对角线条 (135度)
    float diagonal2 = abs(fract((uv.x - uv.y) * 35.0) - 0.5) * 2.0;
    diagonal2 = smoothstep(0.4, 0.6, diagonal2);

    // 结合所有线条
    float lines = min(min(horizontal, vertical), min(diagonal1, diagonal2));

    // 根据光照强度调整线条密度
    float threshold = mix(0.1, 0.8, intensity);
    return step(threshold, lines);
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 漫反射强度
    float diff = max(dot(norm, lightDir), 0.0);

    // 添加一些环境光
    float ambient = 0.3;

    // 总光照强度
    float lightIntensity = diff + ambient;

    // 使用屏幕空间UV坐标创建线条
    vec2 screenUV = gl_FragCoord.xy / 800.0; // 假设屏幕分辨率

    // 生成素描图案
    float sketch = sketchPattern(screenUV, lightIntensity);

    // 添加一些噪波让素描更自然
    float noise = random(screenUV * 100.0) * 0.1;
    sketch += noise;

    // 素描通常是黑白效果，但可以用objectColor的亮度来调整
    float brightness = dot(objectColor, vec3(0.299, 0.587, 0.114)); // RGB转亮度
    vec3 sketchColor = mix(vec3(0.1), vec3(0.9), sketch * brightness);

    // 添加一些纸张的纹理
    float paperTexture = random(screenUV * 200.0) * 0.05 + 0.95;
    sketchColor *= paperTexture;

    FragColor = vec4(sketchColor, 1.0);
}
