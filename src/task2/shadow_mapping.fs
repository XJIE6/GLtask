#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;
uniform float specPower;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 pointLightPos;

void main()
{
    vec3 color = vec3(0.5);
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0, 1, 0);
	vec3 plightColor = vec3(0, 0, 1);

    // ambient
    vec3 ambient = 0.5 * lightColor;
	vec3 pambient = 0.5 * plightColor;

    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);

    vec3 plightDir = normalize(pointLightPos - fs_in.FragPos);
    float pdiff = max(dot(plightDir, normal), 0.0);

    vec3 curpos = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    // transform to [0,1] range
    curpos = curpos * 0.5 + 0.5;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // calculate shadow
    vec2 txsz = vec2(1.0 / textureSize(shadowMap, 0));
    vec2 p11 = floor(curpos.xy * textureSize(shadowMap, 0)) * txsz;
    vec2 p12 = p11 + vec2(0, txsz.y);
    vec2 p21 = p11 + vec2(txsz.x, 0);
    vec2 p22 = p11 + txsz;

    float sh11 = 0;
    for (int i = -1; i < 2; i++) {
         for (int j = -1; j < 2; j++) {
             float d = texture(shadowMap, p11 + vec2(i, j) * txsz).r;
             sh11 += curpos.z - bias > d ? 1.0 : 0.0;
         }
    }
    sh11 /= 9;

    float sh12 = 0;
    for (int i = -1; i < 2; i++) {
         for (int j = -1; j < 2; j++) {
             float d = texture(shadowMap, p12 + vec2(i, j) * txsz).r;
             sh12 += curpos.z - bias > d ? 1.0 : 0.0;
         }
    }
    sh12 /= 9;

    float sh21 = 0;
    for (int i = -1; i < 2; i++) {
         for (int j = -1; j < 2; j++) {
             float d = texture(shadowMap, p21 + vec2(i, j) * txsz).r;
             sh21 += curpos.z - bias > d ? 1.0 : 0.0;
         }
    }
    sh21 /= 9;

    float sh22 = 0;
    for (int i = -1; i < 2; i++) {
         for (int j = -1; j < 2; j++) {
             float d = texture(shadowMap, p22 + vec2(i, j) * txsz).r;
             sh22 += curpos.z - bias > d ? 1.0 : 0.0;
         }
    }
    sh22 /= 9;


    float shadow = 1 / txsz.x / txsz.y * (
             sh11 * (p22.x - curpos.x) * (p22.y - curpos.y)
           + sh12 * (p22.x - curpos.x) * (curpos.y - p11.y)
           + sh21 * (curpos.x - p11.x) * (p22.y - curpos.y)
           + sh22 * (curpos.x - p11.x) * (curpos.y - p11.y)
    );

    // float shadow = curpos.z - bias > texture(shadowMap, curpos.xy).r ? 1.0 : 0.0;
    // float shadow = curpos.z - bias > (d11 + d22) / 2 ? 1.0 : 0.0;
    // float shadow = sh11;

    if(curpos.z > 1.0)
        shadow = 0.0;

    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specPower);
    vec3 specular = spec * lightColor;

    vec3 phalfwayDir = normalize(plightDir + viewDir);
    float pspec = pow(max(dot(normal, phalfwayDir), 0.0), specPower);
    vec3 pspecular = pspec * plightColor;

    float plen = length(pointLightPos - fs_in.FragPos);
    float len = length(lightPos - fs_in.FragPos);

    if (dot(normalize(fs_in.FragPos - lightPos), normalize(-lightPos)) < 0.7) {
        diff = 0;
        ambient = vec3(0);
        specular = vec3(0);
    }

    vec3 lighting = (ambient + (1.0 - shadow) * (diff + specular)) * color * 10 / len / len +
        (pambient + pdiff + pspecular) * color / plen / plen;
    FragColor = vec4(lighting, 1.0);
}