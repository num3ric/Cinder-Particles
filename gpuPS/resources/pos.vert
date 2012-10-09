//#extension GL_ARB_draw_buffers : enable
uniform sampler2D posArray;
uniform sampler2D velArray;
varying vec4 texCoord;

void main()
{
	texCoord = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}