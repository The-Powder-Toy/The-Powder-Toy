config_template = files('Config.template.h')
subdir('config')

powder_files = files(
	'SDLCompat.cpp',
	'Update.cpp',
	'PowderToySDL.cpp',
	'lua/CommandInterface.cpp',
	'lua/LuaEvents.cpp',
	'lua/TPTScriptInterface.cpp',
	'lua/TPTSTypes.cpp',
)

render_files = files(
	'PowderToyRenderer.cpp',
)

font_files = files(
	'PowderToyFontEditor.cpp',
)

common_files = files(
	'Format.cpp',
	'Misc.cpp',
	'Platform.cpp',
	'Probability.cpp',
)

subdir('bson')
subdir('bzip2')
subdir('client')
subdir('common')
subdir('debug')
subdir('graphics')
subdir('gui')
subdir('json')
if uopt_lua != 'none'
	subdir('lua')
endif
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
