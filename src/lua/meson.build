luaconsole_files = files(
	'LuaButton.cpp',
	'LuaBz2.cpp',
	'LuaCheckbox.cpp',
	'LuaCompat.c',
	'LuaComponent.cpp',
	'LuaElements.cpp',
	'LuaEvent.cpp',
	'LuaFileSystem.cpp',
	'LuaGraphics.cpp',
	'LuaHttp.cpp',
	'LuaInterface.cpp',
	'LuaLabel.cpp',
	'LuaMisc.cpp',
	'LuaPlatform.cpp',
	'LuaProgressBar.cpp',
	'LuaRenderer.cpp',
	'LuaScriptInterface.cpp',
	'LuaSimulation.cpp',
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
if not enable_http or host_platform == 'emscripten'
	luaconsole_files += files('LuaSocketTCPNoHttp.cpp')
else
	luaconsole_files += files('LuaSocketTCPHttp.cpp')
endif
conf_data.set('LUACONSOLE', (lua_variant != 'none').to_string())

subdir('luascripts')

powder_files += luaconsole_files
