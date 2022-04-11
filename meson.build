project('the-powder-toy', [ 'c', 'cpp' ], version: 'the.cake.is.a.lie', default_options: [
	'cpp_std=c++17',
	'build.cpp_std=c++17', # used when cross-compiling
])

to_array = generator(
	executable('toarray', sources: 'data/ToArray.cpp', native: true),
	output: [ '@PLAINNAME@.cpp', '@PLAINNAME@.h' ],
	arguments: [ '@OUTPUT0@', '@OUTPUT1@', '@INPUT@', '@EXTRA_ARGS@' ]
)

c_compiler = meson.get_compiler('c')
copt_x86 = false
copt_64bit = false
copt_msvc = false
if c_compiler.get_id() in [ 'msvc' ]
	copt_compiler = 'msvc'
	copt_msvc = true
elif c_compiler.get_id() in [ 'gcc', 'clang' ]
	copt_compiler = 'gcc'
else
	warning('unsupported compiler: ' + c_compiler.get_id() + '; you are on your own')
	copt_compiler = 'gcc'
endif
if host_machine.cpu_family() in [ 'x86_64' ]
	copt_architecture = 'x86_64'
	copt_x86 = true
	copt_64bit = true
elif host_machine.cpu_family() in [ 'x86' ]
	copt_architecture = 'i686'
	copt_x86 = true
elif host_machine.cpu_family() in [ 'aarch64' ]
	copt_architecture = 'arm64'
	copt_64bit = true
elif host_machine.cpu_family() in [ 'arm' ]
	copt_architecture = 'arm'
else
	warning('unsupported architecture: ' + host_machine.cpu_family() + '; you are on your own')
	copt_architecture = 'i686'
endif
if host_machine.system() in [ 'linux', 'freebsd' ]
	copt_platform = 'lin'
elif host_machine.system() in [ 'windows' ]
	copt_platform = 'win'
elif host_machine.system() in [ 'darwin' ]
	copt_platform = 'mac'
elif host_machine.system() in [ 'android' ]
	copt_platform = 'and'
else
	warning('unsupported platform: ' + host_machine.system() + '; you are on your own')
	copt_platform = 'lin'
endif
uopt_static = get_option('static')
use_tpt_libs = 'no'
if uopt_static == 'system'
	if copt_platform == 'win'
		error('no way to find static system libraries on windows')
	endif
elif uopt_static == 'prebuilt'
	use_tpt_libs = 'static'
elif copt_platform == 'win'
	use_tpt_libs = 'dynamic'
elif copt_platform == 'and'
	use_tpt_libs = 'static'
endif

conf_data = configuration_data()
conf_data.set('CURL_STATICLIB', false)
conf_data.set('ZLIB_WINAPI', false)

if get_option('ogli') or get_option('oglr')
	error('OpenGL features are currently unavailable')
endif

tpt_libs_vtag = get_option('tpt_libs_vtag')
if tpt_libs_vtag == ''
	tpt_libs_vtag = 'v20220414193335'
endif
if use_tpt_libs != 'no'
	nope = false
	if copt_architecture == 'i686' and not copt_msvc
		nope = true
	endif
	if copt_architecture == 'x86_64' and (copt_platform == 'lin' or copt_platform == 'mac') and copt_compiler != 'gcc'
		nope = true
	endif
	if copt_architecture == 'arm64' and copt_platform != 'mac'
		nope = true
	endif
	if copt_platform == 'and'
		nope = false
	endif
	quad_compiler = copt_compiler
	if copt_platform == 'win' and copt_compiler == 'gcc'
		quad_compiler = 'mingw'
		if get_option('b_lto')
			error('mingw does not like static + lto, you will have to disable this error if you want to proceed')
		endif
	endif
	if nope
		error('no prebuilt @0@-@1@-@2@-@3@ libraries are currently provided'.format(copt_architecture, copt_platform, quad_compiler, use_tpt_libs))
	endif
	tpt_libs = subproject('tpt-libs-prebuilt-@0@-@1@-@2@-@3@-@4@'.format(copt_architecture, copt_platform, quad_compiler, use_tpt_libs, tpt_libs_vtag))
endif

uopt_native = get_option('native')
uopt_x86_sse = get_option('x86_sse')
if uopt_x86_sse == 'auto'
	uopt_x86_sse_level = 20
elif uopt_x86_sse == 'sse3'
	uopt_x86_sse_level = 30
elif uopt_x86_sse == 'sse2'
	uopt_x86_sse_level = 20
elif uopt_x86_sse == 'sse'
	uopt_x86_sse_level = 10
elif uopt_x86_sse == 'none'
	uopt_x86_sse_level = 0
endif
if not copt_x86 or uopt_native
	uopt_x86_sse_level = 0
endif

uopt_lua = get_option('lua')
if copt_platform == 'and' and uopt_lua == 'luajit'
	message('LuaJIT is not supported on Android')
	uopt_lua = 'lua5.2'
endif
if uopt_lua == 'luajit'
	lua_opt_dep = [ use_tpt_libs != 'no' ? tpt_libs.get_variable('luajit_dep') : dependency('luajit', static: uopt_static == 'system') ]
elif uopt_lua == 'lua5.2'
	lua_opt_dep = [ use_tpt_libs != 'no' ? tpt_libs.get_variable('lua52_dep') : dependency('lua5.2', static: uopt_static == 'system') ]
elif uopt_lua == 'lua5.1'
	lua_opt_dep = [ use_tpt_libs != 'no' ? tpt_libs.get_variable('lua51_dep') : dependency('lua5.1', static: uopt_static == 'system') ]
else
	lua_opt_dep = []
endif

uopt_http = get_option('http')
if uopt_http and copt_platform != 'and'
	curl_opt_dep = [ use_tpt_libs != 'no' ? tpt_libs.get_variable('libcurl_dep') : dependency('libcurl', static: uopt_static == 'system') ]
else
	curl_opt_dep = []
endif

uopt_fftw = get_option('gravfft')
if uopt_fftw
	fftw_opt_dep = [ use_tpt_libs != 'no' ? tpt_libs.get_variable('fftw_dep') : dependency('fftw3f', static: uopt_static == 'system') ]
else
	fftw_opt_dep = []
endif

threads_dep = dependency('threads')
zlib_dep = use_tpt_libs != 'no' ? tpt_libs.get_variable('zlib_dep') : dependency('zlib', static: uopt_static == 'system')
sdl2_dep = use_tpt_libs != 'no' ? tpt_libs.get_variable('sdl2_dep') : dependency('sdl2', static: uopt_static == 'system')
sdl2main_dep = use_tpt_libs != 'no' ? tpt_libs.get_variable('sdl2main_dep') : []

project_link_args = []
project_c_args = []
project_cpp_args = []
if copt_msvc
	if uopt_x86_sse_level >= 30
		message('SSE3 configured to be enabled but unavailable in msvc')
		uopt_x86_sse_level = 20
	endif
	if uopt_native
		message('local machine optimization configured to be enabled but unavailable in msvc')
		uopt_native = false
	endif
	args_msvc = [
		'/GS',
		'-D_SCL_SECURE_NO_WARNINGS',
		'-DUNICODE',
		'-D_UNICODE',
	]
	if copt_64bit
		message('SSE explicitly configured but unavailable in msvc targeting 64-bit machines')
	else
		if uopt_x86_sse_level >= 20
			args_msvc += [ '/arch:SSE2' ]
		elif uopt_x86_sse_level >= 10
			args_msvc += [ '/arch:SSE' ]
		endif
	endif
	if get_option('debug')
		project_link_args += [
			'/NODEFAULTLIB:MSVCRT',
		]
	else
		args_msvc += [
			'/Oy-',
			'/fp:fast',
		]
		project_link_args += [
			'/OPT:REF',
			'/OPT:ICF',
		]
	endif
	project_c_args += args_msvc
	project_cpp_args += args_msvc
else
	args_ccomp = [ '-Wno-unused-result' ]
	if copt_platform == 'mac'
		if uopt_x86_sse_level >= 0
			message('SSE level explicitly configured but unavailable on macosx')
			uopt_x86_sse_level = 0
		endif
		if uopt_native
			message('local machine optimization configured to be enabled but unavailable on macosx')
			uopt_native = false
		endif
	else
		if uopt_x86_sse_level >= 30
			args_ccomp += [ '-msse3' ]
		endif
		if uopt_x86_sse_level >= 20
			args_ccomp += [ '-msse2' ]
		endif
		if uopt_x86_sse_level >= 10
			args_ccomp += [ '-msse' ]
		endif
		if uopt_native
			args_ccomp += [ '-march=native' ]
		endif
	endif
	if copt_platform == 'win'
		args_ccomp += [
			'-DUNICODE',
			'-D_UNICODE',
		]
		if uopt_static != 'none' and copt_platform != 'and'
			project_link_args += [ '-static', '-static-libgcc', '-static-libstdc++' ]
		endif
	endif
	if not get_option('debug')
		args_ccomp += [
			'-ftree-vectorize',
			'-funsafe-math-optimizations',
			'-ffast-math',
			'-fomit-frame-pointer',
		]
	endif
	if copt_platform == 'mac' and uopt_static == 'prebuilt'
		args_ccomp += [ '-mmacosx-version-min=10.9' ]
		project_link_args += [ '-mmacosx-version-min=10.9' ]
	endif
	if copt_platform == 'and'
		if not copt_64bit
			args_ccomp += [ '-U_FILE_OFFSET_BITS' ]
		endif
		project_link_args += [ '-static-libstdc++' ]
	else
		if not get_option('b_pie') and get_option('workaround_gcc_no_pie') # nice one, meson
			if copt_architecture != 'arm64' # no position independent executable for arm64
				if c_compiler.get_id() in [ 'clang' ]
					project_link_args += [ '-Wl,-no_pie' ]
				else
					project_link_args += [ '-no-pie' ]
				endif
			endif
		endif
	endif
	project_c_args += args_ccomp
	project_cpp_args += args_ccomp + [ '-Wno-invalid-offsetof' ]
endif

if copt_platform == 'win'
	other_dep = tpt_libs.get_variable('other_dep')
	args_ccomp_win = [ '-D_WIN32_WINNT=0x0501' ]
	project_c_args   += args_ccomp_win
	project_cpp_args += args_ccomp_win
	windows_mod = import('windows')
	if uopt_static != 'none'
		conf_data.set('CURL_STATICLIB', true)
		if copt_architecture == 'x86_64'
			conf_data.set('ZLIB_WINAPI', true)
		endif
	else
		foreach input_and_output : tpt_libs.get_variable('config_dlls')
			configure_file(input: input_and_output[0], output: input_and_output[1], copy: true)
		endforeach
	endif
elif copt_platform == 'lin'
	if uopt_static != 'none'
		project_link_args += [ '-static-libgcc', '-static-libstdc++' ]
	endif
endif

if copt_platform == 'mac' and uopt_lua == 'luajit' and copt_architecture != 'arm64'
	project_link_args += [
		'-pagezero_size', '10000',
		'-image_base', '100000000',
	]
endif

project_inc = include_directories([ 'src', 'data', 'resources' ])

if copt_platform == 'win'
	copt_identplatform = copt_64bit ? 'WIN64' : 'WIN32'
elif copt_platform == 'lin'
	copt_identplatform = copt_64bit ? 'LIN64' : 'LIN32'
elif copt_platform == 'mac'
	copt_identplatform = copt_architecture == 'arm64' ? 'MACOSARM' : 'MACOSX'
else
	copt_identplatform = 'UNKNOWN'
endif

conf_data.set('LIN', copt_platform == 'lin')
conf_data.set('AND', copt_platform == 'and')
conf_data.set('WIN', copt_platform == 'win')
conf_data.set('MACOSX', copt_platform == 'mac')
conf_data.set('X86', copt_x86)
conf_data.set('X86_SSE3', uopt_x86_sse_level >= 30)
conf_data.set('X86_SSE2', uopt_x86_sse_level >= 20)
conf_data.set('X86_SSE', uopt_x86_sse_level >= 10)
conf_data.set('NATIVE', uopt_native)
conf_data.set('_64BIT', copt_64bit)
conf_data.set('OGLI', get_option('ogli'))
conf_data.set('OGLR', get_option('oglr'))
conf_data.set('PIX32OGL', get_option('ogli'))
conf_data.set('BETA', get_option('beta'))
conf_data.set('NO_INSTALL_CHECK', not get_option('install_check'))
conf_data.set('IGNORE_UPDATES', get_option('ignore_updates'))
conf_data.set('MOD_ID', get_option('mod_id'))
conf_data.set('DEBUG', get_option('debug'))
conf_data.set('SNAPSHOT', get_option('snapshot'))
conf_data.set('SNAPSHOT_ID', get_option('snapshot_id'))
conf_data.set('SERVER', '"' + get_option('server') + '"')
conf_data.set('STATICSERVER', '"' + get_option('static_server') + '"')
conf_data.set('IDENT_PLATFORM', '"' + copt_identplatform + '"')
conf_data.set('IDENT', '"@0@-@1@-@2@"'.format(copt_architecture, copt_platform, copt_compiler).to_upper())

if get_option('update_server') != ''
	conf_data.set('UPDATESERVER', '"' + get_option('update_server') + '"')
else
	conf_data.set('UPDATESERVER', false)
endif

resources_files = []

subdir('src')
subdir('data')
subdir('resources')

if get_option('build_powder')
	powder_deps = [
		threads_dep,
		zlib_dep,
		sdl2_dep,
		sdl2main_dep,
		lua_opt_dep,
		curl_opt_dep,
		fftw_opt_dep,
	]
	if copt_platform == 'win'
		powder_deps += other_dep
	endif
	if copt_platform == 'and'
		subdir('android')
		powder_sha = shared_library(
			'powder',
			sources: powder_files,
			include_directories: [ project_inc, powder_inc ],
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			link_args: project_link_args,
			dependencies: powder_deps,
		)
		python3 = find_program('python3')
		android_platform_jar = meson.get_external_property('android_platform_jar')
		java_runtime_jar = meson.get_external_property('java_runtime_jar')
		android_keystore = get_option('android_keystore')
		sdl_jar = tpt_libs.get_variable('sdl_jar')
		powder_jar = custom_target(
			'powder-jar',
			output: 'powder.jar',
			command: [
				python3,
				files('android/powder-jar.py'),
				find_program('javac'),
				find_program('jar'),
				meson.current_source_dir(),
				meson.current_build_dir(),
				'@PRIVATE_DIR@',
				'@OUTPUT@',
				android_platform_jar,
				java_runtime_jar,
				sdl_jar,
				get_option('debug') ? 'debug' : 'release',
				powder_jar_sources,
			],
		)
		unaligned_apk = custom_target(
			'build-apk',
			output: 'powder.unaligned.apk',
			input: powder_sha,
			command: [
				python3,
				files('android/build-apk.py'),
				find_program('d8'),
				find_program('aapt'),
				find_program('aapt2'),
				meson.current_source_dir(),
				meson.current_build_dir(),
				'@PRIVATE_DIR@',
				'@OUTPUT@',
				'@INPUT@',
				android_platform_jar,
				sdl_jar,
				powder_jar,
				copt_architecture,
				get_option('debug') ? 'debug' : 'release',
				android_manifest_xml,
				android_resources,
			],
		)
		unsigned_apk = custom_target(
			'align-apk',
			output: 'powder.unsigned.apk',
			input: unaligned_apk,
			command: [
				python3,
				files('android/align-apk.py'),
				find_program('zipalign'),
				meson.current_build_dir(),
				'@OUTPUT@',
				'@INPUT@',
			],
		)
		signed_apk = custom_target(
			'sign-apk',
			output: 'powder.apk',
			input: unsigned_apk,
			command: [
				python3,
				files('android/sign-apk.py'),
				find_program('apksigner'),
				meson.current_build_dir(),
				'@OUTPUT@',
				'@INPUT@',
				android_keystore,
				get_option('android_keyalias'),
			],
		)
		adb = find_program('adb', required: false)
		if adb.found()
			install_apk = custom_target(
				'install-apk',
				input: signed_apk,
				output: 'install-apk',
				command: [ python3, files('android/install-apk.py'), adb, meson.current_build_dir(), '@OUTPUT@', '@INPUT@' ],
			)
			custom_target(
				'run-apk',
				input: [ signed_apk, install_apk ],
				output: 'run-apk',
				command: [ python3, files('android/run-apk.py'), adb, meson.current_build_dir(), '@OUTPUT@', '@INPUT0@' ],
			)
		endif
	else
		executable(
			'powder',
			sources: powder_files,
			include_directories: [ project_inc, powder_inc ],
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			gui_app: not get_option('debug'),
			link_args: project_link_args,
			dependencies: powder_deps,
		)
	endif
endif

if get_option('build_render')
	render_deps = [
		threads_dep,
		zlib_dep,
	]
	executable(
		'render',
		sources: render_files,
		include_directories: [ project_inc, render_inc ],
		c_args: project_c_args,
		cpp_args: project_cpp_args,
		link_args: project_link_args,
		dependencies: render_deps,
	)
endif

if get_option('build_font')
	font_deps = [
		threads_dep,
		zlib_dep,
		sdl2_dep,
		sdl2main_dep,
	]
	if copt_platform == 'win'
		font_deps += other_dep
	endif
	executable(
		'font',
		sources: font_files,
		include_directories: [ project_inc, font_inc ],
		c_args: project_c_args,
		cpp_args: project_cpp_args,
		link_args: project_link_args,
		dependencies: font_deps,
	)
endif
