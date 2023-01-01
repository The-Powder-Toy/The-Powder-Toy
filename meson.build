project(
	'the-powder-toy',
	[ 'c', 'cpp' ],
	version: 'the.cake.is.a.lie',
	default_options: [
		'c_std=c99',
		'cpp_std=c++17',
		'cpp_rtti=false',
		'build.c_std=c99', # used by to_array
		'build.cpp_std=c++17', # used by to_array
		'build.cpp_rtti=false', # used by to_array
	],
	meson_version: '>=0.61.0',
)

if get_option('prepare')
	# we're being run by prepare.py in a ghactions workflow only to determine the values of options; exit early
	subdir_done()
endif

to_array = generator(
	executable('toarray', sources: 'resources/ToArray.cpp', native: true),
	output: [ '@PLAINNAME@.cpp', '@PLAINNAME@.h' ],
	arguments: [ '@OUTPUT0@', '@OUTPUT1@', '@INPUT@', '@EXTRA_ARGS@' ]
)

render_icons_with_inkscape = get_option('render_icons_with_inkscape')
inkscape = find_program('inkscape', required: render_icons_with_inkscape)

c_compiler = meson.get_compiler('c')

is_x86 = host_machine.cpu_family() in [ 'x86', 'x86_64' ]
is_64bit = host_machine.cpu_family() in [ 'aarch64', 'x86_64' ]
is_msvc = c_compiler.get_id() in [ 'msvc' ]

host_arch = host_machine.cpu_family()
host_platform = host_machine.system()
# educated guesses follow, PRs welcome
if c_compiler.get_id() in [ 'msvc' ]
	host_libc = 'msvc'
elif c_compiler.get_id() in [ 'gcc' ] and host_platform == 'windows'
	host_libc = 'mingw'
	if get_option('b_lto')
		warning('mingw does not like static + lto')
	endif
elif host_platform in [ 'darwin' ]
	host_libc = 'macos'
elif host_platform in [ 'android' ]
	host_platform = 'android'
	host_libc = 'bionic'
else
	host_libc = 'gnu'
endif

static_variant = get_option('static')
if static_variant != 'prebuilt' and host_platform == 'android'
	warning('only prebuilt libs are supported for android')
	static_variant = 'prebuilt'
endif
if static_variant == 'system' and host_platform == 'windows'
	warning('no way to find system libs on windows')
	static_variant = 'prebuilt'
endif

is_static = static_variant != 'none'
is_debug = get_option('optimization') in [ '0', 'g' ]
enforce_https = get_option('enforce_https')

if not is_debug and not enforce_https
	error('refusing to build a release binary with enforce_https=false')
endif

tpt_libs_static = 'none'
if static_variant == 'prebuilt'
	tpt_libs_static = 'static'
endif
if static_variant == 'none' and host_platform == 'windows'
	tpt_libs_static = 'dynamic'
endif
tpt_libs_debug = is_debug ? 'debug' : 'release'
tpt_libs_variant = '@0@-@1@-@2@-@3@'.format(host_arch, host_platform, host_libc, tpt_libs_static)
tpt_libs_vtag = get_option('tpt_libs_vtag')
if tpt_libs_vtag == ''
	tpt_libs_vtag = 'v20230101182417'
endif
if tpt_libs_static != 'none'
	if tpt_libs_variant not in [
		'x86_64-linux-gnu-static',
		'x86_64-windows-mingw-static',
		'x86_64-windows-mingw-dynamic',
		'x86_64-windows-msvc-static',
		'x86_64-windows-msvc-dynamic',
		'x86-windows-msvc-static',
		'x86-windows-msvc-dynamic',
		'x86_64-darwin-macos-static',
		'aarch64-darwin-macos-static',
		'x86-android-bionic-static',
		'x86_64-android-bionic-static',
		'arm-android-bionic-static',
		'aarch64-android-bionic-static',
	]
		error('no prebuilt @0@ libraries are currently provided'.format(tpt_libs_variant))
	endif
	tpt_libs = subproject('tpt-libs-prebuilt-@0@-@1@-@2@'.format(tpt_libs_variant, tpt_libs_debug, tpt_libs_vtag))
else
	if get_option('workaround_elusive_bzip2')
		bzip2_lib_name = get_option('workaround_elusive_bzip2_lib_name')
		bzip2_include_name = get_option('workaround_elusive_bzip2_include_name')
		bzip2_lib_dir = get_option('workaround_elusive_bzip2_lib_dir')
		bzip2_include_dir = include_directories(get_option('workaround_elusive_bzip2_include_dir'))
		bzip2_static = get_option('workaround_elusive_bzip2_static')
		meson.override_dependency('bzip2', declare_dependency(
			dependencies: c_compiler.find_library(
				bzip2_lib_name,
				has_headers: bzip2_include_name,
				dirs: bzip2_lib_dir,
				header_include_directories: bzip2_include_dir,
				static: bzip2_static,
			),
			include_directories: bzip2_include_dir,
		))
	endif
endif

conf_data = configuration_data()
conf_data.set('CURL_STATICLIB', false)
conf_data.set('ZLIB_WINAPI', false)

x86_sse_level_str = get_option('x86_sse')
if x86_sse_level_str == 'auto'
	x86_sse_level = 20
	if not is_x86 or not is_64bit or host_platform == 'darwin' or (is_64bit and is_msvc)
		x86_sse_level = 0
	endif
elif x86_sse_level_str == 'sse3'
	x86_sse_level = 30
elif x86_sse_level_str == 'sse2'
	x86_sse_level = 20
elif x86_sse_level_str == 'sse'
	x86_sse_level = 10
elif x86_sse_level_str == 'none'
	x86_sse_level = 0
endif

lua_variant = get_option('lua')
if lua_variant == 'auto'
	lua_variant = 'luajit'
endif
if lua_variant == 'none'
	lua_dep = []
elif lua_variant == 'lua5.1' or lua_variant == 'lua5.2'
	lua_dep = dependency(lua_variant + '-c++', static: is_static, required: false)
	if not lua_dep.found()
		if not get_option('workaround_noncpp_lua')
			error('your system @0@ is not compatible with C++, configure with -Dworkaround_noncpp_lua=true to disable this error'.format(lua_variant))
		endif
		lua_dep = dependency(lua_variant, static: is_static)
	endif
elif lua_variant == 'luajit'
	lua_dep = dependency('luajit', static: is_static)
endif

enable_http = get_option('http')
if host_platform == 'android'
	enable_http = false
endif
if host_platform == 'android'
	android_ndk_toolchain_prefix = meson.get_external_property('android_ndk_toolchain_prefix')
	android_platform = meson.get_external_property('android_platform')
	tpt_libs_android_toolchain_prefix = tpt_libs.get_variable('android_toolchain_prefix')
	tpt_libs_android_system_version = tpt_libs.get_variable('android_system_version')
	tpt_libs_android_platform = tpt_libs.get_variable('android_platform')
	if '@0@@1@-'.format(tpt_libs_android_toolchain_prefix, tpt_libs_android_system_version) != android_ndk_toolchain_prefix
		error('tpt-libs android toolchain mismatch')
	endif
	if tpt_libs_android_platform != android_platform
		error('tpt-libs android platform mismatch')
	endif
endif
curl_dep = enable_http ? dependency('libcurl', static: is_static) : []

enable_gravfft = get_option('gravfft')
fftw_dep = enable_gravfft ? dependency('fftw3f', static: is_static) : []

threads_dep = dependency('threads')
zlib_dep = dependency('zlib', static: is_static)
png_dep = dependency('libpng16', static: is_static)
sdl2_dep = dependency('sdl2', static: is_static)
bzip2_dep = dependency('bzip2', static: is_static)
json_dep = dependency('jsoncpp', static: is_static)

project_link_args = []
project_c_args = []
project_cpp_args = []
if is_msvc
	if x86_sse_level >= 30
		warning('SSE3 configured to be enabled but unavailable in msvc')
		x86_sse_level = 20
	endif
	if is_64bit and x86_sse_level > 0
		warning('SSE explicitly configured but unavailable in msvc targeting 64-bit machines')
		x86_sse_level = 0
	endif
	args_msvc = [
		'/GS',
		'/D_SCL_SECURE_NO_WARNINGS',
		'/DUNICODE',
		'/D_UNICODE',
	]
	if x86_sse_level >= 20
		args_msvc += [ '/arch:SSE2' ]
	elif x86_sse_level >= 10
		args_msvc += [ '/arch:SSE' ]
	endif
	if not is_debug
		args_msvc += [
			'/Oy-',
			'/fp:fast',
			'/GL',
		]
		project_link_args += [
			'/OPT:REF',
			'/OPT:ICF',
			'/LTCG',
		]
	endif
	project_c_args += args_msvc
	project_cpp_args += args_msvc
else
	args_ccomp = []
	if host_platform == 'darwin' and x86_sse_level > 0
		message('SSE level explicitly configured but unavailable on macosx')
		x86_sse_level = 0
	endif
	if x86_sse_level >= 30
		args_ccomp += [ '-msse3' ]
	elif x86_sse_level >= 20
		args_ccomp += [ '-msse2' ]
	elif x86_sse_level >= 10
		args_ccomp += [ '-msse' ]
	endif
	if host_platform == 'windows'
		args_ccomp += [
			'-DUNICODE',
			'-D_UNICODE',
		]
	endif
	if not is_debug
		args_ccomp += [
			'-ftree-vectorize',
			'-funsafe-math-optimizations',
			'-ffast-math',
			'-fomit-frame-pointer',
		]
	endif
	if host_platform == 'android'
		if not is_64bit
			args_ccomp += [ '-U_FILE_OFFSET_BITS' ]
		endif
		# android doesn't ship libc++_shared.so, so we might as well link it statically;
		# the alternative would be to grab libc++_shared.so from the NDK and ship it with
		# the app alongside libpowder.so, and possibly add it to SDL's list of libraries to load
		project_link_args += [ '-static-libstdc++' ]
	endif
	project_c_args += args_ccomp + [
		'-Wno-implicit-fallthrough',
		'-Wno-missing-field-initializers',
		'-Wno-unused-result',
		'-Wno-unused-parameter',
	]
	project_cpp_args += args_ccomp + [
		'-Wno-invalid-offsetof',
		'-Wno-unused-result',
		'-Wno-missing-field-initializers',
		'-Wno-unused-parameter',
	]
endif

if host_platform == 'windows'
	args_ccomp_win = [ '-D_WIN32_WINNT=0x0501' ]
	project_c_args   += args_ccomp_win
	project_cpp_args += args_ccomp_win
	windows_mod = import('windows')
	if is_static
		conf_data.set('CURL_STATICLIB', true)
		if host_arch == 'x86_64'
			conf_data.set('ZLIB_WINAPI', true)
		endif
	else
		foreach input_output_condition : tpt_libs.get_variable('config_dlls')
			dll_input = input_output_condition[0]
			dll_output = input_output_condition[1]
			dll_condition = input_output_condition[2]
			do_copy = false
			if dll_condition == 'all'
				do_copy = true
			elif dll_condition == 'lua=' + lua_variant
				do_copy = true
			endif
			if do_copy
				configure_file(input: dll_input, output: dll_output, copy: true)
			endif
		endforeach
	endif
endif

project_inc = include_directories([ 'src', 'resources' ])

if host_platform == 'windows'
	ident_platform = is_64bit ? 'WIN64' : 'WIN32'
elif host_platform == 'linux'
	ident_platform = is_64bit ? 'LIN64' : 'LIN32'
elif host_platform == 'darwin'
	ident_platform = host_arch == 'aarch64' ? 'MACOSARM' : 'MACOSX'
else
	ident_platform = 'UNKNOWN'
endif

app_exe = get_option('app_exe')
app_id = get_option('app_id')

conf_data.set('LIN', host_platform == 'linux')
conf_data.set('AND', host_platform == 'android')
conf_data.set('WIN', host_platform == 'windows')
conf_data.set('MACOSX', host_platform == 'darwin')
conf_data.set('X86', is_x86)
conf_data.set('X86_SSE3', x86_sse_level >= 30)
conf_data.set('X86_SSE2', x86_sse_level >= 20)
conf_data.set('X86_SSE', x86_sse_level >= 10)
conf_data.set('_64BIT', is_64bit)
conf_data.set('BETA', get_option('beta'))
conf_data.set('NO_INSTALL_CHECK', not get_option('install_check'))
conf_data.set('IGNORE_UPDATES', get_option('ignore_updates'))
conf_data.set('MOD_ID', get_option('mod_id'))
conf_data.set('DEBUG', is_debug)
conf_data.set('SNAPSHOT', get_option('snapshot'))
conf_data.set('SNAPSHOT_ID', get_option('snapshot_id'))
conf_data.set('SERVER', '"@0@"'.format(get_option('server')))
conf_data.set('STATICSERVER', '"@0@"'.format(get_option('static_server')))
conf_data.set('IDENT_PLATFORM', '"@0@"'.format(ident_platform))
conf_data.set('IDENT', '"@0@-@1@-@2@"'.format(host_arch, host_platform, host_libc).to_upper())
conf_data.set('ENFORCE_HTTPS', enforce_https)
conf_data.set('APPNAME', get_option('app_name'))
conf_data.set('APPCOMMENT', get_option('app_comment'))
conf_data.set('APPEXE', app_exe)
conf_data.set('APPID', app_id)
conf_data.set('APPDATA', get_option('app_data'))
conf_data.set('APPVENDOR', get_option('app_vendor'))

if get_option('update_server') != ''
	conf_data.set('UPDATESERVER', '"@0@"'.format(get_option('update_server')))
else
	conf_data.set('UPDATESERVER', false)
endif

data_files = []

subdir('src')
subdir('resources')

powder_files += data_files
render_files += data_files
font_files += data_files

if get_option('build_powder')
	powder_deps = [
		threads_dep,
		zlib_dep,
		png_dep,
		sdl2_dep,
		lua_dep,
		curl_dep,
		fftw_dep,
		bzip2_dep,
		json_dep,
	]
	if host_platform == 'android'
		powder_sha = shared_library(
			app_exe,
			sources: powder_files,
			include_directories: [ project_inc, powder_inc ],
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			link_args: project_link_args,
			dependencies: powder_deps,
		)
		subdir('android')
	else
		executable(
			app_exe,
			sources: powder_files,
			include_directories: [ project_inc, powder_inc ],
			c_args: project_c_args,
			cpp_args: project_cpp_args,
			win_subsystem: is_debug ? 'console' : 'windows',
			link_args: project_link_args,
			dependencies: powder_deps,
		)
	endif
endif

if get_option('build_render')
	render_deps = [
		threads_dep,
		zlib_dep,
		bzip2_dep,
		json_dep,
		png_dep,
	]
	render_link_args = project_link_args
	if host_platform == 'linux' and is_static
		render_link_args += [ '-static' ]
	endif
	executable(
		'render',
		sources: render_files,
		include_directories: [ project_inc, render_inc ],
		c_args: project_c_args,
		cpp_args: project_cpp_args,
		link_args: render_link_args,
		dependencies: render_deps,
	)
endif

if get_option('build_font')
	font_deps = [
		threads_dep,
		zlib_dep,
		png_dep,
		sdl2_dep,
		bzip2_dep,
		json_dep,
	]
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
