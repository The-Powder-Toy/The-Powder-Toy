simulation_files = files(
	'Air.cpp',
	'Element.cpp',
	'ElementClasses.cpp',
	'GOLString.cpp',
	'Gravity.cpp',
	'Particle.cpp',
	'SaveRenderer.cpp',
	'Sign.cpp',
	'SimTool.cpp',
	'SimulationData.cpp',
	'ToolClasses.cpp',
	'Simulation.cpp',
	'SnapshotDelta.cpp',
)

subdir('elements')
subdir('simtools')

powder_files += simulation_files
render_files += simulation_files

if enable_gravfft
	powder_files += files('FftGravity.cpp')
else
	powder_files += files('PlainGravity.cpp')
endif
render_files += files('PlainGravity.cpp')
