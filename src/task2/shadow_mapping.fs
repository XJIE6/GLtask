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

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

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

    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

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