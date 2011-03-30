import sys
import os.path
import compileall
class dummyfile():
    def write(self,txt):
        pass
def log(*args):
    try:
        log.a
    except:
        log.a=sys.stdout
    log.a.write(" ".join(args)+"\n")
log()
sys.stdout=dummyfile()

path=os.path.join(sys.exec_prefix,"include","python%s"%sys.version[:3])
#print "headers at ",repr(path)
#-lpython2.3 -lm -L/usr/lib/python2.3/config
args="-lpython%s -lm -L%s"%(sys.version[:3],os.path.join(sys.exec_prefix,"lib","python%s"%sys.version[:3],"config"))
log(args,"-I%s"%path)

path=os.path.join(sys.exec_prefix,"include")
args="-lpython%s -lm -L%s"%(sys.version[:3],os.path.join(sys.exec_prefix,"lib","config"))#,"python%s"%sys.version[:3]
log(args,"-I%s"%path)

ext=False
"""#see if we're on 64bit.
with open("./includes/defines.h") as fid:
    for line in fid:
        if(line.startswith("#define") and line.count("PYEXT")>0):
            print "using external console.py"
            ext=True"""
#print sys.argv
if(len(sys.argv)>=2 and sys.argv[1]=="--ext"):
    ext=True
#raw_input("")

if(ext):
    log("external")
    #raw_input(sys.argv)
    with open("./src/python/tpt_console.py") as fid:
        consolepy=fid.read()
    script="""
import tempfile,os.path,sys
dir=tempfile.gettempdir()
sys.path.append(dir)
tmp=%s
print "making console.py @ %%s"%%os.path.join(dir,"tpt_console.py")
with open(os.path.join(dir,"tpt_console.py"),"w") as fid:
    fid.write(tmp)
    """%repr(consolepy)
    tmp=[hex(ord(char)) for char in script]
    out=["unsigned char tpt_console_py[] = {",','.join(tmp),"};"]
    with open("./includes/pyconsole.h","w") as fid:
        fid.write(''.join(out))
else:
    log("internal")
    #raw_input(sys.argv)
    #unsigned char tpt_console_pyc[] = { 0x1B, 0x57};
    lst=[]
    compileall.compile_dir("./src/python", force=0)

    print "generating pyconsole.h"

    fname="./src/python/tpt_console.pyc"
    try:
        fid=open(fname,"r")
    except IOError:
        fname="./src/python/tpt_console.pyo"
    finally:
        fid.close()

    with open(fname,"r") as fid:
        for char in fid.read():
            lst.append(hex(ord(char)))
    tmp=",".join(lst)
    out=''.join(["#include <Python.h>\nunsigned char tpt_console_pyc[] = {",tmp,"};"])
    with open("./includes/pyconsole.h","w") as fid:
        fid.write(out)
print "done"
