
import os
import subprocess
import sys
import platform
import atexit
import time
import SCons.Util


# because of an implementation detail commandlines are limited to 10000 characters on windows using mingw. the following fix was copied from
# http://scons.org/wiki/LongCmdLinesOnWin32 and circumvents this issue.
class ourSpawn:
	def ourspawn(self, sh, escape, cmd, args, env):
		newargs = ' '.join(args[1:])
		cmdline = cmd + " " + newargs
		startupinfo = subprocess.STARTUPINFO()
		startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
		proc = subprocess.Popen(cmdline, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
			stderr=subprocess.PIPE, startupinfo=startupinfo, shell=False, env=env)
		data, err = proc.communicate()
		rv = proc.wait()
		if rv:
			print("=====")
			print(err)
			print("=====")
		return rv
def SetupSpawn(env):
	buf = ourSpawn()
	buf.ourenv = env
	env['SPAWN'] = buf.ourspawn

def FatalError(message):
	print(message)
	raise SystemExit(1)

#wrapper around SCons' AddOption
def AddSconsOption(name, default, hasArgs, help):
	AddOption("--{0}".format(name), dest=name, action=("store" if hasArgs else "store_true"), default=default, help=help)

AddSconsOption('win', False, False, "Target Windows.")
AddSconsOption('lin', False, False, "Target Linux.")
AddSconsOption('mac', False, False, "Target Mac OS X.")
AddSconsOption('msvc', False, False, "Use the Microsoft Visual Studio compiler.")
AddSconsOption("tool", False, True, "Tool prefix appended before gcc/g++.")

AddSconsOption('beta', False, False, "Beta build.")
AddSconsOption('save-version', False, True, "Save version.")
AddSconsOption('minor-version', False, True, "Minor version.")
AddSconsOption('build-number', False, True, "Build number.")
AddSconsOption('snapshot', False, False, "Snapshot build.")
AddSconsOption('snapshot-id', False, True, "Snapshot build ID.")

AddSconsOption('64bit', False, False, "Compile a 64 bit binary.")
AddSconsOption('32bit', False, False, "Compile a 32 bit binary.")
AddSconsOption("universal", False, False, "compile universal binaries on Mac OS X.")
AddSconsOption('no-sse', False, False, "Disable SSE optimizations.")
AddSconsOption('sse', True, False, "Enable SSE optimizations (default).")
AddSconsOption('sse2', True, False, "Enable SSE2 optimizations (default).")
AddSconsOption('sse3', False, False, "Enable SSE3 optimizations.")
AddSconsOption('native', False, False, "Enable optimizations specific to your cpu.")
AddSconsOption('release', True, False, "Enable loop / compiling optimizations (default).")

AddSconsOption('debugging', False, False, "Compile with debug symbols.")
AddSconsOption('symbols', False, False, "Preserve (don't strip) symbols")
AddSconsOption('static', False, False, "Compile statically.")
AddSconsOption('opengl', False, False, "Build with OpenGL interface support.")
AddSconsOption('opengl-renderer', False, False, "Build with OpenGL renderer support (turns on --opengl).") #Note: this has nothing to do with --renderer, only tells the game to render particles with opengl
AddSconsOption('renderer', False, False, "Build the save renderer.")

AddSconsOption('wall', False, False, "Error on all warnings.")
AddSconsOption('no-warnings', False, False, "Disable all compiler warnings.")
AddSconsOption('nolua', False, False, "Disable Lua.")
AddSconsOption('luajit', False, False, "Enable LuaJIT")
AddSconsOption('lua52', False, False, "Compile using lua 5.2")
AddSconsOption('nofft', False, False, "Disable FFT.")
AddSconsOption("output", False, True, "Executable output name.")


#detect platform automatically, but it can be overrided
tool = GetOption('tool')
isX86 = platform.machine() in ["AMD64", "i386", "i686", "x86", "x86_64"]
platform = compilePlatform = platform.system()
if GetOption('win'):
	platform = "Windows"
elif GetOption('lin'):
	platform = "Linux"
elif GetOption('mac'):
	platform = "Darwin"
elif compilePlatform not in ["Linux", "Windows", "Darwin"]:
	FatalError("Unknown platform: {0}".format(platform))

msvc = GetOption('msvc')
if msvc and platform != "Windows":
	FatalError("Error: --msvc only works on windows")

#Create SCons Environment
if GetOption('msvc'):
	env = Environment(tools=['default'], ENV={'PATH' : os.environ['PATH'], 'TMP' : os.environ['TMP']}, TARGET_ARCH='x86')
elif platform == "Windows" and not GetOption('msvc'):
	env = Environment(tools=['mingw'], ENV={'PATH' : os.environ['PATH']})
else:
	env = Environment(tools=['default'], ENV={'PATH' : os.environ['PATH']})

#attempt to automatically find cross compiler
if not tool and compilePlatform == "Linux" and compilePlatform != platform:
	if platform == "Darwin":
		crossList = ["i686-apple-darwin9", "i686-apple-darwin10"]
	elif not GetOption('64bit'):
		crossList = ["mingw32", "i686-w64-mingw32", "i386-mingw32msvc", "i486-mingw32msvc", "i586-mingw32msvc", "i686-mingw32msvc"]
	else:
		crossList = ["x86_64-w64-mingw32", "amd64-mingw32msvc"]
	for i in crossList:
		#found a cross compiler, set tool here, which will update everything in env later
		if WhereIs("{0}-g++".format(i)):
			tool = i+"-"
			break
	if not tool:
		print("Could not automatically find cross compiler, use --tool to specify manually")

#set tool prefix
#more things may need to be set (http://clam-project.org/clam/trunk/CLAM/scons/sconstools/crossmingw.py), but this works for us
if tool:
	env['CC'] = tool+env['CC']
	env['CXX'] = tool+env['CXX']
	if platform == "Windows":
		env['RC'] = tool+env['RC']
	env['STRIP'] = tool+'strip'
	if os.path.isdir("/usr/{0}/bin".format(tool[:-1])):
		env['ENV']['PATH'] = "/usr/{0}/bin:{1}".format(tool[:-1], os.environ['PATH'])
	if platform == "Darwin":
		sdlconfigpath = "/usr/lib/apple/SDKs/MacOSX10.5.sdk/usr/bin"
		if os.path.isdir(sdlconfigpath):
			env['ENV']['PATH'] = "{0}:{1}".format(sdlconfigpath, env['ENV']['PATH'])

#copy environment variables because scons doesn't do this by default
for var in ["CC","CXX","LD","LIBPATH","STRIP"]:
	if var in os.environ:
		env[var] = os.environ[var]
		print("copying environment variable {0}={1!r}".format(var,os.environ[var]))
# variables containing several space separated things
for var in ["CFLAGS","CCFLAGS","CXXFLAGS","LINKFLAGS","CPPDEFINES","CPPPATH"]:
	if var in os.environ:
		if var in env:
			env[var] += SCons.Util.CLVar(os.environ[var])
		else:
			env[var] = SCons.Util.CLVar(os.environ[var])
		print("copying environment variable {0}={1!r}".format(var,os.environ[var]))

#Used for intro text / executable name, actual bit flags are only set if the --64bit/--32bit command line args are given
def add32bitflags(env):
	env["BIT"] = 32
def add64bitflags(env):
	if platform == "Windows":
		env.Append(CPPDEFINES=['__CRT__NO_INLINE'])
		env.Append(LINKFLAGS=['-Wl,--stack=16777216'])
	env.Append(CPPDEFINES=['_64BIT'])
	env["BIT"] = 64
#add 32/64 bit defines before configuration
if GetOption('64bit'):
	env.Append(LINKFLAGS=['-m64'])
	env.Append(CCFLAGS=['-m64'])
	add64bitflags(env)
elif GetOption('32bit'):
	env.Append(LINKFLAGS=['-m32'])
	env.Append(CCFLAGS=['-m32'])
	add32bitflags(env)

if GetOption('universal'):
	if platform != "Darwin":
		FatalError("Error: --universal only works on Mac OS X")
	else:
		env.Append(CCFLAGS=['-arch', 'i386', '-arch', 'x86_64'])
		env.Append(LINKFLAGS=['-arch', 'i386', '-arch', 'x86_64'])

env.Append(CPPPATH=['src/', 'data/', 'generated/'])
if GetOption("msvc"):
	if GetOption("static"):
		env.Append(LIBPATH=['StaticLibs/'])
	else:
		env.Append(LIBPATH=['Libraries/'])
	env.Append(CPPPATH=['includes/'])

#Check 32/64 bit
def CheckBit(context):
	context.Message('Checking if 64 bit... ')
	program = """#include <stdlib.h>
	#include <stdio.h>
	int main() {
		printf("%d", (int)sizeof(size_t));
		return 0;
	}
	"""
	ret = context.TryCompile(program, '.c')
	if ret == 0:
		return False
	ret = context.TryRun(program, '.c')
	if ret[1] == '':
		return False
	context.Result(int(ret[1]) == 8)
	if int(ret[1]) == 8:
		print("Adding 64 bit compile flags")
		add64bitflags(context.env)
	elif int(ret[1]) == 4:
		print("Adding 32 bit compile flags")
		add32bitflags(context.env)
	return ret[1]

#Custom function to check for Mac OS X frameworks
def CheckFramework(context, framework):
	import SCons.Conftest
	#Extreme hack, TODO: maybe think of a better one (like replicating CheckLib here) or at least just fix the message
	oldLinkFlags = env["LINKFLAGS"]
	context.env.Append(LINKFLAGS=["-framework", framework])
	context.Display("Checking for Darwin Framework {0}...".format(framework))
	ret = SCons.Conftest.CheckLib(context, ["m"], autoadd = 0)
	context.did_show_result = 1
	if not ret:
		context.env.Append(LINKFLAGS=["-framework", framework])
		if framework != "Cocoa":
			env.Append(CPPPATH=['/Library/Frameworks/{0}.framework/Headers/'.format(framework)])
	else:
		context.env.Replace(LINKFLAGS=oldLinkFlags)
	return not ret

#function that finds libraries and appends them to LIBS
def findLibs(env, conf):
	#Windows specific libs
	if platform == "Windows":
		if msvc:
			libChecks = ['shell32', 'wsock32', 'user32', 'Advapi32', 'ws2_32']
			if GetOption('static'):
				libChecks += ['libcmt', 'dxguid']
			for i in libChecks:
				if not conf.CheckLib(i):
					FatalError("Error: some windows libraries not found or not installed, make sure your compiler is set up correctly")
		else:
			if not conf.CheckLib('mingw32') or not conf.CheckLib('ws2_32'):
				FatalError("Error: some windows libraries not found or not installed, make sure your compiler is set up correctly")

		if not conf.CheckLib('SDLmain'):
			FatalError("libSDLmain not found or not installed")

	if not GetOption('renderer'):
		#Look for SDL
		runSdlConfig = platform == "Linux" or compilePlatform == "Linux"
		if platform == "Darwin" and conf.CheckFramework("SDL"):
			runSdlConfig = False
		elif not conf.CheckLib("SDL"):
			FatalError("SDL development library not found or not installed")

		if runSdlConfig:
			try:
				env.ParseConfig('sdl-config --cflags')
				if GetOption('static'):
					env.ParseConfig('sdl-config --static-libs')
				else:
					env.ParseConfig('sdl-config --libs')
			except:
				pass

	#look for SDL.h
	if not GetOption('renderer') and not conf.CheckCHeader('SDL.h'):
		if conf.CheckCHeader('SDL/SDL.h'):
			env.Append(CPPDEFINES=["SDL_INC"])
		else:
			FatalError("SDL.h not found")

	if not GetOption('nolua') and not GetOption('renderer'):
		#Look for Lua
		luaver = "lua5.1"
		if GetOption('luajit'):
			if not conf.CheckLib(['luajit-5.1', 'luajit5.1', 'luajit2.0', 'luajit', 'libluajit']):
				FatalError("luajit development library not found or not installed")
			env.Append(CPPDEFINES=["LUAJIT"])
			luaver = "luajit"
		elif GetOption('lua52'):
			if not conf.CheckLib(['lua5.2', 'lua-5.2', 'lua52', 'lua']):
				FatalError("lua5.2 development library not found or not installed")
			env.Append(CPPDEFINES=["LUA_COMPAT_ALL"])
			luaver = "lua5.2"
		else:
			if not conf.CheckLib(['lua5.1', 'lua-5.1', 'lua51', 'lua']):
				if platform != "Darwin" or not conf.CheckFramework("Lua"):
					FatalError("lua5.1 development library not found or not installed")
		foundpkg = False
		if platform == "Linux":
			try:
				env.ParseConfig("pkg-config --cflags {0}".format(luaver))
				env.ParseConfig("pkg-config --libs {0}".format(luaver))
				env.Append(CPPDEFINES=["LUA_R_INCL"])
				foundpkg = True
			except:
				pass
		if not foundpkg:
			#Look for lua.h
			foundheader = False
			if GetOption('luajit'):
				foundheader = conf.CheckCHeader('luajit-2.0/lua.h')
			elif GetOption('lua52'):
				foundheader = conf.CheckCHeader('lua5.2/lua.h')
			else:
				foundheader = conf.CheckCHeader('lua5.1/lua.h')
			if not foundheader:
				if conf.CheckCHeader('lua.h'):
					env.Append(CPPDEFINES=["LUA_R_INCL"])
				else:
					FatalError("lua.h not found")

		#needed for static lua compiles (in some cases)
		if platform == "Linux":
			conf.CheckLib('dl')

	#Look for fftw
	if not GetOption('nofft') and not conf.CheckLib(['fftw3f', 'fftw3f-3', 'libfftw3f-3', 'libfftw3f']):
			FatalError("fftw3f development library not found or not installed")

	#Look for bz2
	if not conf.CheckLib(['bz2', 'libbz2']):
		FatalError("bz2 development library not found or not installed")

	#Check bz2 header too for some reason
	if not conf.CheckCHeader('bzlib.h'):
		FatalError("bzip2 headers not found")

	#Look for libz
	if not conf.CheckLib(['z', 'zlib']):
		FatalError("libz not found or not installed")

	#Look for pthreads
	if not conf.CheckLib(['pthread', 'pthreadVC2']):
		FatalError("pthreads development library not found or not installed")

	if msvc:
		if not conf.CheckHeader('dirent.h') or not conf.CheckHeader('fftw3.h') or not conf.CheckHeader('pthread.h') or not conf.CheckHeader('zlib.h'):
			FatalError("Required headers not found")
	else:
		#Look for libm
		if not conf.CheckLib('m'):
			FatalError("libm not found or not installed")

	#Look for OpenGL libraries
	if GetOption('opengl'):
		if platform == "Linux":
			if not conf.CheckLib('GL'):
				FatalError("libGL not found or not installed")
			try:
				env.ParseConfig('pkg-config --libs glew gl glu')
			except:
				FatalError(sys.exc_info()[0])

		elif platform == "Windows":
			if not conf.CheckLib('opengl32'):
				FatalError("opengl32 not found or not installed")
			if not conf.CheckLib('glew32'):
				FatalError("glew32 not found or not installed")
		elif platform == "Darwin":
			if not conf.CheckFramework("OpenGL"):
				FatalError("OpenGL framework not found or not installed")

	if platform == "Linux":
		if not conf.CheckLib('X11'):
			FatalError("X11 development library not found or not installed")

		if not conf.CheckLib('rt'):
			FatalError("librt not found or not installed")
	elif platform == "Windows":
		#These need to go last
		if not conf.CheckLib('gdi32') or not conf.CheckLib('winmm') or (not msvc and not conf.CheckLib('dxguid')):
			FatalError("Error: some windows libraries not found or not installed, make sure your compiler is set up correctly")
	elif platform == "Darwin":
		if not conf.CheckFramework("Cocoa"):
			FatalError("Cocoa framework not found or not installed")

if GetOption('clean'):
	import shutil
	try:
		shutil.rmtree("generated/")
	except:
		print("couldn't remove build/generated/")
elif not GetOption('help'):
	conf = Configure(env)
	conf.AddTest('CheckFramework', CheckFramework)
	conf.AddTest('CheckBit', CheckBit)
	if not conf.CheckCC() or not conf.CheckCXX():
		FatalError("compiler not correctly configured")
	if platform == compilePlatform and isX86 and not GetOption('32bit') and not GetOption('64bit') and not GetOption('msvc'):
		conf.CheckBit()
	findLibs(env, conf)
	env = conf.Finish()

if not msvc:
	if platform == "Windows":
		env.Append(CXXFLAGS=['-std=gnu++98'])
	else:
		env.Append(CXXFLAGS=['-std=c++98'])
	env.Append(CXXFLAGS=['-Wno-invalid-offsetof'])
	if platform == "Linux":
		env.Append(CXXFLAGS=['-Wno-unused-result'])


#Add platform specific flags and defines
if platform == "Windows":
	env.Append(CPPDEFINES=["WIN", "_WIN32_WINNT=0x0501", "_USING_V110_SDK71_"])
	if msvc:
		env.Append(CCFLAGS=['/Gm', '/Zi', '/EHsc', '/FS', '/GS']) #enable minimal rebuild, ?, enable exceptions, allow -j to work in debug builds, enable security check
		env.Append(LINKFLAGS=['/SUBSYSTEM:WINDOWS,"5.01"', '/OPT:REF', '/OPT:ICF'])
		env.Append(CPPDEFINES=['_SCL_SECURE_NO_WARNINGS']) #Disable warnings about 'std::print'
		if GetOption('static'):
			env.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib', '/LTCG'])
		elif not GetOption('debugging'):
			env.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrtd.lib'])
	else:
		env.Append(LINKFLAGS=['-mwindows'])
elif platform == "Linux":
	env.Append(CPPDEFINES=['LIN'])
elif platform == "Darwin":
	env.Append(CPPDEFINES=['MACOSX'])
	#env.Append(LINKFLAGS=['-headerpad_max_install_names']) #needed in some cross compiles
	if GetOption('luajit'):
		env.Append(LINKFLAGS=['-pagezero_size', '10000', '-image_base', '100000000'])


#Add architecture flags and defines
if isX86:
	env.Append(CPPDEFINES='X86')
if not GetOption('no-sse'):
	if GetOption('sse'):
		if msvc:
			if not GetOption('sse2'):
				env.Append(CCFLAGS=['/arch:SSE'])
		else:
			env.Append(CCFLAGS=['-msse'])
		env.Append(CPPDEFINES=['X86_SSE'])
	if GetOption('sse2'):
		if msvc:
			env.Append(CCFLAGS=['/arch:SSE2'])
		else:
			env.Append(CCFLAGS=['-msse2'])
		env.Append(CPPDEFINES=['X86_SSE2'])
	if GetOption('sse3'):
		if msvc:
			env.Append(CCFLAGS=['/arch:SSE3'])
		else:
			env.Append(CCFLAGS=['-msse3'])
		env.Append(CPPDEFINES=['X86_SSE3'])
if GetOption('native') and not msvc:
	env.Append(CCFLAGS=['-march=native'])


#Add optimization flags and defines
if GetOption('debugging'):
	if msvc:
		env.Append(CCFLAGS=['/Od'])
		if GetOption('static'):
			env.Append(CCFLAGS=['/MTd'])
		else:
			env.Append(CCFLAGS=['/MDd'])
	else:
		env.Append(CCFLAGS=['-Wall', '-g'])
		env.Append(CPPDEFINES=['DEBUG'])
elif GetOption('release'):
	if msvc:
		env.Append(CCFLAGS=['/O2', '/Oy-', '/fp:fast'])
		if GetOption('static'):
			env.Append(CCFLAGS=['/MT'])
		else:
			env.Append(CCFLAGS=['/MD'])
	else:
		env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer'])
		if platform != "Darwin":
			env.Append(CCFLAGS=['-funsafe-loop-optimizations'])

if GetOption('static'):
	if platform == "Windows":
		if compilePlatform == "Windows" and not msvc:
			env.Append(CPPDEFINES=['_PTW32_STATIC_LIB'])
		else:
			env.Append(CPPDEFINES=['PTW32_STATIC_LIB'])
		if msvc:
			env.Append(CPPDEFINES=['ZLIB_WINAPI'])
		else:
			env.Append(LINKFLAGS=['-Wl,-Bstatic'])


#Add other flags and defines
if not GetOption('nofft'):
	env.Append(CPPDEFINES=['GRAVFFT'])
if not GetOption('nolua') and not GetOption('renderer'):
	env.Append(CPPDEFINES=['LUACONSOLE'])

if GetOption('opengl') or GetOption('opengl-renderer'):
	env.Append(CPPDEFINES=['OGLI', 'PIX32OGL'])
	if GetOption('opengl-renderer'):
		env.Append(CPPDEFINES=['OGLR'])

if GetOption('renderer'):
	env.Append(CPPDEFINES=['RENDERER'])
else:
	env.Append(CPPDEFINES=['USE_SDL'])

if GetOption("wall"):
	if msvc:
		env.Append(CCFLAGS=['/WX'])
	else:
		env.Append(CCFLAGS=['-Werror'])
elif GetOption("no-warnings"):
	if msvc:
		env.Append(CCFLAGS=['/W0'])
	else:
		env.Append(CCFLAGS=['-w'])


#Add version defines
if GetOption('save-version'):
	env.Append(CPPDEFINES=["SAVE_VERSION={0}".format(GetOption('save-version'))])

if GetOption('minor-version'):
	env.Append(CPPDEFINES=["MINOR_VERSION={0}".format(GetOption('minor-version'))])

if GetOption('build-number'):
	env.Append(CPPDEFINES=["BUILD_NUM={0}".format(GetOption('build-number'))])

if GetOption('snapshot-id'):
	env.Append(CPPDEFINES=["SNAPSHOT", "SNAPSHOT_ID={0}".format(GetOption('snapshot-id'))])
elif GetOption('snapshot'):
	env.Append(CPPDEFINES=["SNAPSHOT", "SNAPSHOT_ID={0}".format(str(int(time.time())))])

if GetOption('beta'):
	env.Append(CPPDEFINES=['BETA'])


#Generate list of sources to compile
sources = Glob("src/*.cpp") + Glob("src/*/*.cpp") + Glob("src/*/*/*.cpp") + Glob("generated/*.cpp")
if not GetOption('nolua') and not GetOption('renderer'):
	sources += Glob("src/lua/socket/*.c") + Glob("src/lua/LuaCompat.c")

if platform == "Windows":
	sources += env.RES('resources/powder-res.rc')
	if not msvc:
		sources = filter(lambda source: not 'src\\simulation\\Gravity.cpp' in str(source), sources)
		sources = filter(lambda source: not 'src/simulation/Gravity.cpp' in str(source), sources)
		envCopy = env.Clone()
		envCopy.Append(CCFLAGS='-mstackrealign')
		sources += envCopy.Object('src/simulation/Gravity.cpp')
elif platform == "Darwin":
	sources += ["src/SDLMain.m"]


#Program output name
if GetOption('output'):
	programName = GetOption('output')
else:
	programName = GetOption('renderer') and "render" or "powder"
	if "BIT" in env and env["BIT"] == 64:
		programName += "64"
	if isX86 and GetOption('no-sse'):
		programName += "-legacy"
	if platform == "Windows":
		programName = programName.capitalize()
		programName += ".exe"
	elif platform == "Darwin":
		programName += "-x"

#strip binary after compilation
def strip():
	global programName
	global env
	try:
		os.system("{0} {1}/{2}".format(env['STRIP'] if 'STRIP' in env else "strip", GetOption('builddir'), programName))
	except:
		print("Couldn't strip binary")
if not GetOption('debugging') and not GetOption('symbols') and not GetOption('clean') and not GetOption('help') and not msvc:
	atexit.register(strip)

#Long command line fix for mingw on windows
if compilePlatform == "Windows" and not msvc:
	SetupSpawn(env)

#Once we get here, finally compile
env.Decider('MD5-timestamp')
SetOption('implicit_cache', 1)
t = env.Program(target=programName, source=sources)
Default(t)
