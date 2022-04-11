import os
import os.path
import subprocess
import sys

(
	script,
	zipalign,
	build_dir,
	unsigned_name,
	unaligned_name,
) = sys.argv

unaligned_path = os.path.join(build_dir, unaligned_name)
unsigned_path  = os.path.join(build_dir, unsigned_name)

if os.path.exists(unsigned_path):
	os.remove(unsigned_path)

if subprocess.run([
	zipalign,
	'-f', '4',
	unaligned_path,
	unsigned_path,
]).returncode:
	sys.exit(1)
