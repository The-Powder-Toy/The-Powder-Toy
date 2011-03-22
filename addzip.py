import zipfile
import os
import os.path
import sys

if(len(sys.argv)>1 and sys.argv[1]=="--clean"):
    print "cleaning"
    for dirpath,dirnames,filenames in os.walk("./src/python/stdlib"):
        for fname in filenames:
            if(fname.endswith(".pyc") or fname.endswith(".pyo")):
                os.remove(os.path.join(dirpath,fname))
    raise SystemExit

print "zipping stdlib"
fid=zipfile.ZipFile("./build/tptPython.zip","w",zipfile.ZIP_DEFLATED)

#ZipFile.write(filename)
files=os.walk("./src/python/stdlib")
num=0
pn=0
for dirpath,dirnames,filenames in files:
    for fname in filenames:
        if(fname.endswith(".py")):
            continue
        fid.write(os.path.join(dirpath,fname))
    num+=1
    if(num-5>=pn):
        pn=num
        print "%d done."%num
    
print "writing zipfile"
fid.close()

raise SystemExit

"""not needed."""
print "generating pystdlib.h"
with open("stdlib.zip","r") as fid:
    with open("./includes/pystdlib.h","w") as outfid:
        outfid.write("unsigned char tpt_console_stdlib[] = {")
        tmp=0
        for char in fid.read():
            outfid.write(hex(ord(char)))
            outfid.write(",")
            tmp+=1
        outfid.write("};\n")
        outfid.write("size_t tpt_console_stdlibsize=%d;"%tmp)
print "done"

