#version 330 core

// Input vertex data; different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
//layout(location = 1) in vec3 vertexNormal_modelspace;
//layout(location = 2) in vec2 vertexUV;
//layout(location = 3) in vec3 vertexTangent_modelspace;
//layout(location = 4) in vec3 vertexBitangent_modelspace;

// Values that stay constant for the whole mesh
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 DepthBiasMVP;
uniform vec3 LightPosition_worldspace;
uniform mat3 MV3x3;
uniform vec3 playerPosition_modelspace;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec3 Position_worldspace;

out vec3 vertexNormal_cameraspace;
out vec3 vertexTangent_cameraspace;
out vec3 vertexBitangent_cameraspace;

out vec3 EyeDirection_tangentspace;
out vec3 LightDirection_tangentspace;
out vec3 ShadowCoord;
//out float tex;

float snoise(vec3 v);
float snoise(vec2 v);


#define playerHeight 10.0f
#define translateX 0.0f
#define translateZ 0.0f
#define scaleX 50.0f
#define scaleZ 50.0f
#define minHeight -20.0f
#define maxHeight 500.0f
#define f1 0.7f
#define f2 1.4f
#define f3 3.0f

float getNoiseHeight(const float x_, const float z_) {
    vec2 v = vec2((x_-translateX)/scaleX, (z_-translateZ)/scaleZ);
    
    return ((snoise(v*f1)
             +snoise(v*f2)*0.2f
             +snoise(v*f3)*0.1f))*(maxHeight-minHeight) + minHeight;
}

vec3 getPos(float x, float z) {
    return vec3(x, getNoiseHeight(x/scaleX, z/scaleX)+playerHeight, z);
}
vec2 getUV(float x, float z) {
    return vec2(x,z)*2.0f/scaleX;
}
void main()
{
    //vec3 newVert_modelSpace = vertexPosition_modelspace+floor(playerPosition_modelspace/scaleX)*scaleX;
    vec2 p = vertexPosition_modelspace.xz-floor(playerPosition_modelspace.xz/scaleX)*scaleX;

    // Shortcuts for vertices
    vec3 v0 = getPos(p.x,p.y);
    vec3 v1 = getPos(p.x+1,p.y);
    vec3 v2 = getPos(p.x,p.y+1);
    
    // Shortcuts for UVs
    vec2 uv0 = getUV(p.x,p.y);
    vec2 uv1 = getUV(p.x+1,p.y);
    vec2 uv2 = getUV(p.x,p.y+1);
    
    
    vec4 vert_Modelspace = vec4(v0,1);//vec4(newVert_modelSpace.x, y, newVert_modelSpace.z,1);
    // Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vert_Modelspace).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( V * M * vert_Modelspace).xyz;//vec4(vertexPosition_modelspace,1)).xyz;
	vec3 EyeDirection_cameraspace = vec3(0.0f,0.0f,0.0f) - vertexPosition_cameraspace;
    
	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
    
    // Edges of the triangle : postion delta
    vec3 deltaPos1 = v1-v0;
    vec3 deltaPos2 = v2-v0;
    
    // UV delta
    vec2 deltaUV1 = uv1-uv0;
    vec2 deltaUV2 = uv2-uv0;
    
    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    vec3 tangent_modelspace = normalize((deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r);
    vec3 bitangent_modelspace = normalize((deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r);
    
    vec3 normal_modelspace = normalize(cross(v1-v0, v2-v0));
    
    vertexNormal_cameraspace = MV3x3 * normal_modelspace;
    vertexTangent_cameraspace = MV3x3 * tangent_modelspace;
    vertexBitangent_cameraspace = MV3x3 * bitangent_modelspace;
    
    mat3 TBN = transpose(mat3(
                              vertexTangent_cameraspace,
                              vertexBitangent_cameraspace,
                              vertexNormal_cameraspace
                              )); // You can use dot products instead of building this matrix and transposing it. See References for details.
    
    LightDirection_tangentspace = normalize(TBN * LightDirection_cameraspace);
    EyeDirection_tangentspace =  normalize(TBN * EyeDirection_cameraspace);
    
    UV = uv0;
    gl_Position = MVP * vert_Modelspace;
    
    // Same, but with the light's view matrix
    ShadowCoord = (DepthBiasMVP * vert_Modelspace).xyz;
}

//////////////////////// INCLUDED /////////////////
//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
    
    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;
    
    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );
    
    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y
    
    // Permutations
    i = mod289(i);
    vec4 p = permute( permute( permute(
                                       i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
                              + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
                     + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
    
    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;
    
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)
    
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)
    
    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    
    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );
    
    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    
    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;
    
    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);
    
    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    
    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                 dot(p2,x2), dot(p3,x3) ) );
}




//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//
vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    
    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    
    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                     + i.x + vec3(0.0, i1.x, 1.0 ));
    
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    
    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
    
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    
    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    
    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

//////////////////////// END INCLUDED /////////////////