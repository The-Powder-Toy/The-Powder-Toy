import sys
import os.path
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