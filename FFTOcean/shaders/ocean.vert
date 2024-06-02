#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform sampler2D heightMap;
uniform float texelSize;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    //FragPos.y += 1.*sin(FragPos.z + time);
    FragPos.y += 0.2 * texture(heightMap, aTexCoord).r;
    // Normal = normalMatrix * aNormal;

    // Calculate Normal Vector
    float L = texture(heightMap, aTexCoord + vec2(-1.0, 0.0) * texelSize).r;
    float R = texture(heightMap, aTexCoord + vec2(1.0, 0.0) * texelSize).r;
    float D = texture(heightMap, aTexCoord + vec2(0.0, -1.0) * texelSize).r;
    float U = texture(heightMap, aTexCoord + vec2(0.0, 1.0) * texelSize).r;

    float dX = (R - L)/2.0;
    float dY = (U - D)/2.0;
    Normal = normalize(vec3(-dX, 1.0, -dY));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}