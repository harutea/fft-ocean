#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
//layout (location = 1) in vec3 aNormal;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform sampler2D displacement;


void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    //FragPos.y += 1.*sin(FragPos.z + time);
    FragPos.y += 0.07 * texture(displacement, aTexCoord).r;
    // Normal = normalMatrix * aNormal;
    Normal = vec3(0.0, 1.0, 0.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}