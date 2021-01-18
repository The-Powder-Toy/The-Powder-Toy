render_conf_data = conf_data
render_conf_data.set('FONTEDITOR', false)
render_conf_data.set('RENDERER', true)
render_conf_data.set('LUACONSOLE', false)
render_conf_data.set('NOHTTP', true)
render_conf_data.set('GRAVFFT', false)
configure_file(
	input: config_template,
	output: 'Config.h',
	configuration: render_conf_data
)
render_inc = include_directories('.')
