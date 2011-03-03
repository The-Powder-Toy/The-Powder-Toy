import sys
import os.path
import compileall
path=os.path.join(sys.exec_prefix,"include","python%s"%sys.version[:3])
#print "headers at ",repr(path)
#-lpython2.3 -lm -L/usr/lib/python2.3/config
args="-lpython%s -lm -L%s"%(sys.version[:3],os.path.join(sys.exec_prefix,"lib","python%s"%sys.version[:3],"config"))
print " linux args are"
print args,"-I%s"%path

path=os.path.join(sys.exec_prefix,"include")
args="-lpython%s -lm -L%s"%(sys.version[:3],os.path.join(sys.exec_prefix,"lib","config"))#,"python%s"%sys.version[:3]
print "\n windows args are"
print args,"-I%s"%path

#unsigned char tpt_console_pyc[] = { 0x1B, 0x57};
lst=[]
compileall.compile_dir("./src/python", force=1)

print "\n\n\ngenerating pyconsole.h"

fname="./src/python/tpt_console.pyc"
try:
    fid=open(fname,"r")
except IOError:
    fname="./src/python/tpt_console.pyo"
finally:
    fid.close()

with open(fname,"r") as fid:
    for char in fid.read():
        lst.append("0x"+hex(ord(char)))
tmp=",".join(lst)
out=''.join(["unsigned char tpt_console_pyc[] = {",tmp,"};"])
with open("./includes/pyconsole.h","w") as fid:
    fid.write(out)
print "done"
    