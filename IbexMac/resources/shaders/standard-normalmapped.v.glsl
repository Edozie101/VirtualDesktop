#version 330 core

// Input vertex data; different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;

// Values that stay constant for the whole mesh
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 DepthBiasMVP;
uniform vec3 LightPosition_worldspace;
uniform mat3 MV3x3;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec3 Position_worldspace;

out vec3 vertexNormal_cameraspace;
out vec3 vertexTangent_cameraspace;
out vec3 vertexBitangent_cameraspace;

out vec3 EyeDirection_tangentspace;
out vec3 LightDirection_tangentspace;
out vec4 ShadowCoord;

void main()
{
    // Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
    
	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
    
    vertexNormal_cameraspace = MV3x3 * normalize(vertexNormal_modelspace);
    vertexTangent_cameraspace = MV3x3 * normalize(vertexTangent_modelspace);
    vertexBitangent_cameraspace = MV3x3 * normalize(vertexBitangent_modelspace);
    
    mat3 TBN = transpose(mat3(
                              vertexTangent_cameraspace,
                              vertexBitangent_cameraspace,
                              vertexNormal_cameraspace
                              )); // You can use dot products instead of building this matrix and transposing it. See References for details.
    
    LightDirection_tangentspace = normalize(TBN * LightDirection_cameraspace);
    EyeDirection_tangentspace =  normalize(TBN * EyeDirection_cameraspace);
    
    UV = vertexUV;
    gl_Position = MVP*vec4(vertexPosition_modelspace,1);
    
    // Same, but with the light's view matrix
    ShadowCoord = DepthBiasMVP * vec4(vertexPosition_modelspace,1); //vec4(vertexPosition_modelspace,1);
}
