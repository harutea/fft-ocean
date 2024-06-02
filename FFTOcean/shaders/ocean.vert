#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec3 FragPos;
out vec3 Normal;

out float heightMax;
out float heightMin;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform sampler2D heightMap;
uniform float texelSize;

const float heightScale = 1.0;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    FragPos.y += heightScale * texture(heightMap, aTexCoord).r;

    // Calculate Normal Vector (using Central Difference)
    float L = heightScale * texture(heightMap, aTexCoord + vec2(-1.0, 0.0) * texelSize).r;
    float R = heightScale * texture(heightMap, aTexCoord + vec2(1.0, 0.0) * texelSize).r;
    float D = heightScale * texture(heightMap, aTexCoord + vec2(0.0, -1.0) * texelSize).r;
    float U = heightScale * texture(heightMap, aTexCoord + vec2(0.0, 1.0) * texelSize).r;

    float dX = (R - L)/2.0;
    float dY = (U - D)/2.0;
    Normal = normalize(vec3(-dX, 1.0, -dY));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}