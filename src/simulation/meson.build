simulation_files = files(
	'Air.cpp',
	'AccessProperty.cpp',
	'Element.cpp',
	'ElementClasses.cpp',
	'GOLString.cpp',
	'Particle.cpp',
	'SaveRenderer.cpp',
	'Sign.cpp',
	'SimulationData.cpp',
	'Simulation.cpp',
	'StructProperty.cpp',
)

subdir('elements')
subdir('simtools')
subdir('gravity')

powder_source_files += simulation_files
render_files += simulation_files

powder_source_files += files(
	'AccessPropertyParse.cpp',
	'Editing.cpp',
	'SimTool.cpp',
	'ToolClasses.cpp',
	'Snapshot.cpp',
	'SnapshotDelta.cpp',
)
