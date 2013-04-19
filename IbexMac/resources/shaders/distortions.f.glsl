#version 120

#define highp

//precision highp float;

uniform sampler2D texture;
uniform sampler2D lensTexture;

varying vec2 texcoord;

void main()
{
   vec2 tc = texture2D(lensTexture, texcoord).rg;
   if(tc == vec2(-1,-1))
       gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
   else
       gl_FragColor = texture2D(texture, tc);
}