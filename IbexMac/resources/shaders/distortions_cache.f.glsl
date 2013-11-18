#version 120

#define highp

//precision highp float;

//uniform sampler2D texture;

uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;

varying vec2 oTexCoord;

vec3 HmdWarp(vec2 texIn)
{ 
   vec2  theta  = (texIn - LensCenter) * ScaleIn;
   float rSq = dot(theta,theta);
   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
   
   return vec3(vec2(LensCenter + Scale * theta1), rSq);
}

void main()
{
   vec3 tc = HmdWarp(oTexCoord);
   if (any(notEqual(clamp(tc.rg, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25, 0.5)) - tc.rg, vec2(0.0, 0.0))))
       gl_FragColor.rgb = vec3(-1.0, -1.0, 0.0);
   else
       gl_FragColor.rgb = vec3(tc.r, tc.g, tc.b); //texture2D(texture, tc);
}
