android_manifest_xml = configure_file(
	input: 'AndroidManifest.template.xml',
	output: 'AndroidManifest.xml',
	configuration: conf_data,
)
powder_jar_sources = configure_file(
	input: 'PowderActivity.template.java',
	output: 'PowderActivity.java',
	configuration: conf_data,
)

subdir('res')

python3 = find_program('python3')
android_platform_jar = meson.get_external_property('android_platform_jar')
java_runtime_jar = meson.get_external_property('java_runtime_jar')
android_keystore = get_option('android_keystore')
sdl_jar = tpt_libs.get_variable('sdl_jar')
powder_jar = custom_target(
	'powder-jar',
	output: app_exe + '.jar',
	command: [
		python3,
		files('powder-jar.py'),
		find_program('javac'),
		find_program('jar'),
		meson.current_source_dir(),
		meson.project_build_root(),
		'@PRIVATE_DIR@',
		'@OUTPUT@',
		android_platform_jar,
		java_runtime_jar,
		sdl_jar,
		is_debug ? 'debug' : 'release',
		powder_jar_sources,
	],
)
unaligned_apk = custom_target(
	'build-apk',
	output: app_exe + '.unaligned.apk',
	input: powder_sha,
	command: [
		python3,
		files('build-apk.py'),
		find_program('d8'),
		find_program('aapt'),
		find_program('aapt2'),
		meson.current_source_dir(),
		meson.project_build_root(),
		'@PRIVATE_DIR@',
		'@OUTPUT@',
		'@INPUT@',
		android_platform_jar,
		sdl_jar,
		powder_jar,
		host_arch,
		is_debug ? 'debug' : 'release',
		android_manifest_xml,
		android_resources,
	],
)
unsigned_apk = custom_target(
	'align-apk',
	output: app_exe + '.unsigned.apk',
	input: unaligned_apk,
	command: [
		python3,
		files('align-apk.py'),
		find_program('zipalign'),
		meson.project_build_root(),
		'@OUTPUT@',
		'@INPUT@',
	],
)
signed_apk = custom_target(
	'sign-apk',
	output: app_exe + '.apk',
	input: unsigned_apk,
	command: [
		python3,
		files('sign-apk.py'),
		find_program('apksigner'),
		meson.project_build_root(),
		'@OUTPUT@',
		'@INPUT@',
		android_keystore,
		get_option('android_keyalias'),
	],
)
adb = find_program('adb', required: false)
if adb.found()
	install_apk = custom_target(
		'install-apk',
		input: signed_apk,
		output: 'install-apk',
		command: [ python3, files('install-apk.py'), adb, meson.project_build_root(), '@OUTPUT@', '@INPUT@' ],
	)
	custom_target(
		'run-apk',
		input: [ signed_apk, install_apk ],
		build_always_stale: true,
		output: 'run-apk',
		command: [ python3, files('run-apk.py'), adb, meson.project_build_root(), '@OUTPUT@', '@INPUT0@', app_id ],
	)
endif
