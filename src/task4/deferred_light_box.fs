#version 330 core
out vec4 FragColor;

in vec4 pos;
in vec4 cent;

void main()
{
    if (length(pos.xyz - cent.xyz) < 0.005) {
        FragColor = vec4(1, 0, 0, 1);
    }
    else {
        FragColor = vec4(0, 0, 0, 0);
    }
}