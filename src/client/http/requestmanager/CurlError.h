#pragma once
#include <curl/curl.h> // Has to come first because windows(tm).
#include <stdexcept>

namespace http
{
	struct CurlError : public std::runtime_error
	{
		using runtime_error::runtime_error;
	};

	void UseSystemCertProvider(CURL *easy);
	void SetupCurlEasyCiphers(CURL *easy);
	void HandleCURLcode(CURLcode code);
	void HandleCURLMcode(CURLMcode code);
}
