import tpt
import sys
import time
import socket
import tpt_console

HOST="irc.freenode.net"
PORT=6667
name=tpt.get_name()
print "name is %s"%repr(name)
if(name==""):
    #fuck. abort?
    raise SystemExit("please log in!")
NICK=name+"[tpt]"
IDENT=name+"[tpt]"
REALNAME=name
CHANNEL="#foobar7"
readbuffer=""

def raw(s,st):
    s.send("%s\n\r"%st)

def init():
    global frame,s,rec,readbuffer,namelist
    frame=0
    s=None
    rec=[("connected.",255,0,0,128)]
    readbuffer=""
    namelist=[]

def exit():
    raw(s,"QUIT")
    s.close()

def console_handle(txt):
    """
    :doxin!~lieuwe@unaffiliated/lieuwe JOIN :#foobar7
    :doxin!~lieuwe@unaffiliated/lieuwe PRIVMSG #foobar7 :there
    :doxin!~lieuwe@unaffiliated/lieuwe PRIVMSG #foobar7 :ACTION is fat
    """
    lst=txt.split(" ")
    if(lst[0]=="/me"):
        rec.append(("%s %s"%(NICK,txt[4:]),255,0,255,128))
        raw(s,"PRIVMSG %s :\x01ACTION %s\x01"%(CHANNEL,txt[4:]))
        tpt.console_close()
    else:
        rec.append(("<%s>: %s"%(NICK,txt),255,255,0,128))
        raw(s,"PRIVMSG %s :%s"%(CHANNEL,txt))
        tpt.console_close()

def key(key) :
    #print "got %s"%key
    pass

def step():
    global frame,s,rec,readbuffer,namelist
    frame+=1
    if(frame==1):
        tpt.console_close()
        #lets see if we can seize the console:
        tpt_console._handle=console_handle
    if(frame==2):
        tpt.draw_fillrect(0,0,612,384,0,0,0,128)
        tpt.draw_text(32,32,"opening connection\nhold on to your pants.",255,255,255)
    if(frame==3):
        s=socket.socket()
        s.settimeout(5)
        s.connect((HOST, PORT))
        raw(s,"NICK %s" % NICK)
        raw(s,"USER %s %s bla :%s" % (IDENT, HOST, REALNAME))
        s.settimeout(0)
    if(frame==120):
        raw(s,"JOIN %s"%CHANNEL)
        rec.append(("joined",255,0,0,255))
    if(frame>=3):
        try:
            readbuffer=readbuffer+s.recv(1024)
        except IOError:
            pass
        else:
            temp=readbuffer.split("\n")
            readbuffer=temp.pop()

            for line in temp:
                line=line.strip()
                print repr(line)
                line=line.split()
                if(line[1]=="PRIVMSG"):
                    #:doxin!~lieuwe@unaffiliated/lieuwe PRIVMSG doxin[tpt] :some shit
                    frm=line[0][1:].partition("!")[0]
                    msg=' '.join(line[3:])[1:]
                    tmp=["<",frm,"> ",msg]
                    if(msg[0]=="\x01" and msg[-1]=="\x01"):
                        msg=msg[8:-1]#ACTION
                        tmp=[frm," ",msg]
                    if(line[2]==NICK):
                        rec.append((''.join(tmp),255,255,255,255))
                    else:
                        rec.append((''.join(tmp),255,255,255,128))
                elif(line[0]=="PING"):
                    raw(s,"PONG %s"%line[1])
                elif(line[1]=="353"):
                    #:leguin.freenode.net 353 doxin[tpt] = #powder :doxin[tpt] ZebraineZ _-_Rafael_-_ doxin bildramer BlueMaxima TheRazorsEDGE raj-k webb|AP where @devast8a Merbo FrozenKnight EppyMoon EvilJStoker Mortvert SpitfireWP @frankbro Ares
                    names=line[4:]
                    namelist=[]
                    for item in names:
                        item=item.strip()
                        r=255
                        g=255
                        b=255
                        if(item[0]==":"):
                            item=item[1:]
                        elif(item[0]=="@"):
                            g=0
                            b=0
                        namelist.append((item,r,g,b,128))
                elif(line[1]=="JOIN"):
                    #':savask!~savask@95-190-25-195-xdsl-dynamic.kuzbass.net JOIN :#powder'
                    tmp=line[0][1:].partition("!")[0]
                    namelist.append((tmp,255,255,255,128))
                    rec.append(("%s joined"%name,0,255,0,128))
                elif(line[1]=="PART"):
                    #':savask!~savask@95-190-25-195-xdsl-dynamic.kuzbass.net PART #powder :"Leaving."'
                    tmp=line[0][1:].partition("!")[0]
                    msg=' '.join(line[2:])[1:]
                    rem=None
                    for item in namelist:
                        if(item[0]==tmp or item[0]=="@"+tmp):
                            rem=item
                    rec.append(("%s parted: %s"%(name,msg),0,255,0,128))
                    if(rem!=None):
                        namelist.remove(rem)
                elif(line[1]=="NICK"):
                    #:doxin!~lieuwe@unaffiliated/lieuwe NICK :d0x1n
                    tmp=line[0][1:].partition("!")[0]
                    rem=None
                    for item in namelist:
                        if(item[0]==tmp or item[0]=="@"+tmp):
                            rem=item
                    rec.append(("%s is now known as %s"%(name,line[2]),0,255,0,128))
                    if(rem!=None):
                        if(rem[0][0]=="@"):
                            namelist.append(("@"+line[2][1:],rem[1],rem[2],rem[3],rem[4]))
                        else:
                            namelist.append((line[2][1:],rem[1],rem[2],rem[3],rem[4]))
                        namelist.remove(rem)


                    
    
        yy=32
        if(len(rec)>20):
            rec=rec[-20:]
        for item in rec:
            tpt.draw_text(8,yy,item[0],item[1],item[2],item[3],item[4])
            yy+=8
        #print namelist
        yy=32
        for item in namelist:
            tpt.draw_text(604-tpt.get_width(item[0]),yy,item[0],item[1],item[2],item[3],item[4])
            yy+=8

