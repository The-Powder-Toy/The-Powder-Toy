#include "Platform.h"
#include "Android.h"
#include "common/Defer.h"
#include "Config.h"
#include <ctime>
#include <SDL.h>
#include <jni.h>
#include <android/log.h>

namespace Platform
{
void OpenURI(ByteString uri)
{
	fprintf(stderr, "cannot open URI: not implemented\n");
}

long unsigned int GetTime()
{
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
}

ByteString ExecutableNameFirstApprox()
{
	return "/proc/self/exe";
}

bool CanUpdate()
{
	return false;
}

void SetupCrt()
{
}

std::optional<ByteString> CallActivityStringFunc(const char *funcName)
{
	ByteString result;
	struct CheckFailed : public std::runtime_error
	{
		using runtime_error::runtime_error;
	};
	try
	{
		auto CHECK = [](auto thing, const char *what) {
			if (!thing)
			{
				throw CheckFailed(what);
			}
			return thing;
		};
#define CHECK(a) CHECK(a, #a)
		auto *env              = CHECK((JNIEnv *)SDL_AndroidGetJNIEnv());
		auto activityInst      = CHECK((jobject)SDL_AndroidGetActivity());
		auto activityCls       = CHECK(env->GetObjectClass(activityInst));
		auto getClassLoaderMth = CHECK(env->GetMethodID(activityCls, "getClassLoader", "()Ljava/lang/ClassLoader;"));
		auto classLoaderInst   = CHECK(env->CallObjectMethod(activityInst, getClassLoaderMth));
		auto classLoaderCls    = CHECK(env->FindClass("java/lang/ClassLoader"));
		auto findClassMth      = CHECK(env->GetMethodID(classLoaderCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"));
		auto strClassName      = CHECK(env->NewStringUTF(ByteString::Build(APPID, ".PowderActivity").c_str()));
		Defer deleteStrClassName([env, strClassName]() { env->DeleteLocalRef(strClassName); });
		auto mPowderActivity   = CHECK((jclass)(env->CallObjectMethod(classLoaderInst, findClassMth, strClassName)));
		auto funcMth           = CHECK(env->GetMethodID(mPowderActivity, funcName, "()Ljava/lang/String;"));
		auto resultRef         = CHECK((jstring)env->CallObjectMethod(activityInst, funcMth));
		Defer deleteStr([env, resultRef]() { env->DeleteLocalRef(resultRef); });
		auto *resultBytes      = CHECK(env->GetStringUTFChars(resultRef, 0));
		Defer deleteUtf([env, resultRef, resultBytes]() { env->ReleaseStringUTFChars(resultRef, resultBytes); });
		result = resultBytes;
	}
	catch (const CheckFailed &ex)
	{
		__android_log_print(ANDROID_LOG_ERROR, APPID, "CallActivityStringFunc/%s failed: %s", funcName, ex.what());
		return std::nullopt;
	}
#undef CHECK
	return result;
}

ByteString DefaultDdir()
{
	auto result = CallActivityStringFunc("getDefaultDdir");
	if (result)
	{
		__android_log_print(ANDROID_LOG_ERROR, APPID, "DefaultDdir succeeded, data dir is %s", result->c_str());
		return *result;
	}
	return "";
}
}
