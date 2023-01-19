configure_file(
	input: 'Config.template.h',
	output: 'Config.h',
	configuration: conf_data
)

powder_files = files(
	'SDLCompat.cpp',
	'PowderToySDL.cpp',
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
else
	powder_files += files(
		'lua/PlainCommandInterface.cpp',
	)
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
