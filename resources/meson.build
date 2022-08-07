if host_platform == 'windows'
	powder_files += windows_mod.compile_resources(
		'powder-res.rc',
		depend_files: [
			'icon.ico',
			'document.ico',
			'resource.h',
		],
	)
endif

if host_platform == 'linux'
	data_files += to_array.process('cps16.png', extra_args: 'cps16_png')
	data_files += to_array.process('cps32.png', extra_args: 'cps32_png')
	data_files += to_array.process('exe48.png', extra_args: 'exe48_png')
	data_files += to_array.process('icon/powder-128.png', extra_args: 'icon_png')
	data_files += to_array.process('save.xml', extra_args: 'save_xml')
	data_files += to_array.process(configure_file(
		input: 'powder.template.desktop',
		output: 'powder.desktop',
		configuration: conf_data,
	), extra_args: 'powder_desktop')

	configure_file(
		input: 'appdata.template.xml',
		output: 'appdata.xml',
		configuration: conf_data,
	)
endif
