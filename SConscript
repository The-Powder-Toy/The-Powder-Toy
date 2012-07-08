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
AddOption('--lua-dir',dest="lua-dir",default=False,help="Directory for lua includes")
AddOption('--sdl-dir',dest="sdl-dir",default=False,help="Directory for SDL includes")


if(GetOption('win32')):
    env = Environment(tools = ['mingw'], ENV = os.environ)
else:
    env = Environment(ENV = os.environ)

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

#Check for Lua lib
if not conf.CheckLib('lua') and not conf.CheckLib('lua5.1') and not conf.CheckLib('lua51'):
    print "liblua not found or not installed"
    raise SystemExit(1)

env = conf.Finish();

env.Append(CPPPATH=['src/', 'data/', 'generated/'])
env.Append(CCFLAGS=['-w', '-std=c99', '-fkeep-inline-functions'])
env.Append(LIBS=['pthread', 'm', 'bz2'])
env.Append(CPPDEFINES={"_POSIX_C_SOURCE": "200112L"})
env.Append(CPPDEFINES=["USE_SDL", "LUACONSOLE", "GRAVFFT", "_GNU_SOURCE", "USE_STDINT"])


if(GetOption('win32')):
    openGLLibs = ['opengl32', 'glew32']
    env.Prepend(LIBS=['mingw32', 'ws2_32', 'SDLmain', 'regex'])
    env.Append(CPPDEFINES=["WIN32"])
if(GetOption('lin32') or GetOption('lin64')):
    openGLLibs = ['GL']
    env.Append(LIBS=['X11', 'rt'])
    if GetOption('lin32'):
        env.Append(LINKFLAGS=['-m32'])
        env.Append(CCFLAGS=['-m32'])
        env.Append(CPPDEFINES=["LIN32"])
    else:
        env.Append(LINKFAGS=['-m64'])
        env.Append(CCFLAGS=['-m64'])
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

