#ifdef USE_JNI
#include "OpenGLCanvasMacOS.h"

static jfieldID ctxID = NULL;

NSOpenGLPixelFormat* defaultPixelFormat()
{
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
        0
    };
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

NSOpenGLContext* ensureContext(NSOpenGLContext* openGLContext, NSView *view) {
	NSOpenGLContext* _openGLContext = openGLContext;
	if (!_openGLContext) {
		NSOpenGLPixelFormat* pixelFormat = defaultPixelFormat();
        _openGLContext = [[NSOpenGLContext alloc] 
						  initWithFormat:pixelFormat
						  shareContext:nil];
		[pixelFormat release];
	}
	if ([_openGLContext view] != view) {
		[_openGLContext setView:view];
	}
	[_openGLContext makeCurrentContext];
	
	return _openGLContext;
}

ContextInfo* getContext(JNIEnv *env, jobject canvas) 
{
	if (!ctxID) {
		jclass cls = env->GetObjectClass(canvas);
		ctxID = env->GetFieldID(cls, "openGLContext", "J");
	}	
	
	ContextInfo *ci = (ContextInfo *)(long)(env->GetLongField(canvas, ctxID));
	
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
	ContextInfo *ci = getContext(env, canvas);
	
    // Lock the drawing surface
    // You must lock EACH TIME before drawing
    jint lock = ci->ds->Lock(ci->ds); 
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
        ci->dsi_mac = (JAWT_MacOSXDrawingSurfaceInfo*)ci->dsi->platformInfo;
        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
        }
		
        // Get the corresponding peer from the caller canvas
        ci->view = ci->dsi_mac->cocoaViewRef;
		ci->openGLContext = ensureContext(ci->openGLContext, ci->view);
		
		return JNI_TRUE;
	}
	
	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_endOpenGL(JNIEnv *env, jobject canvas)
{
	ContextInfo *ci = getContext(env, canvas);
	
	[ci->openGLContext flushBuffer];
	
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
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_updateOpenGL(JNIEnv *env, jobject canvas)
{
	ContextInfo *ci = getContext(env, canvas);
	
	[ci->openGLContext update];
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
	
    // Get the drawing surface.  This can be safely cached.
    // Anything below the DS (DSI, contexts, etc) 
    // can possibly change/go away and should not be cached.
    ci->ds = ci->awt->GetDrawingSurface(env, canvas);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
    assert(ci->ds != NULL);
	
	NSLog(@"Alloc Context %d", ci);
}

JNIEXPORT void JNICALL Java_OpenGLCanvas_releaseOpenGL(JNIEnv *env, jobject canvas) 
{
	ContextInfo *ci = getContext(env, canvas);
	NSLog(@"Release Context %d", ci);
	if (ci->openGLContext) {
        if ([ci->openGLContext view] /* == self */) {
            [ci->openGLContext clearDrawable];
        }
        [ci->openGLContext release];
	}
	
    // Free the drawing surface (if not caching it)
    ci->awt->FreeDrawingSurface(ci->ds);
    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }
	
	freeContext(env, canvas, ci);
}
#endif
