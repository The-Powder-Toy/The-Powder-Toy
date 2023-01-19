common_files += files(
	'Platform.cpp',
	'String.cpp',
	'tpt-rand.cpp',
	'tpt-thread-local.cpp',
)
if host_platform == 'windows'
	common_files += files('PlatformWindows.cpp')
elif host_platform == 'darwin'
	common_files += files('PlatformDarwin.cpp')
	common_files += files('PlatformPosix.cpp')
elif host_platform == 'android'
	common_files += files('PlatformAndroid.cpp')
	common_files += files('PlatformPosix.cpp')
elif host_platform == 'linux'
	# TODO: again, this is more like "posix" than "linux"
	common_files += files('PlatformLinux.cpp')
	common_files += files('PlatformPosix.cpp')
else
	common_files += files('PlatformNull.cpp')
	common_files += files('PlatformPosix.cpp')
endif
