#ifdef USE_JNI
#import <jawt_md.h>

#import <Cocoa/Cocoa.h>
#import <AppKit/NSOpenGL.h>

NSOpenGLPixelFormat* defaultPixelFormat();

NSOpenGLContext* ensureContext(NSOpenGLContext* openGLContext, NSView *view);

typedef struct {
    JAWT* awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_MacOSXDrawingSurfaceInfo* dsi_mac;
	NSView *view;
	NSOpenGLContext* openGLContext;
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
