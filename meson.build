project('the-powder-toy', [ 'c', 'cpp' ], version: 'the.cake.is.a.lie', default_options: [
	'cpp_std=c++11',
])

to_array = generator(
	import('python').find_installation('python3'),
	output: [ '@PLAINNAME@.cpp', '@PLAINNAME@.h' ],
	arguments: [ join_paths(meson.current_source_dir(), 'to_array.py'), '@BUILD_DIR@', '@OUTPUT0@', '@OUTPUT1@', '@INPUT@', '@EXTRA_ARGS@' ]
)

cpp_compiler = meson.get_compiler('cpp')

project_c_args = []
project_cpp_args = []
project_link_args = []

conf_data = configuration_data()
conf_data.set('CURL_STATICLIB', false)
conf_data.set('ZLIB_WINAPI', false)

copt_x86 = host_machine.cpu_family() in [ 'x86_64', 'x86' ]
copt_64bit = host_machine.cpu_family() in [ 'x86_64', 'aarch64' ]
copt_msvc = cpp_compiler.get_id() in [ 'msvc' ]

if host_machine.system() in [ 'linux', 'freebsd' ]
	copt_platform = 'linux'
elif host_machine.system() in [ 'windows' ]
	copt_platform = 'windows'
elif host_machine.system() in [ 'darwin' ]
	copt_platform = 'macosx'
else
	error('unsupported platform: ' + host_machine.system())
endif

if copt_platform == 'linux' and not copt_64bit
	error('lin32 is not supported')
endif
if copt_platform == 'windows' and not copt_64bit
	error('win32 is not supported')
endif
if copt_platform == 'macosx' and not copt_64bit
	error('mac32 is not even a thing')
endif

if get_option('ogli') or get_option('oglr')
	error('OpenGL features are currently unavailable')
endif

uopt_static = get_option('static')
use_tpt_libs = false
tpt_libs_vtag = 'v20210103095432'
if uopt_static == 'system'
	if copt_platform == 'windows'
		error('no way to find static system libraries on windows')
	endif
elif uopt_static == 'prebuilt'
	if copt_platform == 'windows'
		use_tpt_libs = true
		tpt_libs = subproject('tpt-libs-prebuilt-win64-static-' + tpt_libs_vtag)
	elif copt_platform == 'linux'
		use_tpt_libs = true
		tpt_libs = subproject('tpt-libs-prebuilt-lin64-static-' + tpt_libs_vtag)
	elif copt_platform == 'macosx'
		use_tpt_libs = true
		tpt_libs = subproject('tpt-libs-prebuilt-mac64-static-' + tpt_libs_vtag)
	endif
else
	if copt_platform == 'windows'
		use_tpt_libs = true
		tpt_libs = subproject('tpt-libs-prebuilt-win64-dynamic-' + tpt_libs_vtag)
	endif
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
if uopt_lua == 'luajit'
	lua_opt_dep = [ use_tpt_libs ? tpt_libs.get_variable('luajit_dep') : dependency('luajit', static: uopt_static == 'system') ]
elif uopt_lua == 'lua5.2'
	lua_opt_dep = [ use_tpt_libs ? tpt_libs.get_variable('lua52_dep') : dependency('lua5.2-c++', static: uopt_static == 'system') ]
elif uopt_lua == 'lua5.1'
	lua_opt_dep = [ use_tpt_libs ? tpt_libs.get_variable('lua51_dep') : dependency('lua5.1-c++', static: uopt_static == 'system') ]
else
	lua_opt_dep = []
endif

uopt_http = get_option('http')
if uopt_http
	curl_opt_dep = [ use_tpt_libs ? tpt_libs.get_variable('libcurl_dep') : dependency('libcurl', static: uopt_static == 'system') ]
else
	curl_opt_dep = []
endif

uopt_fftw = get_option('gravfft')
if uopt_fftw
	fftw_opt_dep = [ use_tpt_libs ? tpt_libs.get_variable('fftw_dep') : dependency('fftw3f', static: uopt_static == 'system') ]
else
	fftw_opt_dep = []
endif

threads_dep = dependency('threads')
zlib_dep = use_tpt_libs ? tpt_libs.get_variable('zlib_dep') : dependency('zlib', static: uopt_static == 'system')
sdl2_dep = use_tpt_libs ? tpt_libs.get_variable('sdl2_dep') : dependency('sdl2', static: uopt_static == 'system')

if copt_msvc
	if uopt_x86_sse_level >= 30
		message('SSE3 configured to be enabled but unavailable in msvc')
		uopt_x86_sse_level = 20
	endif
	if uopt_native
		message('local machine optimization configured to be enabled but unavailable in msvc')
		uopt_native = false
	endif
	if copt_64bit
		message('SSE explicitly configured but unavailable in msvc targeting 64-bit machines')
	else
		args_msvc_sse = []
		if uopt_x86_sse_level >= 20
			args_msvc_sse += '/arch:SSE2'
		elif uopt_x86_sse_level >= 10
			args_msvc_sse += '/arch:SSE'
		endif
		project_c_args += args_msvc_sse
		project_cpp_args += args_msvc_sse
	endif
	args_msvc = [ '/GS', '-D_SCL_SECURE_NO_WARNINGS' ]
	project_c_args += args_msvc
	project_cpp_args += args_msvc
	project_link_args += [
		'/OPT:REF',
		'/OPT:ICF',
	]
	if not get_option('debug')
		args_msvc_opt = [ '/Oy-', '/fp:fast' ]
		project_c_args += args_msvc_opt
		project_cpp_args += args_msvc_opt
	endif
else
	if copt_platform == 'macosx'
		if uopt_x86_sse_level >= 0
			message('SSE level explicitly configured but unavailable on macosx')
			uopt_x86_sse_level = 0
		endif
		if uopt_native
			message('local machine optimization configured to be enabled but unavailable on macosx')
			uopt_native = false
		endif
	else
		args_ccomp_sse = []
		if uopt_x86_sse_level >= 30
			args_ccomp_sse += '-msse3'
		endif
		if uopt_x86_sse_level >= 20
			args_ccomp_sse += '-msse2'
		endif
		if uopt_x86_sse_level >= 10
			args_ccomp_sse += '-msse'
		endif
		if uopt_native
			args_ccomp_sse += '-march=native'
		endif
		project_c_args += args_ccomp_sse
		project_cpp_args += args_ccomp_sse
	endif
	project_c_args += [ '-U__STRICT_ANSI__', '-Wno-unused-result' ]
	project_cpp_args += [ '-U__STRICT_ANSI__', '-Wno-unused-result', '-Wno-invalid-offsetof' ]
	if not get_option('debug')
		args_ccomp = [ '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer' ]
		project_c_args += args_ccomp
		project_cpp_args += args_ccomp
	endif
endif

if copt_platform == 'windows'
	other_dep = tpt_libs.get_variable('other_dep')
	sdl2main_dep = tpt_libs.get_variable('sdl2main_dep')
	project_c_args   += [ '-D_WIN32_WINNT=0x0501' ]
	project_cpp_args += [ '-D_WIN32_WINNT=0x0501' ]
	windows_mod = import('windows')
	if uopt_static != 'none'
		conf_data.set('CURL_STATICLIB', true)
		conf_data.set('ZLIB_WINAPI', true)
	else
		foreach input_and_output : tpt_libs.get_variable('config_dlls')
			configure_file(input: input_and_output[0], output: input_and_output[1], copy: true)
		endforeach
	endif
endif

if copt_platform == 'macosx' and uopt_lua == 'luajit'
	project_link_args += [ '-pagezero_size', '10000', '-image_base', '100000000' ]
endif

project_inc = include_directories([ 'src', 'data', 'resources' ])

conf_data.set('LIN', copt_platform == 'linux')
conf_data.set('WIN', copt_platform == 'windows')
conf_data.set('MACOSX', copt_platform == 'macosx')
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
conf_data.set('SAVE_VERSION', get_option('version_major'))
conf_data.set('MINOR_VERSION', get_option('version_minor'))
conf_data.set('BUILD_NUM', get_option('version_build'))
conf_data.set('MOD_ID', get_option('mod_id'))
conf_data.set('DEBUG', get_option('debug'))
conf_data.set('SNAPSHOT', get_option('snapshot'))
conf_data.set('SNAPSHOT_ID', get_option('snapshot_id'))
conf_data.set('FUTURE_SAVE_VERSION', get_option('future_major'))
conf_data.set('FUTURE_MINOR_VERSION', get_option('future_minor'))
conf_data.set('SERVER', '"' + get_option('server') + '"')
conf_data.set('STATICSERVER', '"' + get_option('static_server') + '"')
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
	powder_args = []
	if uopt_lua != 'none'
		powder_args += '-DLUACONSOLE'
	endif
	if not uopt_http
		powder_args += '-DNOHTTP'
	endif
	if uopt_fftw
		powder_args += '-DGRAVFFT'
	endif
	powder_deps = [
		threads_dep,
		zlib_dep,
		sdl2_dep,
		lua_opt_dep,
		curl_opt_dep,
		fftw_opt_dep,
	]
	if copt_platform == 'windows'
		powder_deps += other_dep
		powder_deps += sdl2main_dep
	endif
	executable(
		'powder',
		sources: powder_files,
		include_directories: project_inc,
		c_args: project_c_args + powder_args,
		cpp_args: project_cpp_args + powder_args,
		cpp_pch: 'pch/pch_cpp.h',
		gui_app: true,
		link_args: project_link_args,
		dependencies: powder_deps,
	)
endif

if get_option('build_render')
	render_args = [ '-DRENDERER', '-DNOHTTP' ]
	render_deps = [
		threads_dep,
		zlib_dep,
	]
	executable(
		'render',
		sources: render_files,
		include_directories: project_inc,
		c_args: project_c_args + render_args,
		cpp_args: project_cpp_args + render_args,
		cpp_pch: 'pch/pch_cpp.h',
		link_args: project_link_args,
		dependencies: render_deps,
	)
endif

if get_option('build_font')
	font_args = [ '-DFONTEDITOR', '-DNOHTTP' ]
	font_deps = [
		threads_dep,
		zlib_dep,
		sdl2_dep,
	]
	if copt_platform == 'windows'
		font_deps += other_dep
		font_deps += sdl2main_dep
	endif
	executable(
		'font',
		sources: font_files,
		include_directories: project_inc,
		c_args: project_c_args + font_args,
		cpp_args: project_cpp_args + font_args,
		cpp_pch: 'pch/pch_cpp.h',
		gui_app: true,
		link_args: project_link_args,
		dependencies: font_deps,
	)
endif
