rendered_icons_optionally_from_svg = {
	'icon_exe'   : { 'png': 'icon_exe.png'   , 'svg': 'icon_exe.svg', 'width': '256', 'height': '256' },
	'icon_exe_48': { 'png': 'icon_exe_48.png', 'svg': 'icon_exe.svg', 'width':  '48', 'height':  '48' },
	'icon_exe_32': { 'png': 'icon_exe_32.png', 'svg': 'icon_exe.svg', 'width':  '32', 'height':  '32' },
	'icon_exe_16': { 'png': 'icon_exe_16.png', 'svg': 'icon_exe.svg', 'width':  '16', 'height':  '16' },
	'icon_cps'   : { 'png': 'icon_cps.png'   , 'svg': 'icon_cps.svg', 'width': '256', 'height': '256' },
	'icon_cps_48': { 'png': 'icon_cps_48.png', 'svg': 'icon_cps.svg', 'width':  '48', 'height':  '48' },
	'icon_cps_32': { 'png': 'icon_cps_32.png', 'svg': 'icon_cps.svg', 'width':  '32', 'height':  '32' },
	'icon_cps_16': { 'png': 'icon_cps_16.png', 'svg': 'icon_cps.svg', 'width':  '16', 'height':  '16' },
}
rendered_icons = {}
if render_icons_with_inkscape.allowed()
	foreach key, info : rendered_icons_optionally_from_svg
		rendered_icons += { key: custom_target(
			key + '-from-svg',
			output: info['png'],
			command: [
				inkscape,
				'--export-area-page',
				'--export-type', 'png',
				'--export-filename', '@OUTPUT@',
				'--export-width', info['width'],
				'--export-height', info['height'],
				files(info['svg']),
			],
		) }
	endforeach
else
	foreach key, info : rendered_icons_optionally_from_svg
		rendered_icons += { key: files('generated_icons/' + info['png']) }
	endforeach
endif

if host_platform == 'windows'
	windows_icons = get_option('windows_icons')
	windows_utf8cp = get_option('windows_utf8cp')
	rc_conf_depends = []
	rc_conf_depend_files = [
		'resource.h',
	]
	icon_exe_ico_path = ''
	icon_cps_ico_path = ''
	winutf8_xml_path = ''
	if windows_icons
		make_ico = executable('makeico', sources: 'MakeIco.cpp', native: true)
		generated_win_icos = {}
		win_icos = {
			'icon_exe': [ 'icon_exe', 'icon_exe_48', 'icon_exe_32', 'icon_exe_16' ],
			'icon_cps': [ 'icon_cps', 'icon_cps_48', 'icon_cps_32', 'icon_cps_16' ],
		}
		foreach key, icons : win_icos
			command = [
				make_ico,
				'@OUTPUT@',
			]
			foreach ikey : icons
				command += [ rendered_icons[ikey] ]
			endforeach
			generated_win_icos += { key: custom_target(
				key + '-ico',
				output: key + '.ico',
				command: command,
			) }
		endforeach
		rc_conf_depends += [
			generated_win_icos['icon_exe'],
			generated_win_icos['icon_cps'],
		]
		icon_exe_ico_path = join_paths(meson.current_build_dir(), 'icon_exe.ico')
		icon_cps_ico_path = join_paths(meson.current_build_dir(), 'icon_cps.ico')
	endif
	if windows_utf8cp
		rc_conf_depend_files += [
			'winutf8.xml',
		]
		winutf8_xml_path = join_paths(meson.current_source_dir(), 'winutf8.xml')
	endif
	rc_conf_data = configuration_data()
	rc_conf_data.merge_from(conf_data)
	rc_conf_data.set('HAVE_ICONS', windows_icons ? 1 : 0)
	rc_conf_data.set('HAVE_UTF8CP', windows_utf8cp ? 1 : 0)
	rc_conf_data.set('RESOUCE_H', join_paths(meson.current_source_dir(), 'resource.h'))
	rc_conf_data.set('WINUTF8_XML', winutf8_xml_path)
	rc_conf_data.set('ICON_EXE_ICO', icon_exe_ico_path)
	rc_conf_data.set('ICON_CPS_ICO', icon_cps_ico_path)
	powder_files += windows_mod.compile_resources(
		configure_file(
			input: 'powder-res.template.rc',
			output: 'powder-res.rc',
			configuration: rc_conf_data,
		),
		depends: rc_conf_depends,
		depend_files: rc_conf_depend_files,
	)
elif host_platform == 'darwin'
	configure_file(
		input: 'Info.template.plist',
		output: 'Info.plist',
		configuration: conf_data,
	)
elif host_platform == 'linux'
	data_files += to_array.process(rendered_icons['icon_exe'], extra_args: 'icon_exe_png')
	data_files += to_array.process(rendered_icons['icon_cps'], extra_args: 'icon_cps_png')
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

data_files += to_array.process('save_local.png', extra_args: 'save_local_png')
data_files += to_array.process('save_online.png', extra_args: 'save_online_png')
data_files += to_array.process('font.bz2', extra_args: 'compressed_font_data')
