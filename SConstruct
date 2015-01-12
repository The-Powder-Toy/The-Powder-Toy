#run generator.py
if not GetOption('clean'):
	execfile("generator.py")

AddOption('--builddir',dest="builddir",default="build",help="Directory to build to.")
SConscript('SConscript', variant_dir=GetOption('builddir'), duplicate=0)
if GetOption('clean'):
	import os, shutil
	try:
		shutil.rmtree(".sconf_temp/")
	except:
		print "couldn't remove .sconf_temp/"

	try:
		shutil.rmtree("generated/")
	except:
		print "couldn't remove generated/"

	try:
		os.remove(".sconsign.dblite")
	except:
		print "couldn't remove .sconsign.dblite"
