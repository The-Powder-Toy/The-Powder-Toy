import datetime
import json
import os
import re
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

with open('.github/mod_id.txt') as f:
	set_output('mod_id', f.read())

build_matrix = []
publish_matrix = []
# consider disabling line wrapping to edit this monstrosity
for bsh_host_arch, bsh_host_platform, bsh_host_libc, bsh_static_dynamic, bsh_build_platform,        runs_on, package_suffix, publish, artifact, debug_suffix,       starcatcher_suffix, build_package in [
	(    'x86_64',           'linux',         'gnu',           'static',            'linux', 'ubuntu-18.04',             '',    True,     True,       '.dbg',  'x86_64-lin-gcc-static',          True ),
	(    'x86_64',           'linux',         'gnu',          'dynamic',            'linux', 'ubuntu-18.04',             '',   False,    False,         None,                       '',         False ),
#	(    'x86_64',         'windows',       'mingw',           'static',            'linux', 'ubuntu-20.04',             '',   False,     True,       '.dbg',                       '',         False ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them
	(    'x86_64',         'windows',       'mingw',          'dynamic',            'linux', 'ubuntu-20.04',             '',   False,    False,         None,                       '',         False ),
	(    'x86_64',         'windows',       'mingw',           'static',          'windows', 'windows-2019',         '.exe',   False,     True,       '.dbg',                       '',         False ),
	(    'x86_64',         'windows',       'mingw',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '',         False ),
	(    'x86_64',         'windows',        'msvc',           'static',          'windows', 'windows-2019',         '.exe',    True,     True,       '.pdb', 'x86_64-win-msvc-static',         False ),
	(    'x86_64',         'windows',        'msvc',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '',         False ),
	(       'x86',         'windows',        'msvc',           'static',          'windows', 'windows-2019',         '.exe',    True,     True,       '.pdb',   'i686-win-msvc-static',         False ),
	(       'x86',         'windows',        'msvc',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '',         False ),
	(    'x86_64',          'darwin',       'macos',           'static',           'darwin',   'macos-11.0',             '',    True,     True,         None,  'x86_64-mac-gcc-static',         False ), # I have no idea how to separate debug info on macos
	(    'x86_64',          'darwin',       'macos',          'dynamic',           'darwin',   'macos-11.0',             '',   False,    False,         None,                       '',         False ),
	(   'aarch64',          'darwin',       'macos',           'static',           'darwin',   'macos-11.0',             '',    True,     True,         None,   'arm64-mac-gcc-static',         False ),
#	(   'aarch64',          'darwin',       'macos',          'dynamic',           'darwin',   'macos-11.0',             '',   False,    False,         None,                       '',         False ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
	(       'x86',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',    'i686-and-gcc-static',         False ),
	(    'x86_64',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',  'x86_64-and-gcc-static',         False ),
	(       'arm',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',     'arm-and-gcc-static',         False ),
	(   'aarch64',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',   'arm64-and-gcc-static',         False ),
]:
	separate_debug = True
	if not debug_suffix:
		debug_suffix = ''
		separate_debug = False
	if publish:
		assert artifact
	for debug_release in [ 'debug', 'release' ]:
		publish_release = publish and debug_release == 'release'
		artifact_release = artifact and debug_release == 'release'
		asset_path = f'powder{package_suffix}'
		asset_name = f'powder-{release_name}-{bsh_host_arch}-{bsh_host_platform}-{bsh_host_libc}{package_suffix}'
		debug_asset_path = f'powder{debug_suffix}'
		debug_asset_name = f'powder-{release_name}-{bsh_host_arch}-{bsh_host_platform}-{bsh_host_libc}{debug_suffix}'
		package_asset_path = f'The_Powder_Toy-{bsh_host_arch}.AppImage'
		package_asset_name = f'The_Powder_Toy-{bsh_host_arch}.AppImage'
		package_debug_asset_path = f'The_Powder_Toy-{bsh_host_arch}.AppImage.dbg'
		package_debug_asset_name = f'The_Powder_Toy-{bsh_host_arch}.AppImage.dbg'
		starcatcher_name = f'powder-{release_name}-{starcatcher_suffix}{package_suffix}'
		build_matrix.append({
			'bsh_build_platform': bsh_build_platform, # part of the unique portion of the matrix
			'bsh_host_arch': bsh_host_arch, # part of the unique portion of the matrix
			'bsh_host_platform': bsh_host_platform, # part of the unique portion of the matrix
			'bsh_host_libc': bsh_host_libc, # part of the unique portion of the matrix
			'bsh_static_dynamic': bsh_static_dynamic, # part of the unique portion of the matrix
			'bsh_debug_release': debug_release, # part of the unique portion of the matrix
			'runs_on': runs_on,
			'package_suffix': package_suffix,
			'publish': publish_release and 'yes' or 'no',
			'artifact': artifact_release and 'yes' or 'no',
			'separate_debug': separate_debug and 'yes' or 'no',
			'build_package': build_package and 'yes' or 'no',
			'asset_path': asset_path,
			'asset_name': asset_name,
			'debug_asset_path': debug_asset_path,
			'debug_asset_name': debug_asset_name,
			'package_asset_path': package_asset_path,
			'package_asset_name': package_asset_name,
			'package_debug_asset_path': package_debug_asset_path,
			'package_debug_asset_name': package_debug_asset_name,
		})
		if publish_release:
			publish_matrix.append({
				'bsh_build_platform': bsh_build_platform, # part of the unique portion of the matrix
				'bsh_host_arch': bsh_host_arch, # part of the unique portion of the matrix
				'bsh_host_platform': bsh_host_platform, # part of the unique portion of the matrix
				'bsh_host_libc': bsh_host_libc, # part of the unique portion of the matrix
				'bsh_static_dynamic': bsh_static_dynamic, # part of the unique portion of the matrix
				'asset_path': asset_path,
				'asset_name': asset_name,
				'starcatcher_name': starcatcher_name,
			})

set_output('build_matrix', json.dumps({ 'include': build_matrix }))
set_output('publish_matrix', json.dumps({ 'include': publish_matrix }))
set_output('do_release', do_release and 'yes' or 'no')
set_output('do_publish', do_publish and 'yes' or 'no')
