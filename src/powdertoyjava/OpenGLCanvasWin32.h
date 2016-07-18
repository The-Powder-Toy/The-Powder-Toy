#ifdef USE_JNI
#import <jawt_md.h>

#include <windows.h>
#include <cassert>
#include <gl/gl.h>

int defaultPixelFormat(PIXELFORMATDESCRIPTOR* pfd);

HGLRC ensureContext(JAWT_Win32DrawingSurfaceInfo* dsi_win, HGLRC hRC);

typedef struct {
    JAWT* awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_Win32DrawingSurfaceInfo* dsi_win;
	HGLRC hRC;
} ContextInfo;

ContextInfo* getContext(JNIEnv *env, jobject canvas);

void freeContext(JNIEnv *env, jobject canvas, ContextInfo* ci);

#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT jboolean JNICALL Java_OpenGLCanvas_beginOpenGL(JNIEnv *env, jobject canvas);
	JNIEXPORT void JNICALL Java_OpenGLCanvas_endOpenGL(JNIEnv *env, jobject canvas);
	JNIEXPORT void JNICALL Java_OpenGLCanvas_updateOpenGL(JNIEnv *env, jobject canvas);
	JNIEXPORT void JNICALL Java_OpenGLCanvas_allocOpenGL(JNIEnv *env, jobject canvas);
	JNIEXPORT void JNICALL Java_OpenGLCanvas_releaseOpenGL(JNIEnv *env, jobject canvas);
#ifdef __cplusplus
}
#endif
#endif
