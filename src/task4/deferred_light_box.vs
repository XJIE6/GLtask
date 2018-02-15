#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aCenter;

out vec4 pos;
out vec4 cent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    cent = view * model * aCenter;
    pos = (cent + model * aPos);
    gl_Position = projection * pos;
}