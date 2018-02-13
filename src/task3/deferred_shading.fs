#version 330 core
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform vec3 Position;
uniform vec3 Color;
uniform float Linear;
uniform float Quadratic;
uniform float Radius;
uniform vec3 viewPos;

void main()
{
    vec2 TexCoords = gl_FragCoord.xy / textureSize(gPosition, 0);
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = vec3(1); //texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = 0.5; //texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting = vec3(0);
    vec3 viewDir  = normalize(viewPos - FragPos);

    // calculate distance between light source and current fragment
    float distance = length(Position - FragPos);
    if(distance < Radius)
    {
        //lighting  = Diffuse * 0.1; // hard-coded ambient component
        // diffuse
        vec3 lightDir = normalize(Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = Color * spec * Specular;
        // attenuation
        float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }

    FragColor = vec4(lighting, 1.0);
}
