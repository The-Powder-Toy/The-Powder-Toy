uniform sampler2D fireAlpha;
void main () {
    vec4 texColor = texture2D(fireAlpha, gl_PointCoord);
    gl_FragColor = vec4(gl_Color.rgb, texColor.a*gl_Color.a);
}
