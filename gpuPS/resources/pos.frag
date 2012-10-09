//#extension GL_ARB_draw_buffers : enable
uniform sampler2D posArray;
uniform sampler2D velArray;
varying vec4 texCoord;

void main(void)
{	
	float mass	= texture2D( posArray, texCoord.st).a;
	vec3 p		= texture2D( posArray, texCoord.st).rgb;
	vec3 v		= texture2D( velArray, texCoord.st).rgb;
	vec3 acc	= -0.0002*p; // Centripetal force
	vec3 ayAcc  = 0.00001*normalize(cross(vec3(0, 1 ,0),p)); // Angular force

	vec3 new_v  = v + mass*(acc+ayAcc);	
	vec3 new_p	= p + new_v;
	
	//Render to positions texture
	gl_FragData[0] = vec4(new_p.x, new_p.y, new_p.z, mass);
	//Render to velocities texture
	gl_FragData[1] = vec4(new_v.x, new_v.y, new_v.z, 1.0);

}