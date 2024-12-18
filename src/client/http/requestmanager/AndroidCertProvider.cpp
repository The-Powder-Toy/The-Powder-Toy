#include "CurlError.h"
#include "common/platform/Android.h"
#include "Config.h"
#include <iostream>
#include <android/log.h>

namespace http
{
	void UseSystemCertProvider(CURL *easy)
	{
		struct DoOnce
		{
			ByteString certificateBundle;

			DoOnce()
			{
				auto certificateBundleOpt = Platform::CallActivityStringFunc("getCertificateBundle");
				if (certificateBundleOpt)
				{
					certificateBundle = *certificateBundleOpt;
					__android_log_print(ANDROID_LOG_ERROR, APPID, "certificate bundle loaded");
				}
			}
		};

		static DoOnce doOnce;
		if (doOnce.certificateBundle.size())
		{
			curl_blob blob;
			blob.data = doOnce.certificateBundle.data();
			blob.len = doOnce.certificateBundle.size();
			blob.flags = CURL_BLOB_COPY;
			HandleCURLcode(curl_easy_setopt(easy, CURLOPT_CAINFO_BLOB, &blob));
		}
	}
}
