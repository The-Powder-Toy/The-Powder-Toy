#include "CurlError.h"
#include "common/String.h"
#include "Config.h"
#include <iostream>
#include <SDL.h>
#include <jni.h>
#include <android/log.h>

static jclass FindClass(JNIEnv *env, const char *name)
{
	jobject   nativeActivity = (jobject)SDL_AndroidGetActivity();                                                   if (!nativeActivity) return NULL;
	jclass    acl            = env->GetObjectClass(nativeActivity);                                                 if (!acl           ) return NULL;
	jmethodID getClassLoader = env->GetMethodID(acl, "getClassLoader", "()Ljava/lang/ClassLoader;");                if (!getClassLoader) return NULL;
	jobject   cls            = env->CallObjectMethod(nativeActivity, getClassLoader);                               if (!cls           ) return NULL;
	jclass    classLoader    = env->FindClass("java/lang/ClassLoader");                                             if (!classLoader   ) return NULL;
	jmethodID findClass      = env->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"); if (!findClass     ) return NULL;
	jstring   strClassName   = env->NewStringUTF(name);                                                             if (!strClassName  ) return NULL;
	jclass clazz = (jclass)(env->CallObjectMethod(cls, findClass, strClassName));
	env->DeleteLocalRef(strClassName);
	return clazz;
}

namespace http
{
	void UseSystemCertProvider(CURL *easy)
	{
		struct DoOnce
		{
			ByteString allPems;

			void InitPem()
			{
				auto die = [](ByteString message) {
					__android_log_print(ANDROID_LOG_ERROR, "AndroidCertProvider", "%s", ("failed to enumerate system certificates: " + message).c_str());
				};
				auto *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
				if (!env)
				{
					return die("SDL_AndroidGetJNIEnv failed");
				}
				jclass mPowderActivity = FindClass(env, ByteString::Build(APPID, ".PowderActivity").c_str());
				if (!mPowderActivity)
				{
					return die("FindClass failed");
				}
				jmethodID midGetCertificateBundle = env->GetStaticMethodID(mPowderActivity, "getCertificateBundle", "()Ljava/lang/String;");
				if (!midGetCertificateBundle)
				{
					return die("GetStaticMethodID failed");
				}
				jstring str = (jstring)env->CallStaticObjectMethod(mPowderActivity, midGetCertificateBundle);
				if (!str)
				{
					return die("getCertificateBundle failed");
				}
				const char *utf = env->GetStringUTFChars(str, 0);
				if (utf)
				{
					allPems = utf;
					env->ReleaseStringUTFChars(str, utf);
				}
				else
				{
					__android_log_print(ANDROID_LOG_ERROR, "AndroidCertProvider", "out of memory???");
				}
				env->DeleteLocalRef(str);
				__android_log_print(ANDROID_LOG_ERROR, "AndroidCertProvider", "certificate bundle loaded");
			}

			DoOnce()
			{
				InitPem();
			}
		};

		static DoOnce doOnce;
		if (doOnce.allPems.size())
		{
			curl_blob blob;
			blob.data = doOnce.allPems.data();
			blob.len = doOnce.allPems.size();
			blob.flags = CURL_BLOB_COPY;
			HandleCURLcode(curl_easy_setopt(easy, CURLOPT_CAINFO_BLOB, &blob));
		}
	}
}
