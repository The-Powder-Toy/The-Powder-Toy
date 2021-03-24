resources_files += files(
	'icon.ico',
	'document.ico',
)

if copt_platform == 'win'
	powder_files += windows_mod.compile_resources(
		'powder-res.rc',
		depend_files: [ resources_files, 'resource.h' ],
	)
endif
