font_conf_data = conf_data
font_conf_data.set('FONTEDITOR', true)
font_conf_data.set('RENDERER', false)
configure_file(
	input: config_template,
	output: 'Config.h',
	configuration: font_conf_data
)
font_inc = include_directories('.')
