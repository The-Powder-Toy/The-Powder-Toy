client_files += files(
	'Common.cpp',
)

use_system_cert_provider = false
if not enable_http
	client_files += files('Null.cpp')
elif host_platform == 'emscripten'
	client_files += files('Emscripten.cpp')
else
	client_files += files('Libcurl.cpp')
	if host_platform == 'windows'
		use_system_cert_provider = true
		client_files += files('WindowsCertProvider.cpp')
		powder_deps += c_compiler.find_library('crypt32')
	endif
	if host_platform == 'android'
		use_system_cert_provider = true
		client_files += files('AndroidCertProvider.cpp')
	endif
endif
conf_data.set('NOHTTP', (not enable_http).to_string())
conf_data.set('USE_SYSTEM_CERT_PROVIDER', use_system_cert_provider.to_string())
