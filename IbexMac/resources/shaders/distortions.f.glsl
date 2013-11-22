#version 120

#define highp

//precision highp float;

uniform sampler2D Texture0;
uniform sampler2D lensTexture1;
uniform sampler2D lensTexture2;

varying vec2 oTexCoord;

void main()
{
    vec4 tc = texture2D(lensTexture1, oTexCoord).rgba;
    vec2 tcRed = tc.rg;
    vec2 tcGreen = tc.ba;
    
    vec2 tcBlue = texture2D(lensTexture2, oTexCoord).rg;
    if (tcBlue == vec2(0))
    {
        gl_FragColor = vec4(0,0,0,1);
        return;
    }
    
    // Now do blue texture lookup.
    float blue = texture2D(Texture0, tcBlue).b;
    
    // Do green lookup (no scaling).
    vec4  center = texture2D(Texture0, tcGreen);
    
    // Do red scale and lookup.
    float red = texture2D(Texture0, tcRed).r;
    
    gl_FragColor = vec4(red, center.g, blue, center.a);
}
