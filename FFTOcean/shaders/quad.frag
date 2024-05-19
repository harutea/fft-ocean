#version 430 core
out vec4 FragColor;
	
in vec2 TexCoords;
	
uniform sampler2D tex;
	
void main()
{             
    vec3 texCol = clamp(texture(tex, TexCoords).rgb, 0.0, 1.0);      
    FragColor = vec4(texCol, 1.0);
}