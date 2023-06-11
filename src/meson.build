conf_data = configuration_data()

app_id = get_option('app_id')
mod_id = get_option('mod_id')
is_snapshot = get_option('snapshot')
is_beta = get_option('beta')
is_mod = mod_id > 0
conf_data.set('X86', is_x86 ? 'true' : 'false')
conf_data.set('BETA', is_beta ? 'true' : 'false')
conf_data.set('MOD_ID', mod_id)
conf_data.set('DEBUG', is_debug ? 'true' : 'false')
conf_data.set('MOD', is_mod ? 'true' : 'false')
conf_data.set('SNAPSHOT', is_snapshot ? 'true' : 'false')
conf_data.set('SNAPSHOT_ID', get_option('snapshot_id'))
conf_data.set('ALLOW_FAKE_NEWER_VERSION', (is_snapshot or is_beta or is_debug or is_mod) ? 'true' : 'false')
conf_data.set('IDENT_PLATFORM', ident_platform)
conf_data.set('IDENT', '@0@-@1@-@2@'.format(host_arch, host_platform, host_libc).to_upper())

update_server = get_option('update_server')
conf_data.set('UPDATESERVER', update_server)
conf_data.set('USE_UPDATESERVER', update_server != '' ? 'true' : 'false')

enforce_https = get_option('enforce_https')
secure_ciphers_only = get_option('secure_ciphers_only')
if not is_debug and not enforce_https
	error('refusing to build a release binary without enforcing HTTPS, configure with -Denforce_https=true to fix this error')
endif
conf_data.set('ENFORCE_HTTPS', enforce_https ? 'true' : 'false')
conf_data.set('SECURE_CIPHERS_ONLY', secure_ciphers_only ? 'true' : 'false')

conf_data.set('IGNORE_UPDATES', get_option('ignore_updates') ? 'true' : 'false')
conf_data.set('SERVER', get_option('server'))
conf_data.set('STATICSERVER', get_option('static_server'))
conf_data.set('APPNAME', get_option('app_name'))
conf_data.set('APPCOMMENT', get_option('app_comment'))
conf_data.set('APPEXE', app_exe)
conf_data.set('APPID', app_id)
conf_data.set('APPDATA', get_option('app_data'))
conf_data.set('APPVENDOR', get_option('app_vendor'))

powder_files = files(
	'SDLCompat.cpp',
	'PowderToySDL.cpp',
	'PowderToy.cpp',
	'lua/CommandInterface.cpp',
	'lua/TPTScriptInterface.cpp',
	'lua/TPTSTypes.cpp',
)
if is_x86
	powder_files += files('X86KillDenormals.cpp')
endif

render_files = files(
	'PowderToyRenderer.cpp',
)

font_files = files(
	'PowderToyFontEditor.cpp',
	'PowderToySDL.cpp',
)

common_files = files(
	'Format.cpp',
	'Misc.cpp',
	'Probability.cpp',
)

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
