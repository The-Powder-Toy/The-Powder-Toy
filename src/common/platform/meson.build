common_files += files(
	'Common.cpp',
)

can_install_enforce_no = false
set_window_icon = false
path_sep_char = '/'
if host_platform == 'windows'
	path_sep_char = '\\\\'
	common_files += files(
		'Windows.cpp',
	)
elif host_platform == 'darwin'
	can_install_enforce_no = true
	common_files += files(
		'Darwin.cpp',
		'Posix.cpp',
	)
elif host_platform == 'android'
	can_install_enforce_no = true
	common_files += files(
		'Android.cpp',
		'Posix.cpp',
	)
elif host_platform == 'linux'
	# TODO: again, this is more like "posix" than "linux"
	set_window_icon = true
	common_files += files(
		'Linux.cpp',
		'Posix.cpp',
	)
else
	can_install_enforce_no = true
	common_files += files(
		'Null.cpp',
		'Posix.cpp',
	)
endif
conf_data.set('SET_WINDOW_ICON', set_window_icon ? 'true' : 'false')
conf_data.set('PATH_SEP_CHAR', path_sep_char)

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
conf_data.set('CAN_INSTALL', can_install != 'no' ? 'true' : 'false')
conf_data.set('INSTALL_CHECK', can_install == 'yes_check' ? 'true' : 'false')
