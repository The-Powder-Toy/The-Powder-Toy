if host_platform == 'windows'
	if use_bluescreen
		project_deps += [
			c_compiler.find_library('dbghelp'),
			c_compiler.find_library('psapi'),
		]
	endif
	stacktrace_files = files('Windows.cpp')
elif host_platform == 'darwin'
	# stacktrace_files = files('Execinfo.cpp') # macos has this but it's useless >_>
	stacktrace_files = files('Null.cpp')
elif host_platform == 'linux'
	# TODO: again, this is more like "posix" than "linux"
	stacktrace_files = files('Execinfo.cpp')
else
	stacktrace_files = files('Null.cpp')
endif
