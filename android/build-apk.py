import os
import os.path
import shutil
import subprocess
import sys

(
	script,
	d8,
	aapt,
	aapt2,
	source_dir,
	build_dir,
	private_name,
	unaligned_name,
	sha_name,
	android_jar,
	sdl_jar,
	powder_jar,
	tpt_arch,
	debug_release,
	manifest_xml,
	*resources,
) = sys.argv

if tpt_arch == 'x86':
	android_arch = 'x86'
if tpt_arch == 'arm':
	android_arch = 'armeabi-v7a'
if tpt_arch == 'aarch64':
	android_arch = 'arm64-v8a'
if tpt_arch == 'x86_64':
	android_arch = 'x86_64'

sha_packaged_name = 'libpowder.so'

manifest_path  = os.path.join(build_dir, manifest_xml)
sha_path       = os.path.join(build_dir, sha_name)
unaligned_path = os.path.join(build_dir, unaligned_name)
private_dir    = os.path.join(build_dir, private_name)
arch_dir       = os.path.join(private_dir, 'lib', android_arch)
sha_lib_path   = os.path.join(arch_dir, sha_packaged_name)
flat_dir       = os.path.join(private_dir, 'flat')

if os.path.exists(arch_dir):
	shutil.rmtree(arch_dir)
os.makedirs(arch_dir)
if os.path.exists(sha_lib_path):
	os.remove(sha_lib_path)
if os.path.exists(unaligned_path):
	os.remove(unaligned_path)
if os.path.exists(flat_dir):
	shutil.rmtree(flat_dir)
os.makedirs(flat_dir)

if subprocess.run([
	aapt2,
	'compile',
	'-o', os.path.join(private_dir, 'flat'),
	*resources,
], cwd = build_dir).returncode:
	sys.exit(1)

aapt2_link_inputs = []
for root, dirs, files in os.walk(flat_dir):
	for name in files:
		if name.endswith(".flat"):
			aapt2_link_inputs.append(os.path.join(root, name))
if subprocess.run([
	aapt2,
	'link',
	'-o', unaligned_path,
	'-I', android_jar,
	'--manifest', manifest_path,
	*aapt2_link_inputs,
]).returncode:
	sys.exit(1)

shutil.copy(sha_path, sha_lib_path)
if subprocess.run([
	aapt,
	'add',
	unaligned_path,
	os.path.join('lib', android_arch, sha_packaged_name),
], cwd = private_dir).returncode:
	sys.exit(1)

if subprocess.run([
	d8,
	os.path.join(build_dir, sdl_jar),
	os.path.join(build_dir, powder_jar),
	'--' + debug_release,
	'--lib', android_jar,
	'--min-api', '21',
], cwd = private_dir).returncode:
	sys.exit(1)

if subprocess.run([
	aapt,
	'add',
	unaligned_path,
	'classes.dex',
], cwd = private_dir).returncode:
	sys.exit(1)
