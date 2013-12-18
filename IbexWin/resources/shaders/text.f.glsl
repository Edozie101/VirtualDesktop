#version 330 core

#define highp

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
layout (location = 0) out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;

void main()
{
    //color = vec4(vec3(texture(textureIn,UV).r),1);//vec4(0,0.5,0.3,1.0-texture(textureIn, UV).r);
	//color = texture(textureIn,UV);//vec4(0,0.5,0.3,1.0-texture(textureIn, UV).r);
	
	color = vec4(1,1,1,texture(textureIn, UV).r);
}
