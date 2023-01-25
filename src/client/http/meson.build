client_files += files(
	'APIRequest.cpp',
	'GetUserInfoRequest.cpp',
	'ImageRequest.cpp',
	'Request.cpp',
	'SaveUserInfoRequest.cpp',
	'ThumbnailRequest.cpp',
	'RequestManager.cpp',
)

if enable_http
	client_files += files('RequestManagerHttp.cpp')
else
	client_files += files('RequestManagerNoHttp.cpp')
endif
conf_data.set('NOHTTP', not enable_http ? 'true' : 'false')
