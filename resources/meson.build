resources_files += files(
	'icon.ico',
	'document.ico',
)

if copt_platform == 'windows'
	powder_files += windows_mod.compile_resources(
		'powder-res.rc',
		depend_files: resources_files,
	)
endif
