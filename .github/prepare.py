import datetime
import json
import os
import re
import sys

ref = os.getenv('GITHUB_REF')
publish_hostport = os.getenv('PUBLISH_HOSTPORT')

match_stable     = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)', ref)
match_beta       = re.fullmatch(r'refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)b', ref)
match_snapshot   = re.fullmatch(r'refs/tags/snapshot-([0-9]+)', ref)
match_tptlibsdev = re.fullmatch(r'refs/heads/tptlibsdev-(.*)', ref)
if match_stable:
	release_type = 'stable'
	release_name = 'v%s.%s.%s' % (match_stable.group(1), match_stable.group(2), match_stable.group(3))
elif match_beta:
	release_type = 'beta'
	release_name = 'v%s.%s.%sb' % (match_beta.group(1), match_beta.group(2), match_beta.group(3))
elif match_snapshot:
	release_type = 'snapshot'
	release_name = 'snapshot-%s' % match_snapshot.group(1)
elif match_tptlibsdev:
	release_type = 'tptlibsdev'
	release_name = 'tptlibsdev-%s' % match_tptlibsdev.group(1)
else:
	release_type = 'dev'
	release_name = 'dev'

print('::set-output name=release_type::' + release_type)
print('::set-output name=release_name::' + release_name)

with open('.github/mod_id.txt') as f:
	print('::set-output name=mod_id::' + f.read())

build_matrix = []
publish_matrix = []
for bsh_host_arch, bsh_host_platform, bsh_host_libc, bsh_static_dynamic, bsh_build_platform,        runs_on, package_suffix, publish, artifact, debug_suffix,         starcatcher_name in [
	(    'x86_64',           'linux',         'gnu',           'static',            'linux', 'ubuntu-18.04',             '',    True,     True,       '.dbg',  'x86_64-lin-gcc-static' ),
	(    'x86_64',           'linux',         'gnu',          'dynamic',            'linux', 'ubuntu-18.04',             '',   False,    False,         None,                       '' ),
#	(    'x86_64',         'windows',       'mingw',           'static',            'linux', 'ubuntu-20.04',             '',   False,     True,       '.dbg',                       '' ), # ubuntu-20.04 doesn't have windows TLS headers somehow and I haven't yet figured out how to get them
	(    'x86_64',         'windows',       'mingw',          'dynamic',            'linux', 'ubuntu-20.04',             '',   False,    False,         None,                       '' ),
	(    'x86_64',         'windows',       'mingw',           'static',          'windows', 'windows-2019',         '.exe',   False,     True,       '.dbg',                       '' ),
	(    'x86_64',         'windows',       'mingw',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '' ),
	(    'x86_64',         'windows',        'msvc',           'static',          'windows', 'windows-2019',         '.exe',    True,     True,       '.pdb', 'x86_64-win-msvc-static' ),
	(    'x86_64',         'windows',        'msvc',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '' ),
	(       'x86',         'windows',        'msvc',           'static',          'windows', 'windows-2019',         '.exe',    True,     True,       '.pdb',   'i686-win-msvc-static' ),
	(       'x86',         'windows',        'msvc',          'dynamic',          'windows', 'windows-2019',         '.exe',   False,    False,         None,                       '' ),
	(    'x86_64',          'darwin',       'macos',           'static',           'darwin',   'macos-11.0',             '',    True,     True,         None,  'x86_64-mac-gcc-static' ), # I have no idea how to separate debug info on macos
	(    'x86_64',          'darwin',       'macos',          'dynamic',           'darwin',   'macos-11.0',             '',   False,    False,         None,                       '' ),
	(   'aarch64',          'darwin',       'macos',           'static',           'darwin',   'macos-11.0',             '',    True,     True,         None,   'arm64-mac-gcc-static' ),
#	(   'aarch64',          'darwin',       'macos',          'dynamic',           'darwin',   'macos-11.0',             '',   False,    False,         None,                       '' ), # macos-11.0 is x86_64 and I haven't yet figured out how to get homebrew to install aarch64 libs on x86_64
	(       'x86',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',    'i686-and-gcc-static' ),
	(    'x86_64',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',  'x86_64-and-gcc-static' ),
	(       'arm',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',     'arm-and-gcc-static' ),
	(   'aarch64',         'android',      'bionic',           'static',            'linux', 'ubuntu-18.04',         '.apk',    True,     True,       '.dbg',   'arm64-and-gcc-static' ),
]:
	separate_debug = True
	if not debug_suffix:
		debug_suffix = ''
		separate_debug = False
	if publish:
		assert artifact
	for debug_release in [ 'debug', 'release' ]:
		publish_release = publish and debug_release == 'release'
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
			'artifact': artifact and 'yes' or 'no',
			'separate_debug': separate_debug and 'yes' or 'no',
			'asset_path': f'powder{package_suffix}',
			'asset_name': f'powder-{release_name}-{bsh_host_arch}-{bsh_host_platform}-{bsh_host_libc}{package_suffix}',
			'debug_asset_path': f'powder{debug_suffix}',
			'debug_asset_name': f'powder-{release_name}-{bsh_host_arch}-{bsh_host_platform}-{bsh_host_libc}{debug_suffix}',
		})
		if publish_release:
			publish_matrix.append({
				'bsh_build_platform': bsh_build_platform, # part of the unique portion of the matrix
				'bsh_host_arch': bsh_host_arch, # part of the unique portion of the matrix
				'bsh_host_platform': bsh_host_platform, # part of the unique portion of the matrix
				'bsh_host_libc': bsh_host_libc, # part of the unique portion of the matrix
				'bsh_static_dynamic': bsh_static_dynamic, # part of the unique portion of the matrix
				'asset_path': f'powder{package_suffix}',
				'asset_name': f'powder-{release_name}-{bsh_host_arch}-{bsh_host_platform}-{bsh_host_libc}{package_suffix}',
				'starcatcher_name': f'powder-{release_name}-{starcatcher_name}{package_suffix}',
			})

print('::set-output name=build_matrix::' + json.dumps({ 'include': build_matrix }))
print('::set-output name=publish_matrix::' + json.dumps({ 'include': publish_matrix }))
print('::set-output name=do_release::' + (ref.startswith('refs/tags/v') and 'yes' or 'no'))
print('::set-output name=do_publish::' + ((publish_hostport and (match_stable or match_beta or match_snapshot)) and 'yes' or 'no'))
