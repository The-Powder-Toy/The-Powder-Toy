import os
import os.path
import shutil
import subprocess
import sys

(
	script,
	javac,
	jar,
	source_dir,
	build_dir,
	private_name,
	powder_jar_name,
	android_jar,
	java_runtime,
	sdl_jar,
	debug_release,
	*javac_sources,
) = sys.argv

powder_jar_path = os.path.join(build_dir, powder_jar_name)
private_dir     = os.path.join(build_dir, private_name)
class_dir       = os.path.join(private_dir, 'class')

if os.path.exists(powder_jar_path):
	os.remove(powder_jar_path)
if os.path.exists(class_dir):
	shutil.rmtree(class_dir)
os.makedirs(class_dir)

if subprocess.run([
	javac,
	'-d', class_dir,
	'-source', '1.8',
	'-target', '1.8',
	'-bootclasspath', java_runtime,
	'-classpath', os.pathsep.join([ android_jar, sdl_jar ]),
	*javac_sources,
], cwd = build_dir).returncode:
	sys.exit(1)

jar_inputs = []
for root, dirs, files in os.walk(class_dir):
	for name in files:
		if name.endswith(".class"):
			jar_inputs.append(os.path.relpath(os.path.join(root, name), start = class_dir))
if subprocess.run([
	jar,
	'cMf',
	powder_jar_path,
	*jar_inputs,
], cwd = class_dir).returncode:
	sys.exit(1)
