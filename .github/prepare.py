import datetime
import json
import os
import re
import subprocess
import sys

ref = os.getenv('GITHUB_REF')
event_name = os.getenv('GITHUB_EVENT_NAME')
publish_hostport = os.getenv('PUBLISH_HOSTPORT')

def set_output(key, value):
	with open(os.getenv('GITHUB_OUTPUT'), 'a') as f:
		f.write(f"{key}={value}\n")

match_stable     = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)', ref)
match_beta       = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)b', ref)
match_snapshot   = re.fullmatch(r'refs/tags/snapshot-([0-9]+)', ref)
match_tptlibsdev = re.fullmatch(r'refs/heads/tptlibsdev-(.*)', ref)
match_alljobs    = re.fullmatch(r'refs/heads/(.*)-alljobs', ref)
do_release       = False
do_priority      = 10
if event_name == 'pull_request':
	do_priority = 0
if match_stable:
	release_type = 'stable'
	release_name = 'v%s.%s.%s' % (match_stable.group(1), match_stable.group(2), match_stable.group(3))
	do_release = True
	do_priority = 0
elif match_beta:
	release_type = 'beta'
	release_name = 'v%s.%s.%sb' % (match_beta.group(1), match_beta.group(2), match_beta.group(3))
	do_release = True
	do_priority = 0
elif match_snapshot:
	release_type = 'snapshot'
	release_name = 'snapshot-%s' % match_snapshot.group(1)
	do_release = True
	do_priority = 0
elif match_tptlibsdev:
	release_type = 'tptlibsdev'
	release_name = 'tptlibsdev-%s' % match_tptlibsdev.group(1)
	do_priority = 0
else:
	release_type = 'dev'
	release_name = 'dev'
	if match_alljobs:
		do_priority = 0
do_publish = publish_hostport and do_release

set_output('release_type', release_type)
set_output('release_name', release_name)

subprocess.run([ 'meson', 'setup', '-Dprepare=true', 'build-prepare' ], check = True)
build_options = {}
with open('build-prepare/meson-info/intro-buildoptions.json') as f:
	for option in json.loads(f.read()):
		build_options[option['name']] = option['value']

if int(build_options['mod_id']) == 0 and os.path.exists('.github/mod_id.txt'):
	with open('.github/mod_id.txt') as f:
		build_options['mod_id'] = f.read()

if int(build_options['mod_id']) == 0:
	if release_type == 'stable':
		pass
	elif release_type == 'beta':
		build_options['app_name'   ] += ' Beta'
		build_options['app_comment'] += ' - Beta'
		build_options['app_exe'    ] += 'beta'
		build_options['app_id'     ] += 'beta'
	elif release_type == 'snapshot':
		build_options['app_name'   ] += ' Snapshot'
		build_options['app_comment'] += ' - Snapshot'
		build_options['app_exe'    ] += 'snapshot'
		build_options['app_id'     ] += 'snapshot'
	else:
		build_options['app_name'   ] += ' Dev'
		build_options['app_comment'] += ' - Dev'
		build_options['app_exe'    ] += 'dev'
		build_options['app_id'     ] += 'dev'

set_output('mod_id'     , build_options['mod_id'     ])
set_output('app_name'   , build_options['app_name'   ])
set_output('app_comment', build_options['app_comment'])
set_output('app_exe'    , build_options['app_exe'    ])
set_output('app_id'     , build_options['app_id'     ])
set_output('app_data'   , build_options['app_data'   ])
set_output('app_vendor' , build_options['app_vendor' ])

app_exe = build_options['app_exe']
app_name = build_options['app_name']
app_name_slug = re.sub('[^A-Za-z0-9]', '_', app_name)

build_matrix = []
publish_matrix = []
# consider disabling line wrapping to edit this monstrosity
for        arch,     platform,         libc,   statdyn, bplatform,         runson, suffix, publish, artifact, dbgsuffix,         mode,             starcatcher,    dbgrel, priority in [
	(  'x86_64',      'linux',        'gnu',  'static',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: static debug build
	(  'x86_64',      'linux',        'gnu',  'static',   'linux', 'ubuntu-20.04',     '',    True,     True,    '.dbg',         None, 'x86_64-lin-gcc-static', 'release',       10 ),
	(  'x86_64',      'linux',        'gnu',  'static',   'linux', 'ubuntu-20.04',     '',   False,     True,    '.dbg',   'appimage',                    None, 'release',        0 ), # priority = 0: appimage release
	(  'x86_64',      'linux',        'gnu', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None,   'debug',       10 ),
	(  'x86_64',      'linux',        'gnu', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,     'nohttp',                    None,   'debug',       10 ),
	(  'x86_64',      'linux',        'gnu', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,      'nolua',                    None,   'debug',       10 ),
	(  'x86_64',      'linux',        'gnu', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None, 'release',       10 ),
#	(  'x86_64',    'windows',      'mingw',  'static',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None,   'debug',       10 ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them; worse, it's a different toolchain
#	(  'x86_64',    'windows',      'mingw',  'static',   'linux', 'ubuntu-20.04',     '',   False,     True,    '.dbg',         None,                    None, 'release',       10 ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them; worse, it's a different toolchain
#	(  'x86_64',    'windows',      'mingw', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None,   'debug',       10 ), # ubuntu-20.04 doesn't have ucrt64-capable mingw >_>
#	(  'x86_64',    'windows',      'mingw', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,         None,                    None, 'release',       10 ), # ubuntu-20.04 doesn't have ucrt64-capable mingw >_>
	(  'x86_64',    'windows',      'mingw',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: static debug build
	(  'x86_64',    'windows',      'mingw',  'static', 'windows', 'windows-2019', '.exe',   False,     True,    '.dbg',         None,                    None, 'release',       10 ),
	(  'x86_64',    'windows',      'mingw', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',       10 ),
	(  'x86_64',    'windows',      'mingw', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None, 'release',       10 ),
	(  'x86_64',    'windows',       'msvc',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: static debug build
	(  'x86_64',    'windows',       'msvc',  'static', 'windows', 'windows-2019', '.exe',    True,     True,    '.pdb',         None,'x86_64-win-msvc-static', 'release',       10 ),
	(  'x86_64',    'windows',       'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',       10 ),
	(  'x86_64',    'windows',       'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,  'backendvs',                    None,   'debug',        0 ), # priority = 0: backend=vs build
	(  'x86_64',    'windows',       'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None, 'release',       10 ),
	(     'x86',    'windows',       'msvc',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: static debug build
	(     'x86',    'windows',       'msvc',  'static', 'windows', 'windows-2019', '.exe',    True,     True,    '.pdb',         None,  'i686-win-msvc-static', 'release',       10 ),
	(     'x86',    'windows',       'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None,   'debug',       10 ),
	(     'x86',    'windows',       'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,         None,                    None, 'release',       10 ),
	(  'x86_64',     'darwin',      'macos',  'static',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None,   'debug',        0 ), # priority = 0: static debug build
	(  'x86_64',     'darwin',      'macos',  'static',  'darwin',     'macos-12', '.dmg',    True,     True,      None,        'dmg', 'x86_64-mac-gcc-static', 'release',       10 ), # I have no idea how to separate debug info on macos
	(  'x86_64',     'darwin',      'macos', 'dynamic',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None,   'debug',       10 ),
	(  'x86_64',     'darwin',      'macos', 'dynamic',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None, 'release',       10 ),
	( 'aarch64',     'darwin',      'macos',  'static',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None,   'debug',        0 ), # priority = 0: static debug build
	( 'aarch64',     'darwin',      'macos',  'static',  'darwin',     'macos-12', '.dmg',    True,     True,      None,        'dmg',  'arm64-mac-gcc-static', 'release',       10 ),
#	( 'aarch64',     'darwin',      'macos', 'dynamic',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None,   'debug',       10 ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
#	( 'aarch64',     'darwin',      'macos', 'dynamic',  'darwin',     'macos-12', '.dmg',   False,    False,      None,        'dmg',                    None, 'release',       10 ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
	(     'x86',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: rarely used debug build
	(     'x86',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',    True,     True,    '.dbg',         None,   'i686-and-gcc-static', 'release',       10 ),
	(  'x86_64',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: rarely used debug build
	(  'x86_64',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',    True,     True,    '.dbg',         None, 'x86_64-and-gcc-static', 'release',       10 ),
	(     'arm',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: rarely used debug build
	(     'arm',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',    True,     True,    '.dbg',         None,    'arm-and-gcc-static', 'release',       10 ),
	( 'aarch64',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: rarely used debug build
	( 'aarch64',    'android',     'bionic',  'static',   'linux', 'ubuntu-20.04', '.apk',    True,     True,    '.dbg',         None,  'arm64-and-gcc-static', 'release',       10 ),
	(  'wasm32', 'emscripten', 'emscripten',  'static',   'linux', 'ubuntu-20.04', '.tar',   False,    False,      None,         None,                    None,   'debug',        0 ), # priority = 0: rarely used debug build
	(  'wasm32', 'emscripten', 'emscripten',  'static',   'linux', 'ubuntu-20.04', '.tar',    True,     True,      None, 'emscripten',     'wasm32-ems-static', 'release',       10 ), # I have no idea how to separate debug info on emscripten
]:
	if priority < do_priority:
		continue
	job_name = f'build'
	if starcatcher:
		job_name += f'+target=starcatcher-{starcatcher}'
	else:
		job_name += f'+target={arch}-{platform}-{libc}-{statdyn}-{dbgrel}'
		if mode:
			job_name += f'+mode={mode}'
		if bplatform != platform:
			job_name += f'+bplatform={bplatform}'
	if not mode:
		mode = 'default'
	separate_debug = True
	if not dbgsuffix:
		dbgsuffix = 'BOGUS'
		separate_debug = False
	if not starcatcher:
		starcatcher = 'BOGUS'
	if publish:
		assert artifact
	if dbgrel != 'release':
		assert not publish
		assert not artifact
	asset_path = f'{app_exe}{suffix}'
	asset_name = f'{app_exe}-{release_name}-{arch}-{platform}-{libc}{suffix}'
	debug_asset_path = f'{app_exe}{dbgsuffix}'
	debug_asset_name = f'{app_exe}-{release_name}-{arch}-{platform}-{libc}{dbgsuffix}'
	if mode	== 'appimage':
		asset_path = f'{app_name_slug}-{arch}.AppImage'
		asset_name = f'{app_name_slug}-{arch}.AppImage'
		debug_asset_path = f'{app_name_slug}-{arch}.AppImage.dbg'
		debug_asset_name = f'{app_name_slug}-{arch}.AppImage.dbg'
	starcatcher_name = f'powder-{release_name}-{starcatcher}{suffix}'
	msys2_bash = (bplatform == 'windows' and libc == 'mingw')
	shell = 'bash'
	if msys2_bash:
		shell = 'msys2 {0}'
	build_matrix.append({
		'bsh_build_platform': bplatform, # part of the unique portion of the matrix
		'bsh_host_arch': arch, # part of the unique portion of the matrix
		'bsh_host_platform': platform, # part of the unique portion of the matrix
		'bsh_host_libc': libc, # part of the unique portion of the matrix
		'bsh_static_dynamic': statdyn, # part of the unique portion of the matrix
		'bsh_debug_release': dbgrel, # part of the unique portion of the matrix
		'runs_on': runson,
		'force_msys2_bash': msys2_bash and 'yes' or 'no',
		'package_suffix': suffix,
		'package_mode': mode,
		'publish': publish and 'yes' or 'no',
		'artifact': artifact and 'yes' or 'no',
		'separate_debug': separate_debug and 'yes' or 'no',
		'asset_path': asset_path,
		'asset_name': asset_name,
		'debug_asset_path': debug_asset_path,
		'debug_asset_name': debug_asset_name,
		'job_name': job_name,
		'shell': shell,
	})
	if publish:
		publish_matrix.append({
			'bsh_build_platform': bplatform, # part of the unique portion of the matrix
			'bsh_host_arch': arch, # part of the unique portion of the matrix
			'bsh_host_platform': platform, # part of the unique portion of the matrix
			'bsh_host_libc': libc, # part of the unique portion of the matrix
			'bsh_static_dynamic': statdyn, # part of the unique portion of the matrix
			'asset_path': asset_path,
			'asset_name': asset_name,
			'starcatcher_name': starcatcher_name,
		})

set_output('build_matrix', json.dumps({ 'include': build_matrix }))
set_output('publish_matrix', json.dumps({ 'include': publish_matrix }))
set_output('do_release', do_release and 'yes' or 'no')
set_output('do_publish', do_publish and 'yes' or 'no')
