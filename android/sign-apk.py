import os
import os.path
import subprocess
import sys

(
	script,
	apksigner,
	build_dir,
	apk_name,
	unsigned_name,
	android_keystore,
	android_keyalias,
) = sys.argv

if 'ANDROID_KEYSTORE_PASS' not in os.environ:
	print('ANDROID_KEYSTORE_PASS not set')
	sys.exit(1)

unsigned_path = os.path.join(build_dir, unsigned_name)
apk_path      = os.path.join(build_dir, apk_name)

if os.path.exists(apk_path):
	os.remove(apk_path)

if subprocess.run([
	apksigner,
	'sign',
	'--ks', android_keystore,
	'--ks-key-alias', android_keyalias,
	'--ks-pass', 'env:ANDROID_KEYSTORE_PASS',
	'--out', apk_path,
	unsigned_path,
]).returncode:
	sys.exit(1)
