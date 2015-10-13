#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 eyePosition;

out vec3 pos;
out vec3 reflected;

void main(){
    vec4 vpm4 = vec4(vertexPosition_modelspace,1);
    
	gl_Position =  MVP * vpm4;
    
    pos = (M*vpm4).xyz;
    vec4 cameraDir = vpm4;
    vec4 normal = vec4(0,1,0,1);
    
    reflected = normalize(reflect(cameraDir, (normal))).xyz;
}

