luaconsole_files = files(
	'LegacyLuaAPI.cpp',
	'LuaButton.cpp',
	'LuaCheckbox.cpp',
	'LuaCompat.c',
	'LuaComponent.cpp',
	'LuaHttp.cpp',
	'LuaLabel.cpp',
	'LuaProgressBar.cpp',
	'LuaScriptInterface.cpp',
	'LuaSlider.cpp',
	'LuaSocket.cpp',
	'LuaSmartRef.cpp',
	'LuaTextbox.cpp',
	'LuaWindow.cpp',
)
if lua_variant != 'luajit'
	luaconsole_files += files(
		'LuaBit.cpp',
	)
endif
if host_platform == 'windows'
	luaconsole_files += files('LuaSocketWindows.cpp')
else
	luaconsole_files += files('LuaSocketDefault.cpp')
endif
if enable_http
	luaconsole_files += files('LuaSocketTCPHttp.cpp')
else
	luaconsole_files += files('LuaSocketTCPNoHttp.cpp')
endif
conf_data.set('LUACONSOLE', lua_variant != 'none' ? 'true' : 'false')

subdir('luascripts')

powder_files += luaconsole_files
