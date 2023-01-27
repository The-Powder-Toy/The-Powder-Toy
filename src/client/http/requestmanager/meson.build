client_files += files(
	'Common.cpp',
)

if enable_http
	client_files += files('Libcurl.cpp')
else
	client_files += files('Null.cpp')
endif
conf_data.set('NOHTTP', not enable_http ? 'true' : 'false')
