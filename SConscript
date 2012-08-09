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
AddOption('--win',dest="win",action='store_true',default=False,help="Windows platform target.")
AddOption('--lin',dest="lin",action='store_true',default=False,help="Linux platform target")
AddOption('--macosx',dest="macosx",action='store_true',default=False,help="Mac OS X platform target")
AddOption('--64bit',dest="_64bit",action='store_true',default=False,help="64-bit platform target")
AddOption('--static',dest="static",action="store_true",default=False,help="Static linking, reduces external library dependancies but increased file size")
AddOption('--pthreadw32-static',dest="ptw32-static",action="store_true",default=False,help="Use PTW32_STATIC_LIB for pthreadw32 headers")
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
AddOption('--snapshot',dest="snapshot",default=False,help="Snapshot build.")

if((not GetOption('lin')) and (not GetOption('win')) and (not GetOption('macosx'))):
    print "You must specify a platform to target"
    raise SystemExit(1)

if(GetOption('win')):
    env = Environment(tools = ['mingw'], ENV = os.environ)
else:
    env = Environment(ENV = os.environ)

if GetOption("toolprefix"):
    env['CC'] = GetOption("toolprefix")+env['CC']
    env['CXX'] = GetOption("toolprefix")+env['CXX']
    env['RC'] = GetOption("toolprefix")+env['RC']

#Check for headers and libraries
conf = Configure(env)

try:
    env.ParseConfig('sdl-config --cflags')
    env.ParseConfig('sdl-config --libs')
except:
    conf.CheckLib("SDL")
    if(GetOption("sdl-dir")):
        if not conf.CheckCHeader(GetOption("sdl-dir") + '/SDL.h'):
            print "sdl headers not found or not installed"
            raise SystemExit(1)
        else:
            env.Append(CPPPATH=GetOption("sdl-dir"))

#Find correct lua include dir
try:
    env.ParseConfig('pkg-config --cflags lua5.1')
except:
    if(GetOption("lua-dir")):
        if not conf.CheckCHeader(GetOption("lua-dir") + '/lua.h'):
            print "lua5.1 headers not found or not installed"
            raise SystemExit(1)
        else:
            env.Append(CPPPATH=GetOption("lua-dir"))

#Check for FFT lib
if not conf.CheckLib('fftw3f') and not conf.CheckLib('fftw3f-3'):
    print "libfftw3f not found or not installed"
    raise SystemExit(1)

#Check for Bzip lib
if not conf.CheckLib('bz2'):
    print "libbz2 not found or not installed"
    raise SystemExit(1)

if not conf.CheckCHeader("bzlib.h"):
    print "bzip2 headers not found"
    raise SystemExit(1)

#Check for Lua lib
if not conf.CheckLib('lua') and not conf.CheckLib('lua5.1') and not conf.CheckLib('lua51') and not conf.CheckLib('lua-5.1'):
    print "liblua not found or not installed"
    raise SystemExit(1)

env = conf.Finish();

env.Append(CPPPATH=['src/', 'data/', 'generated/'])
env.Append(CCFLAGS=['-w', '-std=c99', '-fkeep-inline-functions'])
env.Append(LIBS=['pthread', 'm'])
env.Append(CPPDEFINES=["USE_SDL", "LUACONSOLE", "GRAVFFT", "_GNU_SOURCE", "USE_STDINT", "_POSIX_C_SOURCE=200112L"])

if GetOption("ptw32-static"):
    env.Append(CPPDEFINES=['PTW32_STATIC_LIB']);

if(GetOption('static')):
    env.Append(LINKFLAGS=['-static-libgcc'])

if(GetOption('win')):
    openGLLibs = ['opengl32', 'glew32']
    env.Prepend(LIBS=['mingw32', 'ws2_32', 'SDLmain', 'regex'])
    env.Append(LIBS=['winmm', 'gdi32'])
    env.Append(CPPDEFINES=["WIN"])
    env.Append(LINKFLAGS=['-mwindows'])
    if(GetOption('_64bit')):
        env.Append(CPPDEFINES=['__CRT__NO_INLINE'])
        env.Append(LINKFLAGS=['-Wl,--stack=16777216'])
if(GetOption('lin')):
    openGLLibs = ['GL']
    env.Append(LIBS=['X11', 'rt'])
    env.Append(CPPDEFINES=["LIN"])
    if GetOption('_64bit'):
        env.Append(LINKFAGS=['-m64'])
        env.Append(CCFLAGS=['-m64'])
    else:
        env.Append(LINKFLAGS=['-m32'])
        env.Append(CCFLAGS=['-m32'])

if GetOption('_64bit'):
    env.Append(CPPDEFINES=["_64BIT"])

if(GetOption('beta')):
    env.Append(CPPDEFINES='BETA')

if(GetOption('snapshot')):
    env.Append(CPPDEFINES=['SNAPSHOT_ID=' + GetOption('snapshot')])
    env.Append(CPPDEFINES='SNAPSHOT')
else:
    env.Append(CPPDEFINES=["SNAPSHOT_ID=" + str(int(time.time()))])

if(GetOption('save-version')):
    env.Append(CPPDEFINES=['SAVE_VERSION=' + GetOption('major-version')])

if(GetOption('minor-version')):
    env.Append(CPPDEFINES=['MINOR_VERSION=' + GetOption('minor-version')])

if(GetOption('release')):
    env.Append(CCFLAGS=['-O3', '-ftree-vectorize', '-funsafe-math-optimizations', '-ffast-math', '-fomit-frame-pointer', '-funsafe-loop-optimizations', '-Wunsafe-loop-optimizations'])

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
if(GetOption('win')):
    sources += env.RES('resources/powder-res.rc')
sources+=Glob("src/*/*.cpp")
sources+=Glob("src/simulation/elements/*.cpp")
sources+=Glob("src/simulation/tools/*.cpp")
sources+=Glob("generated/ToolClasses.cpp")
sources+=Glob("generated/ElementClasses.cpp")


SetupSpawn(env)

programName = "powder"

if(GetOption('win')):
    programName = "Powder"

if(GetOption('_64bit')):
    programName += "64"

if(not (GetOption('sse2') or GetOption('sse3'))):
    programName += "-legacy"

if(GetOption('macosx')):
    programName += "-x"

if(GetOption('win')):
    programName += ".exe"

env.Command(['generated/ElementClasses.cpp', 'generated/ElementClasses.h'], Glob('src/simulation/elements/*.cpp'), "python generator.py elements $TARGETS $SOURCES")
env.Command(['generated/ToolClasses.cpp', 'generated/ToolClasses.h'], Glob('src/simulation/tools/*.cpp'), "python generator.py tools $TARGETS $SOURCES")
t=env.Program(target=programName, source=sources)
Default(t)

#if(GetOption('release')):
#    StripExecutable(t);

