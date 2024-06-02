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

uniform samplerCube sky;

void main()
{
    // Environment Mapping
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));
    vec3 environmentResult = texture(sky, R).rgb;

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
    float attenuation = 1.0 / (1.0 + 0.5 * distance + 0.05 * distance * distance);
    vec3 phongResult = (ambient + diffuse + specular) * attenuation;


    // Combine
    vec3 FinalColor = phongResult + environmentResult * 0.5;

    FragColor = vec4(FinalColor, 1.0);
}