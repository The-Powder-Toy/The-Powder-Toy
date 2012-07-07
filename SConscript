import os, sys, subprocess

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

AddOption('--opengl-renderer',dest="opengl-renderer",action='store_true',default=False,help="Build with OpenGL renderer support. (requires --opengl)")
AddOption('--opengl',dest="opengl",action='store_true',default=False,help="Build with OpenGL interface support.")
AddOption('--win32',dest="win32",action='store_true',default=False,help="32bit Windows target.")
AddOption('--lin32',dest="lin32",action='store_true',default=False,help="32bit Linux target")
AddOption('--lin64',dest="lin64",action='store_true',default=False,help="64bit Linux target")
AddOption('--release',dest="release",action='store_true',default=False,help="Enable optimisations (Will slow down compiling)")

luaLib = "lua5.1"
luaInclude = "lua5.1"
fftLib = "fftw3f"

if(GetOption('win32')):
    env = Environment(tools = ['mingw'], ENV = os.environ)
else:
    env = Environment(ENV = os.environ)

#Check for headers and libraries
conf = Configure(env)

#Find correct lua include dir
if conf.CheckCHeader('lua.h'):
    luaInclude = "/MinGW/include/"
elif conf.CheckCHeader('lua5.1/lua.h'):
    luaInclude = "/MinGW/include/lua5.1"
elif conf.CheckCHeader('lua51/lua.h'):
    luaInclude = "/MinGW/include/lua51"
elif conf.CheckCHeader('lua/lua.h'):
    luaInclude = "/MinGW/include/lua"
else:
    print "lua5.1 headers not found or not installed"
    raise SystemExit(1)

#Check for SDL headers
if not conf.CheckCHeader('SDL/SDL.h'):
    print "SDL headers not found or not installed"
    raise SystemExit(1)

#Check for SDL lib
if not conf.CheckLib('SDL'):
    print "libSDL not found or not installed"
    raise SystemExit(1)

#Check for FFT lib
if conf.CheckLib('fftw3f'):
	fftLib = "fftw3f"
elif conf.CheckLib('fftw3f-3'):
	fftLib = "fftw3f-3"
else:
    print "libfftw3f not found or not installed"
    raise SystemExit(1)

#Check for Lua lib
if conf.CheckLib('lua'):
    luaLib = 'lua'
elif conf.CheckLib('lua5.1'):
    luaLib = 'lua5.1'
elif conf.CheckLib('lua51'):
    luaLib = 'lua51'
else:
    print "liblua not found or not installed"
    raise SystemExit(1)

env = conf.Finish();

env.Append(CPPPATH=['src/', 'data/', 'generated/', luaInclude, '/MinGW/include/SDL/'])
env.Append(CCFLAGS=['-w', '-std=c99', '-fkeep-inline-functions'])
env.Append(LIBS=['pthread', fftLib, 'm', 'bz2', luaLib, 'SDL'])
env.Append(CPPDEFINES={"_POSIX_C_SOURCE":"200112L"})
env.Append(CPPDEFINES=["USE_SDL", "LUACONSOLE","GRAVFFT","_GNU_SOURCE","USE_STDINT"])


if(GetOption('win32')):
    openGLLibs = ['opengl32', 'glew32']
    env.Prepend(LIBS=['mingw32', 'ws2_32', 'SDLmain', 'regex'])
    env.Append(CPPDEFINES=["WIN32"])
if(GetOption('lin32') or GetOption('lin64')):
    openGLLibs = ['GL']
    env.Append(LIBS=['X11', 'rt'])
    if GetOption('lin32'):
        env.Append(CPPDEFINES=["LIN32"])
    else:
        env.Append(CPPDEFINES=["LIN64"])


if(GetOption('release')):
    ev.Append(CCFLAGS='-O3')

if(GetOption('opengl')):
    env.Append(CPPDEFINES=["OGLI", "PIX32OGL"])
    env.Append(LIBS=openGLLibs)

if(GetOption('opengl-renderer') and GetOption('opengl-renderer')):
    env.Append(CPPDEFINES=["OGLR"])
elif(GetOption('opengl-renderer')):
    print "opengl-renderer requires opengl"
    raise SystemExit(1)

sources=Glob("src/*.cpp")
if(GetOption('win32')):
	sources += env.RES('resources/powder-res.rc')
sources+=Glob("src/*/*.cpp")
sources+=Glob("src/simulation/elements/*.cpp")
sources+=Glob("src/simulation/tools/*.cpp")
sources+=Glob("generated/*.cpp")

SetupSpawn(env)

t=env.Program(target='powder', source=sources)
Default(t)

