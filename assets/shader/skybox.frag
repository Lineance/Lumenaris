#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);

    // HDR色调映射（可选）
    // FragColor.rgb = FragColor.rgb / (FragColor.rgb + vec3(1.0));

    // Gamma校正
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/2.2));
}
