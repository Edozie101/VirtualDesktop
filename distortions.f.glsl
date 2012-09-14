#version 120

#define highp

//precision highp float;

uniform sampler2D texture;

varying vec2 texcoord;

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

void main()
{
	//const float K1 = 0.5;
	//const float K2 = 0.5;
	//float r_2 = (texcoord.s-0.5)*(texcoord.s-0.5)+(texcoord.t-0.5)*(texcoord.t-0.5);
	 
	//vec2 texcoord2 = vec2(
	//(texcoord.s-0.5)*(1.0+(K1+K2*r_2)*r_2),
	//(texcoord.t-0.5)*(1.0+(K1+K2*r_2)*r_2)
	//);
	
    gl_FragColor = texture2D(texture, clamp(Distort(texcoord), 0.0, 1.0));//texcoord2);
}
