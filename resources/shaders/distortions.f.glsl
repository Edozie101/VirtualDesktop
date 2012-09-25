#version 120

#define highp

//precision highp float;

uniform sampler2D texture;

varying vec2 texcoord;

// based off of: http://github.prideout.net/barrel-distortion/
// Given a vec2 in [-1,+1], generate a texture coord in [0,+1]
vec2 Distort(vec2 p)
{
	const float BarrelPower = 1.25;
    float theta  = atan(p.y, p.x);
    float radius = length(p);
    radius = pow(radius, BarrelPower);
    p.x = radius * cos(theta);
    p.y = radius * sin(theta);
    return 0.5 * (p + 1.0);
}

// based off of: http://paulbourke.net/miscellaneous/lenscorrection/
vec2 Distort2(vec2 p)
{
	// these factors range from 0 to 0.1 for a wide-angle lens
	const float ax = 0.1;
	const float ay = 0.1;
	
	float len = length(p);
	float len_2 = len*len;
	
	float xx = len/(1.0 - ax*len_2);
	float yy = len/(1.0 - ay*len_2);
	
	return (p/vec2(1.0-ax*xx*xx,1.0-ay*yy*yy)+1.0)*0.5;
}

void main()
{
    gl_FragColor = texture2D(texture, clamp(Distort2(texcoord), 0.0, 1.0));
}
