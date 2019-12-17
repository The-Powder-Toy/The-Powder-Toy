simulation_tool_ids = [
	[ 'HEAT', 0 ],
	[ 'COOL', 1 ],
	[ 'AIR',  2 ],
	[ 'VAC',  3 ],
	[ 'PGRV', 4 ],
	[ 'NGRV', 5 ],
	[ 'MIX',  6 ],
	[ 'CYCL', 7 ],
]

simulation_tool_src = []
foreach tool_name_id : simulation_tool_ids
	simulation_tool_src += tool_name_id[0] + '.cpp'
endforeach
simulation_files += files(simulation_tool_src)
