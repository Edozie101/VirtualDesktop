#version 330

uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;
uniform vec4 ChromAbParam;
uniform sampler2D Texture0;
in vec2 oTexCoord;

//layout (location = 0)
out vec4 fragData[2];

// Scales input texture coordinates for distortion.
// ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
// larger due to aspect ratio.
void main()
{
   vec2  theta = (oTexCoord - LensCenter) * ScaleIn; // Scales to [-1, 1]
   float rSq= theta.x * theta.x + theta.y * theta.y;
   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + 
                  HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
   
   // Detect whether blue texture coordinates are out of range since these will scaled out the furthest.
   vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);
   vec2 tcBlue = LensCenter + Scale * thetaBlue;
   if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tcBlue)))
   {
       fragData[0] = vec4(0);
       fragData[1] = vec4(0);
       return;
   }
   
   // Do green lookup (no scaling).
   vec2  tcGreen = LensCenter + Scale * theta1;
   
   // Do red scale and lookup.
   vec2  thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);
   vec2  tcRed = LensCenter + Scale * thetaRed;
   
    //gl_FragColor = vec4(0);//red, center.g, blue, center.a);
    fragData[0] = vec4(tcRed, tcGreen);
    fragData[1] = vec4(tcBlue, 0.0, 0.0);
}

