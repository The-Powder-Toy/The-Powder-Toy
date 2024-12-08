simulation_tool_names = [
	'HEAT',
	'COOL',
	'AIR',
	'VAC',
	'PGRV',
	'NGRV',
	'MIX',
	'CYCL',
	'AMBM',
	'AMBP',
	'WIND',
]

simulation_tool_src = []
simulation_tool_ids = []
tool_id = 0
foreach tool_name : simulation_tool_names
	if not is_disabler(tool_name)
		simulation_tool_src += tool_name + '.cpp'
		simulation_tool_ids += [ [ tool_name, tool_id ] ]
	endif
	tool_id = tool_id + 1
endforeach
simulation_files += files(simulation_tool_src)
