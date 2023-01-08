luaconsole_files = files(
	'LegacyLuaAPI.cpp',
	'LuaBit.cpp',
	'LuaButton.cpp',
	'LuaCheckbox.cpp',
	'LuaCompat.c',
	'LuaComponent.cpp',
	'LuaHttp.cpp',
	'LuaLabel.cpp',
	'LuaProgressBar.cpp',
	'LuaScriptInterface.cpp',
	'LuaSlider.cpp',
	'LuaSmartRef.cpp',
	'LuaTextbox.cpp',
	'LuaWindow.cpp',
	'LuaSocket.cpp',
)
if enable_http
	luaconsole_files += files('LuaSocketTCPHttp.cpp')
else
	luaconsole_files += files('LuaSocketTCPNoHttp.cpp')
endif

subdir('luascripts')

powder_files += luaconsole_files
