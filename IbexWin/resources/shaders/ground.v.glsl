#version 330

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace,1);
    
    UV = v.xz;//normalize(v).xz;
    gl_Position = projectionMatrix*(modelViewMatrix*v);
}
