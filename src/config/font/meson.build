font_conf_data = conf_data
font_conf_data.set('FONTEDITOR', true)
font_conf_data.set('RENDERER', false)
font_conf_data.set('LUACONSOLE', false)
font_conf_data.set('NOHTTP', true)
font_conf_data.set('GRAVFFT', false)
configure_file(
	input: config_template,
	output: 'Config.h',
	configuration: font_conf_data
)
font_inc = include_directories('.')
