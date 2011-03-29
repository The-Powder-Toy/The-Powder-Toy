import tpt
from tpt import *
import sys
import code
import ctypes
import traceback
DEBUG=False

#print "console module loaded."
#redirect stdout like this:
class logger:
    def write(self,txt):
        txt=txt.strip().split("\n")[-1]
        repr(txt)
        try:
            tpt.log(txt)
        except:#yeah, this happens.
            pass
if(DEBUG==False):
    sys.stdout=logger()
    sys.stderr=logger()
    
#raise SyntaxError()

element={"none":0,"dust":1,"watr":2,"oil":3,"fire":4,"stne":5,"lava":6,"gunp":7,
    "nitr":8,"clne":9,"gas":10,"plex":11,"goo":12,"icei":13,"metl":14,"sprk":15,
    "snow":16,"wood":17,"neut":18,"plut":19,"plnt":20,"acid":21,"void":22,
    "wtrv":23,"cnct":24,"dstw":25,"salt":26,"sltw":27,"dmnd":28,"bmtl":29,
    "brmt":30,"phot":31,"uran":32,"wax":33,"mwax":34,"pscn":35,"nscn":36,
    "lntg":37,"insl":38,"bhol":39,"whol":40,"rbdm":41,"lrbd":42,"ntct":43,
    "sand":44,"glas":45,"ptct":46,"bgla":47,"thdr":48,"plsm":49,"etrd":50,
    "nice":51,"nble":52,"btry":53,"lcry":54,"stkm":55,"swch":56,"smke":57,
    "desl":58,"coal":59,"lo2":60,"o2":61,"inwr":62,"yest":63,"dyst":64,
    "thrm":65,"glow":66,"brck":67,"hflm":68,"firw":69,"fuse":70,"fsep":71,
    "amtr":72,"bcol":73,"pcln":74,"hswc":75,"iron":76,"mort":77,"gol":78,
    "hlif":79,"asim":80,"2x2":81,"dani":82,"amoe":83,"move":84,"pgol":85,
    "dmoe":86,"34":87,"llif":88,"stan":89,"spng":90,"rime":91,"fog":92,
    "bcln":93,"love":94,"deut":95,"warp":96,"pump":97,"fwrk":98,"pipe":99,
    "frzz":100,"frzw":101,"grav":102,"bizr":103,"bizrg":104,"bizrs":105,
    "inst":106,"isoz":107,"iszs":108,"prti":109,"prto":110,"pste":111,
    "psts":112,"anar":113,"vine":114,"invis":115,"equalvel":116,"spawn2":117,
    "spawn":118,"shld1":119,"shld2":120,"shld3":121,"shld4":122,"lolz":123,
    "wifi":124,"filt":125,"aray":126,"bray":127,"stkm2":128,"bomb":129,
    "c5":130,"sing":131,"qrtz":132,"pqrt":133,"seed":134,"maze":135,
    "coag":136,"wall":137,"gnar":138,"repl":139,"myst":140,"boyl":141,
    "lote":142,"frg2":143,"star":144,"frog":145,"bran":146,"wind":147,
    "num":148}


def fork_unblock():
    pass#i need to implement this some day.
def error(ex):
    traceback.print_exc()
    err=traceback.format_exc()
    sys.stdout.write(err)

def clean():
    #add any functions that must be reachable here.
    """copy=["__builtins__","__name__","__doc__","__package__",'tpt','clean',
        'element','fork','_fork','fork_status','fork_unblock','sys']
    handle.glob={}
    for item in copy:
        handle.glob[item]=globals()[item]"""
    handle.glob=globals()
    handle.buf=""

def handle(txt):
    try:
        a=handle.glob
    except:
        clean()
    try:
        _handle(txt)
    except Exception as ex:
        error(ex)
        
def _handle(txt):
    #print "handling '%s'"%txt
    try:
        sys.stdout.write(repr(eval(txt,handle.glob)))
    except:
        try:
            exec txt in handle.glob
        except Exception as ex:
            error(ex)


_extensions=[]
def loadext(fname):
    ext=__import__(fname)
    ext.init()
    _extensions.append(ext)

def keypress(key):
    unload=[]
    for item in _extensions:
        try:
            item.key(key)
        except Exception as ex:
            error(ex)
            unload.append(item)
    for item in unload:
        item.exit()
        _extensions.remove(item)
                

def step():
    unload=[]
    for item in _extensions:
        try:
            item.step()
        except Exception as ex:
            error(ex)
            unload.append(item)
    for item in unload:
        try:
            item.exit()
        except Exception as ex:
            error(ex)
        _extensions.remove(item)
