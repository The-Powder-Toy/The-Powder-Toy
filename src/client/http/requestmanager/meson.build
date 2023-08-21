client_files += files(
	'Common.cpp',
)

if not enable_http
	client_files += files('Null.cpp')
elif host_platform == 'emscripten'
	client_files += files('Emscripten.cpp')
else
	client_files += files('Libcurl.cpp')
endif
conf_data.set('NOHTTP', (not enable_http).to_string())
