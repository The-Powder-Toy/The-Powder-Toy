conf_data = configuration_data()

upstream_version = meson.project_version()
upstream_version_components = upstream_version.split('-')
if upstream_version_components.length() > 1
	upstream_version = upstream_version_components[upstream_version_components.length() - 1]
endif
display_version = upstream_version
override_display_version = get_option('override_display_version')
if override_display_version != ''
	display_version = override_display_version
endif
display_version = display_version.split('.')
upstream_version = upstream_version.split('.')

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
conf_data.set('DISPLAY_VERSION_MAJOR', display_version[0].to_int())
conf_data.set('DISPLAY_VERSION_MINOR', display_version[1].to_int())
conf_data.set('BUILD_NUM', display_version[2].to_int())
conf_data.set('UPSTREAM_VERSION_MAJOR', upstream_version[0].to_int())
conf_data.set('UPSTREAM_VERSION_MINOR', upstream_version[1].to_int())
conf_data.set('UPSTREAM_BUILD_NUM', upstream_version[2].to_int())
conf_data.set('MANIFEST_COPYRIGHT', get_option('manifest_copyright'))
conf_data.set('MANIFEST_MACOS_MIN_VER', get_option('manifest_macos_min_ver'))
conf_data.set('MANIFEST_DATE', get_option('manifest_date'))
conf_data.set('ALLOW_FAKE_NEWER_VERSION', (is_snapshot or is_beta or is_debug or is_mod).to_string())
conf_data.set('IDENT_PLATFORM', ident_platform)
conf_data.set('IDENT', '@0@-@1@-@2@'.format(host_arch, host_platform, host_libc).to_upper())

enforce_https = get_option('enforce_https')
server = get_option('server')
static_server = get_option('static_server')
update_server = get_option('update_server')

if not (server.startswith('http://') or server.startswith('https://'))
	server = 'https://' + server
endif
if server.startswith('http://') and enforce_https
	error('enforce_https is true but server is a http:// URL base')
endif
if not (static_server.startswith('http://') or static_server.startswith('https://'))
	static_server = 'https://' + static_server
endif
if static_server.startswith('http://') and enforce_https
	error('enforce_https is true but static_server is a http:// URL base')
endif

use_update_server = false
if update_server != ''
	use_update_server = true
	if not (update_server.startswith('http://') or update_server.startswith('https://'))
		update_server = 'https://' + update_server
	endif
	if update_server.startswith('http://') and enforce_https
		error('enforce_https is true but update_server is a http:// URL base')
	endif
endif
conf_data.set('USE_UPDATESERVER', use_update_server.to_string())
conf_data.set('UPDATESERVER', update_server)

platform_clipboard = get_option('platform_clipboard')
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
conf_data.set('PLATFORM_CLIPBOARD', platform_clipboard.to_string())

conf_data.set('IGNORE_UPDATES', get_option('ignore_updates').to_string())
conf_data.set('SERVER', server)
conf_data.set('STATICSERVER', static_server)
conf_data.set('APPNAME', get_option('app_name'))
conf_data.set('APPCOMMENT', get_option('app_comment'))
conf_data.set('APPEXE', app_exe)
conf_data.set('APPID', app_id)
conf_data.set('APPDATA', get_option('app_data'))
conf_data.set('APPVENDOR', get_option('app_vendor'))
conf_data.set('PACKAGE_MODE', get_option('package_mode'))

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
	'lua/TPTSTypes.cpp',
)
powder_external_files = []

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

element_numbers_x = []
foreach elem_name_id : simulation_elem_ids
	element_numbers_x += 'X(@0@, @1@) \\'.format(elem_name_id[0], elem_name_id[1])
endforeach
configure_file(
	input: 'simulation/ElementNumbers.template.h',
	output: 'ElementNumbers.h',
	configuration: configuration_data({
		'element_numbers': '\n\t'.join(element_numbers_x),
	}),
)

if platform_clipboard
	clipboard_impls_x = []
	foreach impl_subsys_factory : clipboard_impl_factories
		clipboard_impls_x += 'X(@0@, @1@) \\'.format(impl_subsys_factory[0], impl_subsys_factory[1])
	endforeach
	configure_file(
		input: 'common/clipboard/ClipboardImpls.template.h',
		output: 'ClipboardImpls.h',
		configuration: configuration_data({
			'clipboard_impls': '\n\t'.join(clipboard_impls_x),
		}),
	)
endif

tool_numbers_x = []
foreach tool_name_id : simulation_tool_ids
	tool_numbers_x += 'X(@0@, @1@) \\'.format(tool_name_id[0], tool_name_id[1])
endforeach
configure_file(
	input: 'simulation/ToolNumbers.template.h',
	output: 'ToolNumbers.h',
	configuration: configuration_data({
		'tool_numbers': '\n\t'.join(tool_numbers_x),
	}),
)

configure_file(
	input: 'Config.template.h',
	output: 'Config.h',
	configuration: conf_data
)
