#version 110

uniform sampler2D texture;

varying vec2 texcoord;

void main()
{
	const float K1 = 0.5;
	const float K2 = 0.5;
	const float r_2 = (texcoord.u-0.5)*(texcoord.u-0.5)+(texcoord.v-0.5)*(texcoord.v-0.5);
	 
	const vec2 texcoord2 = vec2(
	(texcoord.u-0.5)*(1+(K1+K2*r_2)*r_2),
	(texcoord.v-0.5)*(1+(K1+K2*r_2)*r_2)
	);
	
    gl_FragColor = texture2D(textures[0], texcoord2);
}
