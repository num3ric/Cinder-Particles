varying vec4 color;

void main(void)
{
    gl_FragColor = vec4(2.0*abs(color.xyz)+color.w, 1.0);
}