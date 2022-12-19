import datetime
import json
import os
import re
import subprocess
import sys

ref = os.getenv('GITHUB_REF')
publish_hostport = os.getenv('PUBLISH_HOSTPORT')

def set_output(key, value):
	with open(os.getenv('GITHUB_OUTPUT'), 'a') as f:
		f.write(f"{key}={value}\n")

match_stable     = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)', ref)
match_beta       = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)b', ref)
match_snapshot   = re.fullmatch(r'refs/tags/snapshot-([0-9]+)', ref)
match_tptlibsdev = re.fullmatch(r'refs/heads/tptlibsdev-(.*)', ref)
do_release       = False
if match_stable:
	release_type = 'stable'
	release_name = 'v%s.%s.%s' % (match_stable.group(1), match_stable.group(2), match_stable.group(3))
	do_release = True
elif match_beta:
	release_type = 'beta'
	release_name = 'v%s.%s.%sb' % (match_beta.group(1), match_beta.group(2), match_beta.group(3))
	do_release = True
elif match_snapshot:
	release_type = 'snapshot'
	release_name = 'snapshot-%s' % match_snapshot.group(1)
	do_release = True
elif match_tptlibsdev:
	release_type = 'tptlibsdev'
	release_name = 'tptlibsdev-%s' % match_tptlibsdev.group(1)
else:
	release_type = 'dev'
	release_name = 'dev'
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
for        arch,  platform,     libc,   statdyn, bplatform,         runson, suffix, publish, artifact, dbgsuffix,       mode,             starcatcher,    dbgrel in [
	(  'x86_64',   'linux',    'gnu',  'static',   'linux', 'ubuntu-18.04',     '',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64',   'linux',    'gnu',  'static',   'linux', 'ubuntu-18.04',     '',    True,     True,    '.dbg',       None, 'x86_64-lin-gcc-static', 'release' ),
	(  'x86_64',   'linux',    'gnu',  'static',   'linux', 'ubuntu-18.04',     '',   False,     True,    '.dbg', 'appimage',                    None, 'release' ),
	(  'x86_64',   'linux',    'gnu', 'dynamic',   'linux', 'ubuntu-18.04',     '',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64',   'linux',    'gnu', 'dynamic',   'linux', 'ubuntu-18.04',     '',   False,    False,      None,       None,                    None, 'release' ),
#	(  'x86_64', 'windows',  'mingw',  'static',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,       None,                    None,   'debug' ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them
#	(  'x86_64', 'windows',  'mingw',  'static',   'linux', 'ubuntu-20.04',     '',   False,     True,    '.dbg',       None,                    None, 'release' ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them
	(  'x86_64', 'windows',  'mingw', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'windows',  'mingw', 'dynamic',   'linux', 'ubuntu-20.04',     '',   False,    False,      None,       None,                    None, 'release' ),
	(  'x86_64', 'windows',  'mingw',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'windows',  'mingw',  'static', 'windows', 'windows-2019', '.exe',   False,     True,    '.dbg',       None,                    None, 'release' ),
	(  'x86_64', 'windows',  'mingw', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'windows',  'mingw', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None, 'release' ),
	(  'x86_64', 'windows',   'msvc',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'windows',   'msvc',  'static', 'windows', 'windows-2019', '.exe',    True,     True,    '.pdb',       None,'x86_64-win-msvc-static', 'release' ),
	(  'x86_64', 'windows',   'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'windows',   'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None, 'release' ),
	(     'x86', 'windows',   'msvc',  'static', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(     'x86', 'windows',   'msvc',  'static', 'windows', 'windows-2019', '.exe',    True,     True,    '.pdb',       None,  'i686-win-msvc-static', 'release' ),
	(     'x86', 'windows',   'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None,   'debug' ),
	(     'x86', 'windows',   'msvc', 'dynamic', 'windows', 'windows-2019', '.exe',   False,    False,      None,       None,                    None, 'release' ),
	(  'x86_64',  'darwin',  'macos',  'static',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None,   'debug' ),
	(  'x86_64',  'darwin',  'macos',  'static',  'darwin',   'macos-11.0', '.dmg',    True,     True,      None,      'dmg', 'x86_64-mac-gcc-static', 'release' ), # I have no idea how to separate debug info on macos
	(  'x86_64',  'darwin',  'macos', 'dynamic',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None,   'debug' ),
	(  'x86_64',  'darwin',  'macos', 'dynamic',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None, 'release' ),
	( 'aarch64',  'darwin',  'macos',  'static',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None,   'debug' ),
	( 'aarch64',  'darwin',  'macos',  'static',  'darwin',   'macos-11.0', '.dmg',    True,     True,      None,      'dmg',  'arm64-mac-gcc-static', 'release' ),
#	( 'aarch64',  'darwin',  'macos', 'dynamic',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None,   'debug' ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
#	( 'aarch64',  'darwin',  'macos', 'dynamic',  'darwin',   'macos-11.0', '.dmg',   False,    False,      None,      'dmg',                    None, 'release' ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
	(     'x86', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',   False,    False,      None,       None,                    None,   'debug' ),
	(     'x86', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',    True,     True,    '.dbg',       None,   'i686-and-gcc-static', 'release' ),
	(  'x86_64', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',   False,    False,      None,       None,                    None,   'debug' ),
	(  'x86_64', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',    True,     True,    '.dbg',       None, 'x86_64-and-gcc-static', 'release' ),
	(     'arm', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',   False,    False,      None,       None,                    None,   'debug' ),
	(     'arm', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',    True,     True,    '.dbg',       None,    'arm-and-gcc-static', 'release' ),
	( 'aarch64', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',   False,    False,      None,       None,                    None,   'debug' ),
	( 'aarch64', 'android', 'bionic',  'static',   'linux', 'ubuntu-18.04', '.apk',    True,     True,    '.dbg',       None,  'arm64-and-gcc-static', 'release' ),
]:
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
	build_matrix.append({
		'bsh_build_platform': bplatform, # part of the unique portion of the matrix
		'bsh_host_arch': arch, # part of the unique portion of the matrix
		'bsh_host_platform': platform, # part of the unique portion of the matrix
		'bsh_host_libc': libc, # part of the unique portion of the matrix
		'bsh_static_dynamic': statdyn, # part of the unique portion of the matrix
		'bsh_debug_release': dbgrel, # part of the unique portion of the matrix
		'runs_on': runson,
		'package_suffix': suffix,
		'package_mode': mode,
		'publish': publish and 'yes' or 'no',
		'artifact': artifact and 'yes' or 'no',
		'separate_debug': separate_debug and 'yes' or 'no',
		'asset_path': asset_path,
		'asset_name': asset_name,
		'debug_asset_path': debug_asset_path,
		'debug_asset_name': debug_asset_name,
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
