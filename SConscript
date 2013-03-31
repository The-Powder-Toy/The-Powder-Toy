import os, sys, subprocess, time


##Fix for long command line - http://scons.org/wiki/LongCmdLinesOnWin32
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

AddOption('--opengl',dest="opengl",action='store_true',default=False,help="Build with OpenGL interface support.")
AddOption('--opengl-renderer',dest="opengl-renderer",action='store_true',default=False,help="Build with OpenGL renderer support. (requires --opengl)")
AddOption('--renderer',dest="renderer",action='store_true',default=False,help="Save renderer")
AddOption('--win',dest="win",action='store_true',default=False,help="Windows platform target.")
AddOption('--lin',dest="lin",action='store_true',default=False,help="Linux platform target")
AddOption('--macosx',dest="macosx",action='store_true',default=False,help="Mac OS X platform target")
AddOption('--rpi',dest="rpi",action='store_true',default=False,help="Raspbain platform target")
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

AddOption('--debugging', dest="debug", action="store_true", default=False, help="Enable debug options")
AddOption('--beta',dest="beta",action='store_true',default=False,help="Beta build.")
AddOption('--save-version',dest="save-version",default=False,help="Save version.")
AddOption('--minor-version',dest="minor-version",default=False,help="Minor version.")
AddOption('--build-number',dest="build-number",default=False,help="Build number.")
AddOption('--snapshot',dest="snapshot",action='store_true',default=False,help="Snapshot build.")
AddOption('--snapshot-id',dest="snapshot-id",default=False,help="Snapshot build ID.")
AddOption('--stable',dest="stable",default=True,help="Non snapshot build")

AddOption('--aao', dest="everythingAtOnce", action='store_true', default=False, help="Compile the whole game without generating intermediate objects (very slow), enable this when using compilers like clang or mscc that don't support -fkeep-inline-functions")

if((not GetOption('lin')) and (not GetOption('win')) and (not GetOption('rpi')) and (not GetOption('macosx'))):
	print "You must specify a platform to target"
	raise SystemExit(1)

if(GetOption('win')):
	env = Environment(tools = ['mingw'], ENV = os.environ)
else:
	env = Environment(tools = ['default'], ENV = os.environ)

if GetOption("toolprefix"):
	env['CC'] = GetOption("toolprefix")+env['CC']
	env['CXX'] = GetOption("toolprefix")+env['CXX']
	if GetOption('win'):
		env['RC'] = GetOption("toolprefix")+env['RC']

#Check for headers and libraries
if not GetOption("macosx"):
	conf = Configure(env)

	try:
		env.ParseConfig('sdl-config --cflags')
		env.ParseConfig('sdl-config --libs')
	except:
		if not conf.CheckLib("SDL"):
			print "libSDL not found or not installed"
			raise SystemExit(1)
			
		if(GetOption("sdl-dir")):
			if not conf.CheckCHeader(GetOption("sdl-dir") + '/SDL.h'):
				print "sdl headers not found or not installed"
				raise SystemExit(1)
			else:
				env.Append(CPPPATH=[GetOption("sdl-dir")])

	#Find correct lua include dir
	try:
		env.ParseConfig('pkg-config --cflags lua5.1')
	except:
		if(GetOption("lua-dir")):
			if not conf.CheckCHeader(GetOption("lua-dir") + '/lua.h'):
				print "lua5.1 headers not found or not installed"
				raise SystemExit(1)
			else:
				env.Append(CPPPATH=[GetOption("lua-dir")])

	#Check for FFT lib
	if not conf.CheckLib('fftw3f') and not conf.CheckLib('fftw3f-3'):
		print "libfftw3f not found or not installed"
		raise SystemExit(1)

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

	#Check for Lua lib
	if not GetOption("macosx"):
		if not conf.CheckLib('lua5.1') and not conf.CheckLib('lua-5.1') and not conf.CheckLib('lua51') and not conf.CheckLib('lua'):
			print "liblua not found or not installed"
			raise SystemExit(1)

	env = conf.Finish();
else:
	env.Append(LIBS=['z', 'bz2', 'fftw3f'])

env.Append(CPPPATH=['src/', 'data/', 'generated/'])
env.Append(CCFLAGS=['-w', '-std=c++98', '-fkeep-inline-functions'])
env.Append(LIBS=['pthread', 'm'])
env.Append(CPPDEFINES=["LUACONSOLE", "GRAVFFT", "_GNU_SOURCE", "USE_STDINT", "_POSIX_C_SOURCE=200112L"])

if GetOption("ptw32-static"):
	env.Append(CPPDEFINES=['PTW32_STATIC_LIB']);

if(GetOption('static')):
	env.Append(LINKFLAGS=['-static-libgcc'])

if(GetOption('renderer')):
	env.Append(CPPDEFINES=['RENDERER'])
else:
	env.Append(CPPDEFINES=["USE_SDL"])

if(GetOption('rpi')):
        if(GetOption('opengl')):
                env.ParseConfig('pkg-config --libs glew gl glu')
        openGLLibs = ['GL']
        env.Append(LIBS=['X11', 'rt'])
        env.Append(CPPDEFINES=["LIN"])
		
if(GetOption('win')):
	openGLLibs = ['opengl32', 'glew32']
	env.Prepend(LIBS=['mingw32', 'ws2_32', 'SDLmain', 'regex'])
	env.Append(CCFLAGS=['-std=gnu++98'])
	env.Append(LIBS=['winmm', 'gdi32'])
	env.Append(CPPDEFINES=["WIN"])
	env.Append(LINKFLAGS=['-mwindows'])
	if(GetOption('_64bit')):
		env.Append(CPPDEFINES=['__CRT__NO_INLINE'])
		env.Append(LINKFLAGS=['-Wl,--stack=16777216'])
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
if(GetOption('macosx')):
	env.Append(CPPDEFINES=["MACOSX"])
	env.Append(CCFLAGS=['-I/Library/Frameworks/SDL.framework/Headers'])
	env.Append(CCFLAGS=['-I/Library/Frameworks/Lua.framework/Headers'])
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

sources=Glob("src/*.cpp")
if(GetOption('macosx')):
	sources +=["SDLMain.m"]
if(GetOption('win')):
	sources += env.RES('resources/powder-res.rc')
sources+=Glob("src/*/*.cpp")
sources+=Glob("src/simulation/elements/*.cpp")
sources+=Glob("src/simulation/tools/*.cpp")
sources+=Glob("src/client/requestbroker/*.cpp")

#for source in sources:
#	print str(source)

if(GetOption('win')):
	sources = filter(lambda source: not 'src\\simulation\\Gravity.cpp' in str(source), sources)
	sources = filter(lambda source: not 'src/simulation/Gravity.cpp' in str(source), sources)

SetupSpawn(env)

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

if(GetOption('release')):
	if GetOption('macosx'):
		env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer'])
	else:
		env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer', '-funsafe-loop-optimizations', '-Wunsafe-loop-optimizations'])


if(GetOption('pythonver')):
	pythonVer = GetOption('pythonver')
elif(GetOption('lin')):
	pythonVer = "python2"
else:
	pythonVer = "python"

if(GetOption('win')):
	envCopy = env.Clone()
	envCopy.Append(CCFLAGS=['-mincoming-stack-boundary=2'])
	sources+=envCopy.Object('src/simulation/Gravity.cpp')

env.Command(['generated/ElementClasses.cpp', 'generated/ElementClasses.h'], Glob('src/simulation/elements/*.cpp'), pythonVer + " generator.py elements $TARGETS $SOURCES")
sources+=Glob("generated/ElementClasses.cpp")

env.Command(['generated/ToolClasses.cpp', 'generated/ToolClasses.h'], Glob('src/simulation/tools/*.cpp'), pythonVer + " generator.py tools $TARGETS $SOURCES")
sources+=Glob("generated/ToolClasses.cpp")

env.Decider('MD5')
t=env.Program(target=programName, source=sources)
Default(t)
