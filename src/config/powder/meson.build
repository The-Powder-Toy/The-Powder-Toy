powder_conf_data = conf_data
powder_conf_data.set('FONTEDITOR', false)
powder_conf_data.set('RENDERER', false)
powder_conf_data.set('NOHTTP', not enable_http)
configure_file(
	input: config_template,
	output: 'Config.h',
	configuration: powder_conf_data
)
powder_inc = include_directories('.')
