#if defined(USE_JNI) && defined(WIN)
#include "OpenGLCanvasWin32.h"

static jfieldID ctxID = NULL;

int defaultPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
{
    ::ZeroMemory( pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
    pfd->nSize = sizeof( PIXELFORMATDESCRIPTOR );
    pfd->nVersion = 1;
    pfd->dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd->iPixelType = PFD_TYPE_RGBA;
    pfd->cColorBits = 24;
    pfd->cDepthBits = 16;
    pfd->iLayerType = PFD_MAIN_PLANE;
    return 0;
}

HGLRC ensureContext(JAWT_Win32DrawingSurfaceInfo* dsi_win, HGLRC hRC) {

	if (!hRC) {
		int iFormat;
		PIXELFORMATDESCRIPTOR pfd;
		defaultPixelFormat(&pfd);

	    iFormat = ChoosePixelFormat( dsi_win->hdc, &pfd );
	    SetPixelFormat( dsi_win->hdc, iFormat, &pfd );

		hRC = wglCreateContext( dsi_win->hdc );
	}
	if (1 && wglGetCurrentDC() != dsi_win->hdc) {
		wglMakeCurrent( dsi_win->hdc, hRC );
	}
	
	return hRC;
}

ContextInfo* getContext(JNIEnv *env, jobject canvas) 
{
	ContextInfo *ci;
	if (!ctxID) {
		jclass cls = env->GetObjectClass(canvas);
		ctxID = env->GetFieldID(cls, "openGLContext", "J");
	}	

	ci = (ContextInfo *)(long)(env->GetLongField(canvas, ctxID));

	if (!ci) {
		ci = (ContextInfo *)calloc(sizeof(ContextInfo), 1);
		ci->awt = (JAWT *)calloc(sizeof(JAWT), 1);
		env->SetLongField(canvas, ctxID, (jlong)(long)ci);
	}
	
	return ci;
}

void freeContext(JNIEnv *env, jobject canvas, ContextInfo* ci) 
{
	if (ci) {
		free(ci->awt);
		free(ci);
		env->SetLongField(canvas, ctxID, 0L);
	}
}

JNIEXPORT jboolean JNICALL Java_OpenGLCanvas_beginOpenGL(JNIEnv *env, jobject canvas)
{
	jint lock;
	ContextInfo *ci = getContext(env, canvas);
	
    // Get the drawing surface.  This can be safely cached -- not in win32
    // Anything below the DS (DSI, contexts, etc)
    // can possibly change/go away and should not be cached.
    ci->ds = ci->awt->GetDrawingSurface(env, canvas);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
    assert(ci->ds != NULL);

	// Lock the drawing surface
    // You must lock EACH TIME before drawing
    lock = ci->ds->Lock(ci->ds);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
    assert((lock & JAWT_LOCK_ERROR) == 0);
    
    // Get the drawing surface info
    ci->dsi = ci->ds->GetDrawingSurfaceInfo(ci->ds);
	
	// Check DrawingSurfaceInfo.  This can be NULL on Mac OS X
    // if the windowing system is not ready
	if (ci->dsi != NULL) {
        // Get the platform-specific drawing info
        // We will use this to get at Cocoa and CoreGraphics
        // See <JavaVM/jawt_md.h>
        ci->dsi_win = (JAWT_Win32DrawingSurfaceInfo*)ci->dsi->platformInfo;
        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
        }

        // Get the corresponding peer from the caller canvas
		ci->hRC = ensureContext(ci->dsi_win, ci->hRC);

		return JNI_TRUE;
	}
	
	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_endOpenGL(JNIEnv *env, jobject canvas)
{
	ContextInfo *ci = getContext(env, canvas);
	
	SwapBuffers( ci->dsi_win->hdc );

	// Free the DrawingSurfaceInfo
	ci->ds->FreeDrawingSurfaceInfo(ci->dsi);
	if (env->ExceptionOccurred()){
		env->ExceptionDescribe();
	}
	
    // Unlock the drawing surface
    // You must unlock EACH TIME when done drawing
    ci->ds->Unlock(ci->ds); 
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }

    // Free the drawing surface (if not caching it)
    ci->awt->FreeDrawingSurface(ci->ds);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_updateOpenGL(JNIEnv *env, jobject canvas)
{
	ContextInfo *ci = getContext(env, canvas);

	wglMakeCurrent( ci->dsi_win->hdc, ci->hRC );
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_allocOpenGL(JNIEnv *env, jobject canvas)
{
	ContextInfo *ci = getContext(env, canvas);
	
	jboolean result = JNI_FALSE;
    
    // get the AWT
    ci->awt->version = JAWT_VERSION_1_4;
    result = JAWT_GetAWT(env, ci->awt);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
    assert(result != JNI_FALSE);	
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_releaseOpenGL(JNIEnv *env, jobject canvas) 
{
	ContextInfo *ci = getContext(env, canvas);
	if (ci->hRC) {
		wglDeleteContext(ci->hRC);
	}

	freeContext(env, canvas, ci);
}
#endif
