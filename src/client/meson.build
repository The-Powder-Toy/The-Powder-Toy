client_files = files(
	'SaveFile.cpp',
	'SaveInfo.cpp',
	'ThumbnailRendererTask.cpp',
	'Client.cpp',
	'GameSave.cpp',
	'User.cpp',
)
if host_platform == 'emscripten'
	client_files += files('AuthUserEmscripten.cpp')
else
	client_files += files('AuthUserCommon.cpp')
endif

subdir('http')

powder_files += client_files

render_files += files(
	'GameSave.cpp',
)
