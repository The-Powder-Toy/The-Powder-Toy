conf_data = configuration_data()

app_id = get_option('app_id')
mod_id = get_option('mod_id')
is_snapshot = get_option('snapshot')
is_beta = get_option('beta')
is_mod = mod_id > 0
conf_data.set('X86', is_x86.to_string())
conf_data.set('BETA', is_beta.to_string())
conf_data.set('MOD_ID', mod_id)
conf_data.set('DEBUG', is_debug.to_string())
conf_data.set('MOD', is_mod.to_string())
conf_data.set('SNAPSHOT', is_snapshot.to_string())
conf_data.set('DISPLAY_VERSION_MAJOR', get_option('display_version_major'))
conf_data.set('DISPLAY_VERSION_MINOR', get_option('display_version_minor'))
conf_data.set('BUILD_NUM', get_option('build_num'))
conf_data.set('UPSTREAM_VERSION_MAJOR', get_option('upstream_version_major'))
conf_data.set('UPSTREAM_VERSION_MINOR', get_option('upstream_version_minor'))
conf_data.set('UPSTREAM_BUILD_NUM', get_option('upstream_build_num'))
conf_data.set('MANIFEST_COPYRIGHT', get_option('manifest_copyright'))
conf_data.set('MANIFEST_MACOS_MIN_VER', get_option('manifest_macos_min_ver'))
conf_data.set('MANIFEST_DATE', get_option('manifest_date'))
conf_data.set('ALLOW_FAKE_NEWER_VERSION', (is_snapshot or is_beta or is_debug or is_mod).to_string())
conf_data.set('IDENT_PLATFORM', ident_platform)
conf_data.set('IDENT', '@0@-@1@-@2@'.format(host_arch, host_platform, host_libc).to_upper())

update_server = get_option('update_server')
conf_data.set('UPDATESERVER', update_server)
conf_data.set('USE_UPDATESERVER', (update_server != '').to_string())

enforce_https = get_option('enforce_https')
allow_quit = true
force_window_frame_ops = 'forceWindowFrameOpsNone'
allow_data_folder = true
if host_platform == 'emscripten'
	allow_quit = false
	force_window_frame_ops = 'forceWindowFrameOpsEmbedded'
	allow_data_folder = false
endif
default_touch_ui = false
if host_platform == 'android'
	default_touch_ui = true # TODO: some more sophisticated heuristic at runtime instead
	force_window_frame_ops = 'forceWindowFrameOpsHandheld'
endif
secure_ciphers_only = get_option('secure_ciphers_only')
if not is_debug and not enforce_https
	error('refusing to build a release binary without enforcing HTTPS, configure with -Denforce_https=true to fix this error')
endif
conf_data.set('ALLOW_QUIT', allow_quit.to_string())
conf_data.set('FORCE_WINDOW_FRAME_OPS', force_window_frame_ops)
conf_data.set('DEFAULT_TOUCH_UI', default_touch_ui.to_string())
conf_data.set('ALLOW_DATA_FOLDER', allow_data_folder.to_string())
conf_data.set('ENFORCE_HTTPS', enforce_https.to_string())
conf_data.set('SECURE_CIPHERS_ONLY', secure_ciphers_only.to_string())

conf_data.set('IGNORE_UPDATES', get_option('ignore_updates').to_string())
conf_data.set('SERVER', get_option('server'))
conf_data.set('STATICSERVER', get_option('static_server'))
conf_data.set('APPNAME', get_option('app_name'))
conf_data.set('APPCOMMENT', get_option('app_comment'))
conf_data.set('APPEXE', app_exe)
conf_data.set('APPID', app_id)
conf_data.set('APPDATA', get_option('app_data'))
conf_data.set('APPVENDOR', get_option('app_vendor'))

if host_platform == 'android'
	android_permissions = [
		'<uses-permission android:name="android.permission.VIBRATE" />',
	]
	if enable_http
		android_permissions += [
			'<uses-permission android:name="android.permission.INTERNET" />',
		]
	endif
	android_properties = []
	if is_debug
		android_properties += [ 'android:debuggable="true"' ]
	endif
	conf_data.set('ANDROID_PERMISSIONS', '\n'.join(android_permissions))
	conf_data.set('ANDROID_PROPERTIES', '\n'.join(android_properties))
endif

powder_files = files(
	'SDLCompat.cpp',
	'PowderToySDL.cpp',
	'PowderToy.cpp',
	'lua/CommandInterface.cpp',
	'lua/TPTScriptInterface.cpp',
	'lua/TPTSTypes.cpp',
)
if host_platform == 'emscripten'
	powder_files += files(
		'PowderToySDLEmscripten.cpp',
	)
else
	powder_files += files(
		'PowderToySDLCommon.cpp',
	)
endif
if is_x86
	powder_files += files('X86KillDenormals.cpp')
endif

render_files = files(
	'PowderToyRenderer.cpp',
)

font_files = files(
	'PowderToyFontEditor.cpp',
	'PowderToySDL.cpp',
	'PowderToySDLCommon.cpp',
)

common_files = files(
	'Format.cpp',
	'Misc.cpp',
	'Probability.cpp',
)

resolve_vcs_tag = get_option('resolve_vcs_tag')
if resolve_vcs_tag == 'yes' or (resolve_vcs_tag == 'static_release_only' and not is_debug and is_static)
	common_files += vcs_tag(
		input: 'VcsTag.template.h',
		output: 'VcsTag.h',
		fallback: 'unknown',
	)
else
	vcs_tag_conf_data = configuration_data()
	vcs_tag_conf_data.set('VCS_TAG', '')
	configure_file(
		input: 'VcsTag.template.h',
		output: 'VcsTag.h',
		configuration: vcs_tag_conf_data
	)
endif

if host_platform == 'linux'
	powder_files += files('WindowIcon.cpp')
	font_files += files('WindowIcon.cpp')
endif

subdir('bson')
subdir('bzip2')
subdir('client')
subdir('common')
subdir('debug')
subdir('graphics')
subdir('gui')
if lua_variant != 'none'
	subdir('lua')
	conf_data.set('LUACONSOLE', 'true')
else
	powder_files += files(
		'lua/PlainCommandInterface.cpp',
	)
	conf_data.set('LUACONSOLE', 'false')
endif
subdir('prefs')
subdir('resampler')
subdir('simulation')
subdir('tasks')

powder_files += common_files
render_files += common_files
font_files += common_files

simulation_elem_defs = []
foreach elem_name_id : simulation_elem_ids
	simulation_elem_defs += 'ELEMENT_DEFINE(' + elem_name_id[0] + ', ' + elem_name_id[1].to_string() + ');'
endforeach
elements_conf_data = configuration_data()
elements_conf_data.set('element_defs', '\n'.join(simulation_elem_defs))
configure_file(
	input: 'simulation/ElementNumbers.template.h',
	output: 'ElementNumbers.h',
	configuration: elements_conf_data
)

clipboard_impl_defs = []
foreach impl_subsys_factory : clipboard_impl_factories
	clipboard_impl_defs += 'IMPL_DEFINE(' + impl_subsys_factory[0] + ', ' + impl_subsys_factory[1] + ')'
endforeach
clipboard_impls_data = configuration_data()
clipboard_impls_data.set('impl_defs', '\n'.join(clipboard_impl_defs))
configure_file(
	input: 'common/clipboard/ClipboardImpls.template.h',
	output: 'ClipboardImpls.h',
	configuration: clipboard_impls_data
)

simulation_tool_defs = []
foreach tool_name_id : simulation_tool_ids
	simulation_tool_defs += 'TOOL_DEFINE(' + tool_name_id[0] + ', ' + tool_name_id[1].to_string() + ');'
endforeach
tools_conf_data = configuration_data()
tools_conf_data.set('tool_defs', '\n'.join(simulation_tool_defs))
configure_file(
	input: 'simulation/ToolNumbers.template.h',
	output: 'ToolNumbers.h',
	configuration: tools_conf_data
)

configure_file(
	input: 'Config.template.h',
	output: 'Config.h',
	configuration: conf_data
)
