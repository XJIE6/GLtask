#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
    FragColor = vec4((texture(depthMap, TexCoords).xyz + vec3(1)) / 2, 1.0);
}