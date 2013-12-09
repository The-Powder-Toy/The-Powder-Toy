# ============
# SCons script
# ============

# the purpose of this script is to run a build of tpt from start to finish, including dependency checks.

# .. contents :: Table of Contents

# ============
# requirements
# ============

# stdlib
# ======

import os
import sys
import subprocess
import time

# 3rd party
# =========

# nothing besides scons.

# =================
# long commandlines
# =================

# .. : Fix for long command line - http://scons.org/wiki/LongCmdLinesOnWin32

# because of an implementation detail commandlines are limited to 10000 characters on windows using mingw. the following fix was copied from
# http://scons.org/wiki/LongCmdLinesOnWin32 and circumvents this issue.

class ourSpawn:
	def ourspawn(self, sh, escape, cmd, args, env):
		newargs = ' '.join(args[1:])
		cmdline = cmd + " " + newargs
		startupinfo = subprocess.STARTUPINFO()
		startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
		proc = subprocess.Popen(cmdline, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
			stderr=subprocess.PIPE, startupinfo=startupinfo, shell = False, env = env)
		data, err = proc.communicate()
		rv = proc.wait()
		if rv:
			print "====="
			print err
			print "====="
		return rv

def SetupSpawn( env ):
	if sys.platform == 'win32':
		buf = ourSpawn()
		buf.ourenv = env
		env['SPAWN'] = buf.ourspawn

# ===================
# commandline options
# ===================

# the following defines all optional commandlines

AddOption('--opengl',dest="opengl",action='store_true',default=False,help="Build with OpenGL interface support.")
AddOption('--opengl-renderer',dest="opengl-renderer",action='store_true',default=False,help="Build with OpenGL renderer support. (requires --opengl)")
AddOption('--renderer',dest="renderer",action='store_true',default=False,help="Save renderer")
AddOption('--64bit',dest="_64bit",action='store_true',default=False,help="64-bit platform target")
AddOption('--static',dest="static",action="store_true",default=False,help="Static linking, reduces external library dependancies but increased file size")
AddOption('--pthreadw32-static',dest="ptw32-static",action="store_true",default=False,help="Use PTW32_STATIC_LIB for pthreadw32 headers")
AddOption('--python-ver',dest="pythonver",default=False,help="Python version to use for generator.py")
AddOption('--release',dest="release",action='store_true',default=False,help="Enable optimisations (Will slow down compiling)")
AddOption('--lua-dir',dest="lua-dir",default=False,help="Directory for lua includes")
AddOption('--sdl-dir',dest="sdl-dir",default=False,help="Directory for SDL includes")
AddOption('--tool',dest="toolprefix",default=False,help="Prefix")
AddOption('--sse',dest="sse",action='store_true',default=False,help="Enable SSE optimisations")
AddOption('--sse2',dest="sse2",action='store_true',default=False,help="Enable SSE2 optimisations")
AddOption('--sse3',dest="sse3",action='store_true',default=False,help="Enable SSE3 optimisations")
AddOption('--x86',dest="x86",action='store_true',default=True,help="Target Intel x86 platform")
AddOption('--nofft',dest="nofft", action='store_true',default=False,help="Do not use fftw3f for gravity.")
AddOption('--nolua',dest="nolua", action='store_true',default=False,help="Disable all lua scripting features.")

AddOption('--warnings-as-errors', dest="warnings_as_errors", action="store_true", default=False, help="Treat all warnings as errors")
AddOption('--debugging', dest="debug", action="store_true", default=False, help="Enable debug options")
AddOption('--beta',dest="beta",action='store_true',default=False,help="Beta build.")
AddOption('--save-version',dest="save-version",default=False,help="Save version.")
AddOption('--minor-version',dest="minor-version",default=False,help="Minor version.")
AddOption('--build-number',dest="build-number",default=False,help="Build number.")
AddOption('--snapshot',dest="snapshot",action='store_true',default=False,help="Snapshot build.")
AddOption('--snapshot-id',dest="snapshot-id",default=False,help="Snapshot build ID.")
AddOption('--stable',dest="stable",default=True,help="Non snapshot build")
AddOption('--aao', dest="everythingAtOnce", action='store_true', default=False, help="Compile the whole game without generating intermediate objects (very slow), enable this when using compilers like clang or mscc that don't support -fkeep-inline-functions")

AddOption('--fullclean',dest="justwork",action='store_true',default=False,help="for when nothing else works. Deletes all sconscript temporary files.") 
AddOption('--copy-env',dest="copy_env",action='store_true',default=False,help="copy some common enviroment variables from the parent enviroment.") 

# using one of these commandline options is compulsory

AddOption('--win',dest="win",action='store_true',default=False,help="Windows platform target.")
AddOption('--lin',dest="lin",action='store_true',default=False,help="Linux platform target")
AddOption('--macosx',dest="macosx",action='store_true',default=False,help="Mac OS X platform target")
AddOption('--rpi',dest="rpi",action='store_true',default=False,help="Raspbain platform target")

# ============
# main program
# ============

# the gist of the compiling rules are defined here


if(GetOption("justwork")):
	import shutil
	try:
		shutil.rmtree("../.sconf_temp/")
	except:
		print "couldn't remove .sconf_temp"
	try:
		os.remove("../.sconsign.dblite")
	except:
		print "couldn't remove .sconsign.dblite"

# platform selection
# ==================

# generic platform settings
# +++++++++++++++++++++++++

# check if a platform is specified.
# .. : TODO: make it suggest commandline options if it isn't

if((not GetOption('lin')) and (not GetOption('win')) and (not GetOption('rpi')) and (not GetOption('macosx'))):
	print "You must specify a platform to target"
	raise SystemExit(1)

# windows specific platform settings
# ++++++++++++++++++++++++++++++++++

# if the platform is windows switch to a mingw toolset, use the default otherwise

if(GetOption('win')):
	env = Environment(tools = ['mingw'], ENV = os.environ)
else:
	env = Environment(tools = ['default'], ENV = os.environ)

if(GetOption("copy_env")):
    lstvar=["CC","CXX","LD","CFLAGS","LIBPATH"]
    print "WARNING: enviroment copying enabled. changes in the enviroment can easily break the build process."
    for var in lstvar:
        if var in os.environ:
            env[var]=os.environ[var]
            print "WARNING: copying enviroment variable {}={!r}".format(var,os.environ[var])

# macosx specific platform settings
# +++++++++++++++++++++++++++++++++

# if we're not on MACOSX check for headers etc

if not GetOption("macosx"):
	conf = Configure(env)
	
# if sdl-dir is set check if we can find the sdl header there, if we can't just pass the header path to the compiler.

	if(GetOption("sdl-dir")):
		if not conf.CheckCHeader(GetOption("sdl-dir") + '/SDL.h'):
			print "sdl headers not found or not installed"
			raise SystemExit(1)
		else:
			env.Append(CPPPATH=[GetOption("sdl-dir")])
	else:

# otherwise try to parse the pkg config for sdl and grab the correct flags from there.

		try:
			env.ParseConfig('sdl-config --cflags')
			env.ParseConfig('sdl-config --libs')
		except:
			if not conf.CheckLib("SDL"):
				print "libSDL not found or not installed"
				raise SystemExit(1)
			

# if lua is enabled try to parse the lua pgk-config, or the lua-dir option if given
	
	if not GetOption("nolua"):
		if(GetOption("lua-dir")):
			if not conf.CheckCHeader(GetOption("lua-dir") + '/lua.h'):
				print "lua5.1 headers not found or not installed"
				raise SystemExit(1)
			else:
				env.Append(CPPPATH=[GetOption("lua-dir")])
		try:
			env.ParseConfig('pkg-config --cflags lua5.1')
		except:
			#Check for Lua lib
			if not conf.CheckLib('lua5.1') and not conf.CheckLib('lua-5.1') and not conf.CheckLib('lua51') and not conf.CheckLib('lua'):
				print "liblua5.1 not found or not installed"
				raise SystemExit(1)

# if fft is enabled try to parse its config, fail otherwise.

	if not GetOption('nofft'):
		# Check for FFT lib
		if not conf.CheckLib('fftw3f') and not conf.CheckLib('fftw3f-3'):
			print "libfftw3f not found or not installed"
			raise SystemExit(1)

# try to autodetect some libraries, fail otherwise

	#Check for Bzip lib
	if not conf.CheckLib('bz2'):
		print "libbz2 not found or not installed"
		raise SystemExit(1)

	#Check for zlib
	if not conf.CheckLib('z'):
		print "libz not found or not installed"
		raise SystemExit(1)

	if not conf.CheckCHeader("bzlib.h"):
		print "bzip2 headers not found"
		raise SystemExit(1)

# finish the configuration

	env = conf.Finish();
else:

# if we ARE on macosx add the libraries to LIBS
# .. : seems like we're terrible at mac support? what gives?

	env.Append(LIBS=['z', 'bz2'])
	if not GetOption('nofft'):
		env.Append(LIBS=['fftw3f'])

# enviroment setup
# ================

# add the correct compiler flags.

# generic enviroment settings
# +++++++++++++++++++++++++++

# check if a tool prefix is set, and if it is select the propper tools for building.
# .. : TODO someone explain wtf this actually does

if GetOption("toolprefix"):
	env['CC'] = GetOption("toolprefix")+env['CC']
	env['CXX'] = GetOption("toolprefix")+env['CXX']
	if GetOption('win'):
		env['RC'] = GetOption("toolprefix")+env['RC']

# make sure the compiler can find the source data and generated files. enable warnings, set C++ flavor, and keep inline functions

env.Append(CPPPATH=['src/', 'data/', 'generated/'])
env.Append(CXXFLAGS=['-std=c++98'])
env.Append(LIBS=['pthread', 'm'])
env.Append(CPPDEFINES=["_GNU_SOURCE", "USE_STDINT", "_POSIX_C_SOURCE=200112L"])

# set the warnings we want, treat all warnings as errors, and ignore all "offsetof" warnings

env.Append(CCFLAGS=['-Wno-invalid-offsetof']);
if GetOption('warnings_as_errors'):
	env.Append(CCFLAGS=['-Werror']);

# check all enabled libs, and add a define if they are enabled.

if not GetOption('nofft'):
	env.Append(CPPDEFINES=["GRAVFFT"])
if not GetOption('nolua'):
	env.Append(CPPDEFINES=["LUACONSOLE"])

# check if we need to use PTW32_STATIC_LIB for pthreadw32 headers, won't compile statically without this

if GetOption("ptw32-static"):
	env.Append(CPPDEFINES=['PTW32_STATIC_LIB']);

# check if we need to do static linking.

if(GetOption('static')):
	env.Append(LINKFLAGS=['-static-libgcc'])

# check if we need to compile the save renderer. add a define accordingly. compile the game by default.

if(GetOption('renderer')):
	env.Append(CPPDEFINES=['RENDERER'])
else:
	env.Append(CPPDEFINES=["USE_SDL"])

# apply optimisations if it's a release build
	
if(GetOption('release')):
	if GetOption('macosx'):
		env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer'])
	else:
		env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer', '-funsafe-loop-optimizations'])

# rpi specific enviroment settings
# ++++++++++++++++++++++++++++++++

# check if we're compiling for raspberry pi, if we are include rpi specific libraries and defines.

if(GetOption('rpi')):
        if(GetOption('opengl')):
                env.ParseConfig('pkg-config --libs glew gl glu')
        openGLLibs = ['GL']
        env.Append(LIBS=['X11', 'rt'])
        env.Append(CPPDEFINES=["LIN"])


# windows specific enviroment settings
# ++++++++++++++++++++++++++++++++++++

# check if we're compiling for windows, if we are include windows specific libraries and defines.

if(GetOption('win')):
	openGLLibs = ['opengl32', 'glew32']
	env.Prepend(LIBS=['mingw32', 'ws2_32', 'SDLmain', 'SDL'])
	env.Append(CCFLAGS=['-std=gnu++98'])
	env.Append(LIBS=['winmm', 'gdi32'])
	env.Append(CPPDEFINES=["WIN"])
	env.Append(LINKFLAGS=['-mwindows'])
	if(GetOption('_64bit')):
		env.Append(CPPDEFINES=['__CRT__NO_INLINE'])
		env.Append(LINKFLAGS=['-Wl,--stack=16777216'])

# linux specific enviroment settings
# ++++++++++++++++++++++++++++++++++++

# check if we're compiling for linux, if we are include linux specific libraries and defines.

if(GetOption('lin')):
	if(GetOption('opengl')):
		env.ParseConfig('pkg-config --libs glew gl glu')
	openGLLibs = ['GL']
	env.Append(LIBS=['X11', 'rt'])
	env.Append(CPPDEFINES=["LIN"])
	if GetOption('_64bit'):
		env.Append(LINKFLAGS=['-m64'])
		env.Append(CCFLAGS=['-m64'])
	else:
		env.Append(LINKFLAGS=['-m32'])
		env.Append(CCFLAGS=['-m32'])

# macosx specific enviroment settings
# ++++++++++++++++++++++++++++++++++++

# check if we're compiling for macosx, if we are include macosx specific libraries and defines.

if(GetOption('macosx')):
	env.Append(CPPDEFINES=["MACOSX"])
	env.Append(CCFLAGS=['-I/Library/Frameworks/SDL.framework/Headers'])
	env.Append(CCFLAGS=['-I/Library/Frameworks/Lua.framework/Headers'])
	if not GetOption('nofft'):
		env.Append(LINKFLAGS=['-lfftw3f'])
	env.Append(LINKFLAGS=['-framework'])
	env.Append(LINKFLAGS=['SDL'])
	env.Append(LINKFLAGS=['-framework'])
	env.Append(LINKFLAGS=['Lua'])
	env.Append(LINKFLAGS=['-framework']);
	env.Append(LINKFLAGS=['Cocoa'])
	#env.Append(LINKFLAGS=['-framework SDL'])
	#env.Append(LINKFLAGS=['-framework Lua'])
	#env.Append(LINKFLAGS=['-framework Cocoa'])
	if GetOption('_64bit'):
		env.Append(LINKFLAGS=['-m64'])
		env.Append(CCFLAGS=['-m64'])
	else:
		env.Append(LINKFLAGS=['-m32'])
		env.Append(CCFLAGS=['-m32'])

# defines
# =======

# A lot of commandline flags translate directly into defines. those flags follow:

if GetOption('_64bit'):
	env.Append(CPPDEFINES=["_64BIT"])

if(GetOption('beta')):
	env.Append(CPPDEFINES='BETA')

if(not GetOption('snapshot') and not GetOption('beta') and not GetOption('release') and not GetOption('stable')):
	env.Append(CPPDEFINES='SNAPSHOT_ID=0')
	env.Append(CPPDEFINES='SNAPSHOT')
elif(GetOption('snapshot') or GetOption('snapshot-id')):
	if(GetOption('snapshot-id')):
		env.Append(CPPDEFINES=['SNAPSHOT_ID=' + GetOption('snapshot-id')])
	else:
		env.Append(CPPDEFINES=['SNAPSHOT_ID=' + str(int(time.time()))])
	env.Append(CPPDEFINES='SNAPSHOT')
elif(GetOption('stable')):
	env.Append(CPPDEFINES='STABLE')

if(GetOption('save-version')):
	env.Append(CPPDEFINES=['SAVE_VERSION=' + GetOption('save-version')])

if(GetOption('minor-version')):
	env.Append(CPPDEFINES=['MINOR_VERSION=' + GetOption('minor-version')])

if(GetOption('build-number')):
	env.Append(CPPDEFINES=['BUILD_NUM=' + GetOption('build-number')])

if(GetOption('x86')):
	env.Append(CPPDEFINES='X86')

if(GetOption('debug')):
	env.Append(CPPDEFINES='DEBUG')
	env.Append(CCFLAGS='-g')

if(GetOption('sse')):
	env.Append(CCFLAGS='-msse')
	env.Append(CPPDEFINES='X86_SSE')

if(GetOption('sse2')):
	env.Append(CCFLAGS='-msse2')
	env.Append(CPPDEFINES='X86_SSE2')

if(GetOption('sse3')):
	env.Append(CCFLAGS='-msse3')
	env.Append(CPPDEFINES='X86_SSE3')

if(GetOption('opengl')):
	env.Append(CPPDEFINES=["OGLI", "PIX32OGL"])
	env.Append(LIBS=openGLLibs)

if(GetOption('opengl') and GetOption('opengl-renderer')):
	env.Append(CPPDEFINES=["OGLR"])
elif(GetOption('opengl-renderer')):
	print "opengl-renderer requires opengl"
	raise SystemExit(1)

# compiling
# =========

# sources
# +++++++

# find all source files

# generic sources
# ---------------
sources=Glob("src/*.cpp")
	
sources+=Glob("src/*/*.cpp")
sources+=Glob("src/gui/*/*.cpp")
sources+=Glob("src/simulation/elements/*.cpp")
sources+=Glob("src/simulation/tools/*.cpp")
sources+=Glob("src/client/requestbroker/*.cpp")
if not GetOption('nolua'):
	sources+=Glob("src/socket/*.c")

# windows specific sources
# ------------------------

if(GetOption('win')):
    sources += env.RES('resources/powder-res.rc')
    sources = filter(lambda source: not 'src\\simulation\\Gravity.cpp' in str(source), sources)
    sources = filter(lambda source: not 'src/simulation/Gravity.cpp' in str(source), sources)

# macosx specific sources
# -----------------------

if(GetOption('macosx')):
	sources +=["SDLMain.m"]

# apply `long commandlines`_ fix
# ==============================

# apply the commandline fix

SetupSpawn(env)

# find proper executable name
# ===========================

# use some settings to detect what name to use for the executable

programName = "powder"

if(GetOption('renderer')):
	programName = "render"

if(GetOption('win')):
	if(GetOption('renderer')):
		programName = "Render"
	else:
		programName = "Powder"

if(GetOption('_64bit')):
	programName += "64"

if(not (GetOption('sse2') or GetOption('sse3'))):
	programName += "-legacy"

if(GetOption('macosx')):
	programName += "-x"

if(GetOption('win')):
	programName += ".exe"


# detect python executable name
# =============================

# detect the executable name for python so we can run some generator scripts

if(GetOption('pythonver')):
	pythonVer = GetOption('pythonver')
elif(GetOption('lin')):
	pythonVer = "python2"
else:
	pythonVer = "python"

# Extra compiler flag to fix stack alignment
# When Windows creates the gravity calculation thread, it has 4 byte stack alignment
# But we need 16 byte alignment so that SSE instructions in FFTW work without crashing
if(GetOption('win')):
   envCopy = env.Clone()
   envCopy.Append(CCFLAGS=['-mstackrealign'])
   #envCopy.Append(CCFLAGS=['-mincoming-stack-boundary=2'])
   sources+=envCopy.Object('src/simulation/Gravity.cpp')

# run generator commands
# ======================

env.Command(['generated/ElementClasses.cpp', 'generated/ElementClasses.h'], Glob('src/simulation/elements/*.cpp'), pythonVer + " generator.py elements $TARGETS $SOURCES")
sources+=Glob("generated/ElementClasses.cpp")

env.Command(['generated/ToolClasses.cpp', 'generated/ToolClasses.h'], Glob('src/simulation/tools/*.cpp'), pythonVer + " generator.py tools $TARGETS $SOURCES")
sources+=Glob("generated/ToolClasses.cpp")

# final settings
# ==============

# make a MD5 checksum decide wether or not a file changed. we had some problems with using the modification date for this purpose.

env.Decider('MD5')

# set a defaukt target

t=env.Program(target=programName, source=sources)
Default(t)
