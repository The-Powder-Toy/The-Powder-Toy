resources_files += files(
	'icon.ico',
	'document.ico',
)

if host_platform == 'windows'
	powder_files += windows_mod.compile_resources(
		'powder-res.rc',
		depend_files: [ resources_files, 'resource.h' ],
	)
endif
