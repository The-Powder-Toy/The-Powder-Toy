#include "CurlError.h"
#include "common/String.h"
#include <wincrypt.h> // crypt32.lib is pulled in by tpt-libs
#include <iostream>
#include <memory>

namespace http
{
	// see https://stackoverflow.com/questions/9507184/can-openssl-on-windows-use-the-system-certificate-store
	void UseSystemCertProvider(CURL *easy)
	{
		struct DoOnce
		{
			ByteString allPems;

			void InitPem()
			{
				struct StoreDeleter
				{
					typedef HCERTSTORE pointer;

					void operator ()(HCERTSTORE p) const
					{
						::CertCloseStore(p, 0);
					}
				};
				using StorePtr = std::unique_ptr<void, StoreDeleter>;
				struct ContextDeleter
				{
					typedef PCCERT_CONTEXT pointer;

					void operator ()(PCCERT_CONTEXT p) const
					{
						::CertFreeCertificateContext(p);
					}
				};
				using ContextPtr = std::unique_ptr<void, ContextDeleter>;

				auto die = [](ByteString message) {
					std::cerr << "failed to enumerate system certificates: " << message << ": " << GetLastError() << std::endl;
				};
				auto store = StorePtr(::CertOpenSystemStore(0, L"ROOT"), StoreDeleter{});
				if (!store)
				{
					return die("CertOpenSystemStore failed");
				}
				ContextPtr context;
				while (true)
				{
					context = ContextPtr(::CertEnumCertificatesInStore(store.get(), context.release()), ContextDeleter{});
					if (!context)
					{
						if (::GetLastError() != DWORD(CRYPT_E_NOT_FOUND))
						{
							return die("CertEnumCertificatesInStore failed");
						}
						break;
					}
					DWORD pemLength;
					// get required buffer size first
					if (!CryptBinaryToStringA(context->pbCertEncoded, context->cbCertEncoded, CRYPT_STRING_BASE64HEADER, NULL, &pemLength))
					{
						return die("CryptBinaryToStringA failed");
					}
					std::vector<char> pem(pemLength);
					// actually get the data
					if (!CryptBinaryToStringA(context->pbCertEncoded, context->cbCertEncoded, CRYPT_STRING_BASE64HEADER, pem.data(), &pemLength))
					{
						return die("CryptBinaryToStringA failed");
					}
					allPems += ByteString(pem.data(), pem.data() + pem.size() - 1); // buffer includes the zero terminator, omit that
				}
				if (!allPems.size())
				{
					die("no system certificates");
				}
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
