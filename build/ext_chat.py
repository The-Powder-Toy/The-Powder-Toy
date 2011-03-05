import tpt
import sys
import time
import socket

HOST="irc.freenode.net"
PORT=6667
NICK="doxin[tpt]"
IDENT="doxin"
REALNAME="lieuwe"
CHANNEL="#powder"
readbuffer=""

def raw(s,st):
    s.send("%s\n\r"%st)

frame=0
s=None
rec=[("connected.",255,0,0,128)]
readbuffer=""


def key(key) :
    #print "got %s"%key
    pass

def step():
    global frame,s,rec,readbuffer
    frame+=1
    if(frame==1):
        tpt.console_close()
    if(frame==2):
        tpt.draw_fillrect(0,0,612,384,0,0,0,128)
        tpt.draw_text(32,32,"opening connection\nhold on to your pants.",255,255,255)
    if(frame==3):
        s=socket.socket( )
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
                #print line
                line=line.split()
                if(line[1]=="PRIVMSG"):
                    #:doxin!~lieuwe@unaffiliated/lieuwe PRIVMSG doxin[tpt] :some shit
                    frm=line[0][1:].partition("!")[0]
                    msg=' '.join(line[3:])[1:]
                    tmp=["<",frm,"> ",msg]
                    if(line[2]==NICK):
                        rec.append((''.join(tmp),255,255,255,255))
                    else:
                        rec.append((''.join(tmp),255,255,255,128))
                if(line[0]=="PING"):
                    raw(s,"PONG %s"%line[1])
    
        yy=32
        if(len(rec)>20):
            rec=rec[20:]
        for item in rec:
            tpt.draw_text(8,yy,item[0],item[1],item[2],item[3],item[4])
            yy+=8

