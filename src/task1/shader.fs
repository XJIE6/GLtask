#version 330 core

in vec2 pos;

uniform sampler2D ourTexture;
uniform float mul;

out vec4 color;

float y = -0.9;
float size = 0.6;
float x = -size / 2;

int inBox()
{
    if (pos.x > x && pos.x <= x + size && pos.y > y && pos.y <= y + size)
        return 1;

    if (pos.y <= y + size)
        return 0;

    if (pos.x < x + size / 2)
        return 3;

    return 2;
}

void main()
{
    for (int i = 0; i < 100; ++i)
    {
        int cur = inBox();
        if (cur == 0)
        {
            color = vec4(1, 1, 1, 1);
            return;
        }
        else if (cur == 1)
        {
            color = texture(ourTexture, vec2((pos.x - x) / size, (y - pos.y) / size));
            return;
        }
        y += size;
        if (cur == 2) {
            x += size;
        }
        else {
            x -= size / mul;
        }
        size /= mul;
    }
    color = vec4(0, 0, 0, 1);
}