#version 330

// Input vertex data; different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

// Values that stay constant for the whole mesh
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
//uniform vec3 LightPosition_worldspace;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec3 Position_worldspace;

void main()
{
    UV = vertexUV;
    gl_Position = MVP*vec4(vertexPosition_modelspace,1);
}
