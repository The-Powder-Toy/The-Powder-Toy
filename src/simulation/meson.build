simulation_files = files(
	'Air.cpp',
	'Element.cpp',
	'ElementClasses.cpp',
	'GOLString.cpp',
	'Particle.cpp',
	'SaveRenderer.cpp',
	'Sign.cpp',
	'SimulationData.cpp',
	'Simulation.cpp',
)

subdir('elements')
subdir('simtools')
subdir('gravity')

powder_files += simulation_files
render_files += simulation_files

powder_files += files(
	'Editing.cpp',
	'SimTool.cpp',
	'ToolClasses.cpp',
	'Snapshot.cpp',
	'SnapshotDelta.cpp',
)
render_files += files(
	'NoToolClasses.cpp',
)
