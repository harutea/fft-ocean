#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;  

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
    /* Phong Lighting, Phong Shading */

    // ambient
    vec3 ambient = material.ambient * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * material.diffuse) * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (spec * material.specular) * lightColor;  

    // attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.05 * distance * distance);
    vec3 result = (ambient + diffuse + specular) * attenuation;

    vec3 foamColor = vec3(1.0);
    float foamFactor = smoothstep(0.5, 1.0, mix(0.0, 1.0, FragPos.y)) * dot(norm, vec3(0.0, 1.0, 0.0));

    FragColor = vec4(mix(result, foamColor, foamFactor), 1.0);
}