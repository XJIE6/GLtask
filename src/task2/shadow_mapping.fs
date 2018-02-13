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

vec4 Bilinear(sampler2D tex, vec2 texCoord, int texSize)
{
   vec2 trTexCoord = texCoord*texSize;
   vec2 texf = floor(trTexCoord);
   vec2 ratio = trTexCoord - texf;
   vec2 opposite = 1.0 - ratio;
   vec4 result = (texture(tex, texf/texSize) * opposite.x  + texture(tex, (texf+vec2(1, 0))/texSize)   * ratio.x) * opposite.y +
                   (texture(tex, (texf+vec2(0, 1))/texSize) * opposite.x + texture(tex, (texf+vec2(1, 1))/texSize) * ratio.x) * ratio.y;
   return result;
 }

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


    float bilinDepth = Bilinear(shadowMap, projCoords.xy, textureSize(shadowMap, 0).x).r;
    float shadow = currentDepth - bias > bilinDepth  ? 1.0 : 0.0;
    
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