uniform sampler2D displacementMap;
uniform sampler2D velocityMap;

varying vec4 color;
void main()
{
    //using the displacement map to move vertices
	vec4 pos = texture2D( displacementMap, gl_MultiTexCoord0.xy );
    color = texture2D( velocityMap, gl_MultiTexCoord0.xy );
	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
}