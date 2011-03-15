import sys
import subprocess
if len(sys.argv)<2:
    print("usage: syntaxcheck.py <filename>")
    raise SystemExit

fname=sys.argv[1]
cppargs=["-E"]+sys.argv[2:]

p=subprocess.Popen(["gcc",fname]+cppargs,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
ret=p.communicate()
if len(ret[1])>0:
    print("errors hapened!")
    print(repr(ret[1].decode()))
    raise SystemExit(-1)

print(len(ret[0].decode()))
#python syntaxcheck.py ./src/main.c -DINTERNAL -opowder -w -std=c99 -D_POSIX_C_SOURCE=200112L -Iincludes/  -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -lSDL -lm -lbz2        -lpython2.7 -lm -L/usr/lib/python2.7/config -I/usr/include/python2.7 -march=native -DX86 -DX86_SSE3 -msse3 src/*.c src/elements/*.c -DLIN64