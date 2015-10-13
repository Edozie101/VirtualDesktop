#version 330 core

// Input vertex data; different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexUV;

// Values that stay constant for the whole mesh
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 DepthBiasMVP;
uniform vec3 LightPosition_worldspace;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec4 ShadowCoord;

void main()
{
    //vec3 LightPosition_worldspace = vec3(0, 5, 5);
    
    vec4 vpm4 = vec4(vertexPosition_modelspace,1);
    vec4 vnm4 = vec4(vertexNormal_modelspace,0);

    // Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vpm4).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = (V * M * vpm4).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
    
	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = (V * vec4(LightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = normalize(LightPosition_cameraspace + EyeDirection_cameraspace);
	
	// Normal of the the vertex, in camera space
	Normal_cameraspace = normalize((V * M * vnm4).xyz); // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
    
    UV = vertexUV;
    gl_Position = MVP*vpm4;
    
    // Same, but with the light's view matrix
    ShadowCoord = DepthBiasMVP * vpm4; //vec4(vertexPosition_modelspace,1);
}
