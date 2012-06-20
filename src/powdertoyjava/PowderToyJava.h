#include <jni.h>

#ifndef POWDERTOYJAVA
#define POWDERTOYJAVA

#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT void JNICALL Java_PowderToy_initialise(JNIEnv *, jobject);
	JNIEXPORT void JNICALL Java_PowderToy_tick(JNIEnv *, jobject);
	JNIEXPORT void JNICALL Java_PowderToy_draw(JNIEnv *, jobject);
	JNIEXPORT void JNICALL Java_PowderToy_finish(JNIEnv *, jobject);
	JNIEXPORT jint JNICALL Java_PowderToy_getWidth(JNIEnv * env, jobject canvas);
	JNIEXPORT jint JNICALL Java_PowderToy_getHeight(JNIEnv * env, jobject canvas);
	JNIEXPORT void JNICALL Java_PowderToy_mousePressed(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY, jint mouseButton);
	JNIEXPORT void JNICALL Java_PowderToy_mouseReleased(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY, jint mouseButton);
	JNIEXPORT void JNICALL Java_PowderToy_mouseMoved(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY);
#ifdef __cplusplus
}
#endif
#endif
