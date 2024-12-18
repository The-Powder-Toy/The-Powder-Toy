common_files += files(
	'Common.cpp',
)

if get_option('use_bluescreen') == 'auto'
	use_bluescreen = not is_debug
else
	use_bluescreen = get_option('use_bluescreen') == 'yes'
endif
can_install_enforce_no = false
set_window_icon = false
path_sep_char = '/'
if host_platform == 'windows'
	path_sep_char = '\\\\'
	common_files += files(
		'Windows.cpp',
		'ExitCommon.cpp',
	)
	powder_files += files(
		'MainCommon.cpp',
		'DdirCommon.cpp',
	)
elif host_platform == 'darwin'
	can_install_enforce_no = true
	common_files += files(
		'Darwin.cpp',
		'Posix.cpp',
		'PosixProc.cpp',
		'ExitCommon.cpp',
	)
	powder_files += files(
		'MainCommon.cpp',
		'DdirCommon.cpp',
	)
elif host_platform == 'android'
	can_install_enforce_no = true
	common_files += files(
		'Android.cpp',
		'Posix.cpp',
		'PosixProc.cpp',
		'ExitCommon.cpp',
	)
	powder_files += files(
		'MainCommon.cpp',
	)
elif host_platform == 'emscripten'
	use_bluescreen = false
	can_install_enforce_no = true
	common_files += files(
		'Emscripten.cpp',
		'Posix.cpp',
	)
elif host_platform == 'linux'
	# TODO: again, this is more like "posix" than "linux"
	set_window_icon = true
	common_files += files(
		'Linux.cpp',
		'Posix.cpp',
		'PosixProc.cpp',
		'ExitCommon.cpp',
	)
	powder_files += files(
		'MainCommon.cpp',
		'DdirCommon.cpp',
	)
else
	can_install_enforce_no = true
	common_files += files(
		'Null.cpp',
		'Posix.cpp',
		'PosixProc.cpp',
		'ExitCommon.cpp',
	)
	powder_files += files(
		'MainCommon.cpp',
		'DdirCommon.cpp',
	)
endif

bluescreen_export_symbols = false

subdir('stacktrace')

if use_bluescreen
	common_files += stacktrace_files
	if bluescreen_export_symbols and is_debug
		project_export_dynamic = true
	endif
else
	common_files += files('stacktrace/Null.cpp')
endif
conf_data.set('SET_WINDOW_ICON', set_window_icon.to_string())
conf_data.set('PATH_SEP_CHAR', path_sep_char)
conf_data.set('USE_BLUESCREEN', use_bluescreen.to_string())

can_install = get_option('can_install')
if can_install == 'auto'
	can_install = 'yes_check'
	if is_debug
		can_install = 'yes'
	endif
	if can_install_enforce_no
		can_install = 'no'
	endif
endif
if can_install != 'no' and can_install_enforce_no
	error('cannot provide install support, configure with -Dcan_install=no to fix this error')
endif
conf_data.set('CAN_INSTALL', (can_install != 'no').to_string())
conf_data.set('SHARED_DATA_FOLDER', get_option('shared_data_folder').to_string())
conf_data.set('INSTALL_CHECK', (can_install == 'yes_check').to_string())
