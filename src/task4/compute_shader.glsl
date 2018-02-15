#version 430 compatibility
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout( std140, binding=4 ) buffer Pos
{
    vec4 Positions[ ];
};
layout( std140, binding=5 ) buffer Vel
{
    vec4 Velocities[ ];
};
layout( std140, binding=6 ) buffer Col
{
    vec4 Colors[ ];
};
layout( std140, binding=7 ) buffer Pla
{
    vec3 Plane[ ];
};

uniform mat4 view;
uniform int h;

layout( local_size_x = 256, local_size_y = 1, local_size_z = 1 ) in;

void main() {
    const vec3 G = vec3(0, -0.1, 0);
    const float DT = 0.1;

    uint gid = gl_GlobalInvocationID.x;

    vec3 p = Positions[ gid ].xyz;
    vec3 v = Velocities[ gid ].xyz;

    vec3 pp = p + v*DT + .5*DT*DT*G;
    vec3 vp = v + G*DT;

    vec3 posA, posB, posC, normA, normB, normC;

    float flo =  pp.x + pp.z - floor(pp.x) - floor(pp.z);
    float cei = -pp.x - pp.z + ceil (pp.x) + ceil (pp.z);
    if (flo < cei) {
        int _x = int(floor(pp.x));
        int _z = int(floor(pp.z));
        posA = Plane[(_x * (h + 1) + _z) * 2];
        posB = Plane[((_x + 1) * (h + 1) + _z) * 2];
        posC = Plane[(_x * (h + 1) + _z + 1) * 2];
        normA = Plane[(_x * (h + 1) + _z) * 2 + 1];
        normB = Plane[((_x + 1) * (h + 1) + _z) * 2 + 1];
        normC = Plane[(_x * (h + 1) + _z + 1) * 2 + 1];
    }
    else {
        int _x = int(ceil(pp.x));
        int _z = int(ceil(pp.z));
        posA = Plane[(_x * (h + 1) + _z) * 2];
        posB = Plane[((_x - 1) * (h + 1) + _z) * 2];
        posC = Plane[(_x * (h + 1) + _z - 1) * 2];
        normA = Plane[(_x * (h + 1) + _z) * 2 + 1];
        normB = Plane[((_x - 1) * (h + 1) + _z) * 2 + 1];
        normC = Plane[(_x * (h + 1) + _z - 1) * 2 + 1];
    }

    if ((posA.y + posB.y + posC.y) / 3 > pp.y) {
        vec3 norm = -normalize(normA + normB + normC);
        vp = reflect(vp, norm);
        pp.y = (posA.y + posB.y + posC.y) / 3;
    }

    Positions[ gid ].xyz = pp;
    Velocities[ gid ].xyz = vp;

    vec4 newPos = Positions[ gid ];
    Colors[gid * 6 + 0] = vec4(0, 0.2, 0, 0);
    Colors[gid * 6 + 1] = newPos;
    Colors[gid * 6 + 2] = vec4(0.1738, -0.1, 0, 0);
    Colors[gid * 6 + 3] = newPos;
    Colors[gid * 6 + 4] = vec4(-0.1738, -0.1, 0, 0);
    Colors[gid * 6 + 5] = newPos;
}