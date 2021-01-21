luaconsole_files += files(
	'auxiliar.c',
	'buffer.c',
	'compat.c',
	'except.c',
	'inet.c',
	'io.c',
	'luasocket.c',
	'options.c',
	'select.c',
	'tcp.c',
	'timeout.c',
	'udp.c',
)
luaconsole_files += to_array.process('socket.lua', extra_args: 'socket_lua')

if copt_platform == 'win'
	luaconsole_files += files(
		'wsocket.c',
	)
else
	luaconsole_files += files(
		'usocket.c',
	)
endif
