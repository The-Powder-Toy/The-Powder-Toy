import os
import os.path
import subprocess
import sys

(
	script,
	adb,
	build_dir,
	phony,
	apk_name,
) = sys.argv

apk_path   = os.path.join(build_dir, apk_name)
phony_path = os.path.join(build_dir, phony)

if os.path.exists(phony_path):
	os.remove(phony_path)

if subprocess.run([
	adb,
	'install',
	apk_path,
]).returncode:
	sys.exit(1)

with open(phony_path, 'w') as _:
	pass
