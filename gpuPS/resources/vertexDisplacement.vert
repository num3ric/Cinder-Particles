uniform sampler2D displacementMap;
void main()
{
	float scale = 1.0;
    //using the displacement map to move vertices
	vec4 pos = scale * texture2D( displacementMap, gl_MultiTexCoord0.xy );
	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
}