#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

uniform vec3 light;

void main()
{
    vec3 color = vec3(1, 1, 1);
    float len = max(length(fs_in.FragPos - light), 5);
    FragColor = vec4(color * dot(fs_in.Normal, normalize(light - fs_in.FragPos)) / len / len, 1);
}