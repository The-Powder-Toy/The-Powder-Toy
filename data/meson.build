to_array = generator(
	import('python').find_installation('python3'),
	output: [ '@BASENAME@.cpp', '@BASENAME@.h' ],
	arguments: [ join_paths(meson.current_source_dir(), 'to_array.py'), '@BUILD_DIR@', '@OUTPUT0@', '@OUTPUT1@', '@INPUT@', '@EXTRA_ARGS@' ]
)

data_files = files(
	'hmap.cpp',
	'icon.cpp',
	'images.cpp',
)
data_files += to_array.process('font.bz2', extra_args: 'compressed_font_data')

powder_files += data_files
render_files += data_files
font_files += data_files
