uniform sampler2D fireAlpha; // the texture with the scene you want to blur
//varying vec2 vTexCoord;
 
//precision mediump float;
//uniform sampler2D tex;
//varying float vAlpha;

in vec2 gl_PointCoord;

void main () {
    vec4 texColor = texture2D(fireAlpha, gl_PointCoord);
    gl_FragColor = vec4(gl_Color.rgb, texColor.a*gl_Color.a);
}
