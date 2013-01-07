//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform sampler2D positions;
uniform sampler2D velocities;
varying vec4 texCoord;

void main(void)
{	
	float mass	= texture2D( positions, texCoord.st).a;
	vec3 p		= texture2D( positions, texCoord.st).rgb;
	vec3 v		= texture2D( velocities, texCoord.st).rgb;
    
    float x0    = 0.5; //distance from center of sphere to be maintaned
    float x     = distance(p, vec3(0,0,0)); // current distance
	vec3 acc	= -0.0002*(x - x0)*p; //apply spring force (hooke's law)
    
	vec3 new_v  = v + mass*(acc);
    new_v = 0.999*new_v; // friction to slow down velocities over time
	vec3 new_p	= p + new_v;
	
	//Render to positions texture
	gl_FragData[0] = vec4(new_p.x, new_p.y, new_p.z, mass);
	//Render to velocities texture
	gl_FragData[1] = vec4(new_v.x, new_v.y, new_v.z, 1.0);
}