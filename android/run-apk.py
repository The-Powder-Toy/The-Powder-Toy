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
	app_id,
) = sys.argv

apk_path = os.path.join(build_dir, apk_name)

if subprocess.run([
	adb,
	'shell',
	'am',
	'start',
	'--activity-clear-top',
	'-n', app_id + '/.PowderActivity',
]).returncode:
	sys.exit(1)
