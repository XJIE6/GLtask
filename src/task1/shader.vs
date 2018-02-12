#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform float zoom;
uniform float x;
uniform float y;

out vec2 pos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    pos = vec2((aPos.x + x) / zoom, (aPos.y + y) / zoom);
}