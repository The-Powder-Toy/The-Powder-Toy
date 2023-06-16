client_files += files(
	'Common.cpp',
)

if not enable_http
	client_files += files('Null.cpp')
elif host_platform == 'emscripten'
	client_files += files('Emscripten.cpp')
	project_link_args += [
		'-s', 'EXPORTED_FUNCTIONS=_main,_RequestManager_UpdateRequestStatusThunk',
		'-s', 'EXPORTED_RUNTIME_METHODS=cwrap',
	]
else
	client_files += files('Libcurl.cpp')
endif
conf_data.set('NOHTTP', not enable_http ? 'true' : 'false')
