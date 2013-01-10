//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

#define EPS 0.001

uniform sampler2D positions;
uniform sampler2D velocities;
uniform vec3 attractorPos;
varying vec4 texCoord;

void main(void)
{
vec3 p0 = texture2D( positions, texCoord.st).rgb;
vec3 v0 = texture2D( velocities, texCoord.st).rgb;
float invmass = texture2D( positions, texCoord.st).a;

    float h = 1.0; //time step
    vec3 f = attractorPos-p0; //force
    float fMag = length(f); //force magnitude
    vec3 v1 = v0 + h * 0.05 * invmass * f/(fMag*fMag + EPS); //velocity update
    v1 = v1 - 0.02 * v1; //friction
    vec3 p1	= p0 + h * v1; //(symplectic euler) position update
    
    //Render to positions texture
    gl_FragData[0] = vec4(p1, invmass);
    //Render to velocities texture
    gl_FragData[1] = vec4(v1, length(v0)); //alpha component used for coloring
} 

