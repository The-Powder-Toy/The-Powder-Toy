#ifdef MACOSX

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
//#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#define GL_RGBA32F 0x8814
#endif

#elif defined(WIN)

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#else

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
