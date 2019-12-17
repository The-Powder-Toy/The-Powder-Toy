project('tpt-bzip2', 'c')

subdir('src')

bzip2_inc = include_directories('include')
bzip2_sta = static_library(
	'bzip2',
	sources: bzip2_src,
	include_directories: bzip2_inc,
)
bzip2_dep = declare_dependency(
	link_with: bzip2_sta,
	include_directories: bzip2_inc,
)
