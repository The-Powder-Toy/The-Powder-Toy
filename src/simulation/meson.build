simulation_files = files(
	'Air.cpp',
	'Element.cpp',
	'ElementClasses.cpp',
	'GOLString.cpp',
	'Gravity.cpp',
	'Particle.cpp',
	'SaveRenderer.cpp',
	'Sign.cpp',
	'SimulationData.cpp',
	'Simulation.cpp',
)

subdir('elements')
subdir('simtools')

powder_files += simulation_files
render_files += simulation_files

powder_files += files(
	'Editing.cpp',
	'SimTool.cpp',
	'ToolClasses.cpp',
	'SnapshotDelta.cpp',
)
render_files += files(
	'NoToolClasses.cpp',
)
if enable_gravfft
	powder_files += files('FftGravity.cpp')
else
	powder_files += files('PlainGravity.cpp')
endif
conf_data.set('GRAVFFT', enable_gravfft ? 'true' : 'false')
render_files += files('PlainGravity.cpp')
