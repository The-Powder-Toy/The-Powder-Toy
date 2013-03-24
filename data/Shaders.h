#ifndef SHADERS_H_
#define SHADERS_H_

const char * fireFragment = "#version 120\n\
uniform sampler2D fireAlpha;\
void main () {\
	vec4 texColor = texture2D(fireAlpha, gl_PointCoord);\
	gl_FragColor = vec4(gl_Color.rgb, texColor.a*gl_Color.a);\
}";
const char * fireVertex = "#version 120\n\
void main(void)\
{\
   gl_Position = ftransform();;\
   gl_FrontColor = gl_Color;\
}";
const char * lensFragment = "#version 120\n\
uniform sampler2D pTex;\
uniform sampler2D tfX;\
uniform sampler2D tfY;\
uniform float xres;\
uniform float yres;\
void main () {\
	vec4 transformX = texture2D(tfX, vec2(gl_TexCoord[0].s, -gl_TexCoord[0].t));\
	vec4 transformY = -texture2D(tfY, vec2(gl_TexCoord[0].s, -gl_TexCoord[0].t));\
	transformX.r /= xres;\
	transformY.g /= yres;\
	vec4 texColor = vec4(\
		texture2D(pTex, gl_TexCoord[0].st-vec2(transformX.r*0.75, transformY.g*0.75)).r,\
		texture2D(pTex, gl_TexCoord[0].st-vec2(transformX.r*0.875, transformY.g*0.875)).g,\
		texture2D(pTex, gl_TexCoord[0].st-vec2(transformX.r, transformY.g)).b,\
		1.0\
	);\
	gl_FragColor = texColor;\
}";
const char * lensVertex = "#version 120\n\
void main(void)\
{\
	gl_TexCoord[0]  = gl_MultiTexCoord0;\
	gl_Position = ftransform();;\
	gl_FrontColor = gl_Color;\
}";
const char * airVFragment = "#version 120\n\
uniform sampler2D airX;\
uniform sampler2D airY;\
uniform sampler2D airP;\
void main () {\
	vec4 texX = texture2D(airX, gl_TexCoord[0].st);\
	vec4 texY = texture2D(airY, gl_TexCoord[0].st);\
	vec4 texP = texture2D(airP, gl_TexCoord[0].st);\
	gl_FragColor = vec4(abs(texX.r)/2.0, texP.b/2.0, abs(texY.g)/2.0, 1.0);\
}";
const char * airVVertex = "#version 120\n\
void main(void)\
{\
	gl_TexCoord[0]  = gl_MultiTexCoord0;\
	gl_Position = ftransform();;\
	gl_FrontColor = gl_Color;\
}";
const char * airPFragment = "#version 120\n\
uniform sampler2D airX;\
uniform sampler2D airY;\
uniform sampler2D airP;\
void main () {\
	vec4 texP = texture2D(airP, gl_TexCoord[0].st);\
	gl_FragColor = vec4(max(texP.b/2.0, 0), 0, abs(min(texP.b/2.0, 0)), 1.0);\
}";
const char * airPVertex = "#version 120\n\
void main(void)\
{\
	gl_TexCoord[0]  = gl_MultiTexCoord0;\
	gl_Position = ftransform();;\
	gl_FrontColor = gl_Color;\
}";
const char * airCFragment = "#version 120\n\
uniform sampler2D airX;\
uniform sampler2D airY;\
uniform sampler2D airP;\
void main () {\
	vec4 texX = texture2D(airX, gl_TexCoord[0].st);\
	vec4 texY = texture2D(airY, gl_TexCoord[0].st);\
	vec4 texP = texture2D(airP, gl_TexCoord[0].st);\
	gl_FragColor = vec4(max(texP.b/2.0, 0), 0, abs(min(texP.b/2.0, 0)), 1.0) + vec4(abs(texX.r)/8.0, abs(texX.r)/8.0, abs(texX.r)/8.0, 1.0) + vec4(abs(texY.g)/8.0, abs(texY.g)/8.0, abs(texY.g)/8.0, 1.0);\
}";
const char * airCVertex = "#version 120\n\
void main(void)\
{\
	gl_TexCoord[0]  = gl_MultiTexCoord0;\
	gl_Position = ftransform();;\
	gl_FrontColor = gl_Color;\
}";

#endif /* SHADERS_H_ */
