-- ************************************************************************
-- ** Copyright 2012 mniip
-- **               mniip@yandex.ru
-- ************************************************************************
-- ** This program is free software: you can redistribute it and/or modify
-- ** it under the terms of the GNU General Public License as published by
-- ** the Free Software Foundation, either version 3 of the License, or
-- ** (at your option) any later version.
-- ** 
-- ** This program is distributed in the hope that it will be useful,
-- ** but WITHOUT ANY WARRANTY; without even the implied warranty of
-- ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- ** GNU General Public License for more details.
-- ** 
-- ** You should have received a copy of the GNU General Public License
-- ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
-- ************************************************************************

version="xsTPTIRC v1.1 beta by mniip"

local n=tpt.get_name()
if not n or n=="" then
 error"you MUST be logged in to use xsTPTIrc"
end
local mynick=n

local conf={}

local function loadpresets()
 local f=io.open"irc.conf"
 if not f then
  f=io.open("irc.conf","w")
  f:write('nick="'..mynick..[["
server="irc.freenode.net"
port=6667
autojoin={"#powder"}
wcolor={255,255,0}
tabtextcolor={255,255,255}
ctabcolor={255,0,255,63}
nickcolor={0,255,255}
textcolor={127,255,255}
opcolor={255,0,0}
voicecolor={64,192,64}
mycolor={255,255,255}
mecolor={128,128,128}
chancolor={255,127,0}
]])
  f:close()
  f=io.open"irc.conf"
 end
 local s=f:read"*a"
 f:close()
 f=loadstring(s)
 if f then
  setfenv(f,conf)
  f()
 end
 mynick=tostring(conf.nick) or n
 conf.server=tostring(conf.server) or "irc.freenode.net"
 conf.port=tonumber(conf.port) or 6667
 conf.autojoin=type(conf.autojoin)=="table" and conf.autojoin or {"#powder"}
end

loadpresets()

pcall(package.loadlib,"socket","luasocket.dll")

local loader=package.loadlib("luasocket.dll","luaopen_socket_core")
if loader then
 loader()
else
 socket=require("socket")
end


local drawtext=function(x,y,s,...)return pcall(tpt.drawtext,x,y,tostring(s):gsub("\15(.)(.)(.)",
function(r,g,b)return "\15"..(r=="\0"and"\1"or r)..(g=="\0"and"\1"or g)..(b=="\0"and"\1"or b)
end),...)end

local function textwidth(s)
 return select(2,pcall(tpt.textwidth,s))
end

local function r(t)
 return "\15"..string.char(unpack(t))
end

local tabs
local ctab=1
local edit=""
local cpos=0
local cursorbl=0
local history={[0]=""}
local histp=0

local wx,wy=10,10
local wcolor=conf.wcolor or {255,255,0}
local tabtextcolor=conf.tabtextcolor or {255,255,255}
local ctabcolor=conf.ctabcolor or {255,0,255,63}
local nickcolor=conf.nickcolor or {0,255,255}
local textcolor=conf.textcolor or {127,255,255}
local opcolor=conf.opcolor or {255,0,0}
local voicecolor=conf.voicecolor or {64,192,64}
local mycolor=conf.mycolor or {255,255,255}
local mecolor=conf.mecolor or {128,128,128}
local chancolor=conf.chancolor or {255,127,0}

local function ui()
 tpt.fillrect(wx,wy,600,360,0,0,0,240)
 -- Window
tpt.drawrect(wx,wy,600,360,unpack(wcolor))
 -- Topic
 tpt.drawrect(wx,wy+12,484,12,unpack(wcolor))
 local f=#tabs[ctab].topic
 while textwidth(tabs[ctab].topic:sub(1,f))>480 do
  f=f-1
 end
 drawtext(wx+2,wy+14,tabs[ctab].topic:sub(1,f),unpack(wcolor))
 tpt.drawrect(wx+484,wy+12,12,12,unpack(wcolor))
 -- Tabs
 if ctab>#tabs then
  ctab=#tabs
 end
 local ox=0
 for i=1,#tabs do
  tpt.drawrect(wx+ox,wy,#tabs[i].name*6+2,12,unpack(wcolor))
  drawtext(wx+ox+2,wy+2,tabs[i].name,unpack(tabtextcolor))
  if i==ctab then 
   tpt.fillrect(wx+ox,wy,#tabs[i].name*6+3,12,unpack(ctabcolor))
  end
  ox=ox+#tabs[i].name*6+2
 end
 -- Nicklist
 tpt.drawrect(wx+496,wy+12,104,336,unpack(wcolor))
 for i,v in ipairs(tabs[ctab].nicks) do
  if i<=33 then
   if v:gsub("[^@+]","")=="@" then
    drawtext(wx+498,wy+4+10*i,v,unpack(opcolor))
   elseif v:gsub("[^@+]","")=="+" then
    drawtext(wx+498,wy+4+10*i,v,unpack(voicecolor))
   else
    drawtext(wx+498,wy+4+10*i,v,unpack(nickcolor))
   end
  end
 end
 tpt.drawrect(wx,wy+24,496,324,unpack(wcolor))
 tpt.drawrect(wx+587,wy+348,13,12,unpack(wcolor))
 drawtext(wx+589,wy+351,">>",unpack(wcolor))
 -- Text
 for i=1,math.min(#tabs[ctab].text,32) do
  drawtext(wx+2,wy+348-i*10,r(textcolor)..tabs[ctab].text[i])
 end
 drawtext(wx+2,wy+351,edit,unpack(textcolor))
 if math.floor(cursorbl/10)%2==1 then
  tpt.fillrect(wx+2+textwidth(edit:sub(1,cpos)),wy+354,textwidth((edit:sub(cpos+1,cpos+1).." "):sub(1,1))+2,4,unpack(textcolor))
 end
 cursorbl=cursorbl+1
end

local function isinrect(x,y,xz,yz,w,h)
 return x>=xz and y>=yz and x<xz+w and y<yz+h
end

local function wprint(n,s)
 local t={[0]="\1\1\1","\255\255\255","\1\1\128","\1\128\1","\255\1\1","\128\1\1","\128\1\128","\255\128\1","\255\255\1","\1\255\1","\1\128\128","\1\255\255"}
 s=s:gsub("\3(%d%d?)",function(i)return t[tonumber(i)]and"\15"..t[tonumber(i)]or""end)
 while #s>0 do
  print(s)
  local f=#s
  while textwidth(s:sub(1,f))>493 do
   f=f-1
  end
  table.insert(tabs[n].text,1,s:sub(1,f))
  s=s:sub(f+1)
 end
end

local function findtab(s)
 for i,v in ipairs(tabs) do
  if s:lower()==v.name:lower() then
   return i
  end
 end
end

local function resetwindows()
 tabs={{name="-server-",
 nicks={},
 topic=version,
 text={"--- "..version.." ---","The idea becomes real","Line by line"}}}
 ctab=1
 edit=""
 cpos=0
end

local function send()
 table.insert(history,1,edit)
 cpos=0
 if edit:match"^/" then
  local command,params=edit:match"/(%S+)%s*(.*)"
  if command:lower()=="clear" then
   if params~="" then
    (tabs[findtab(params)or tonumber(params)or -1]or {}).text={}
   else
     tabs[ctab].text={}
   end
  elseif command:lower()=="close" or command:lower()=="part" then
   if params~="" then
    table.remove(tabs,findtab(params)or tonumber(params) or 0)
   else
    table.remove(tabs,ctab)
   end
  elseif command:lower()=="open" or command:lower()=="query" then
   table.insert(tabs,{name=params,nicks={},topic="",text={}})
  elseif command:lower()=="ctcp" then
   local who,what=params:match"(%S+)%s*(.*)"
   c:send("PRIVMSG "..who.." :\1"..what.."\1\n")
  elseif command:lower()=="me" then
   c:send("PRIVMSG "..tabs[ctab].name.." :\1ACTION "..params.."\1\n")
   wprint(ctab,r(mecolor).."* "..mynick.." "..params)
  elseif command:lower()=="server" then
   if c then
    pcall(c.send,c,"QUIT\n")
    pcall(c.close,c)
    c=nil
   end
   resetwindows()
   c=socket.tcp()
   if params:find"%S" then
    local server,port=params:match"(%S+)%s+(%S+)"
    server=server or params
    port=port or 6667
    c:connect(server,tonumber(port))
   else
    c:connect(conf.server,conf.port)
   end
   c:settimeout(0)
  else
   c:send(edit:match"/(.*)".."\n")
   wprint(ctab,">"..edit)
  end
 else
  c:send("PRIVMSG "..tabs[ctab].name.." :"..edit.."\n")
  wprint(ctab,r(mycolor).."<"..mynick.."> "..edit)
 end
 edit=""
end

local function mouse(x,y,b,n)
 if n==1 then
  -- Tabs
  local ox=0
  for i=1,#tabs do
   if isinrect(x,y,wx+ox,wy,#tabs[i].name*6+3,12) then
    ctab=i
   end
   ox=ox+#tabs[i].name*6+2
  end
  if isinrect(x,y,wx+587,wy+348,13,12) then
   send()
  end
 end
 return false
end

local function resortnicks(n)
 local d=tabs[n].nicks
 local a,b={},{}
 for _,v in ipairs(d) do
  a[#a+1],b[#b+1]=v:match"([@+]?)(.*)"
 end
 local op,vc,rg={},{},{}
 for i,v in ipairs(a) do
  if v=="@" then
   table.insert(op,b[i])
  elseif v=="+" then
   table.insert(vc,b[i])
  else
   table.insert(rg,b[i])
  end
 end
 local function f(a,b)return a:lower()<b:lower() end
 table.sort(op,f)
 table.sort(vc,f)
 table.sort(rg,f)
 for i,_ in ipairs(d) do
  d[i]=nil
 end
 for _,v in ipairs(op) do
  table.insert(d,"@"..v)
 end
 for _,v in ipairs(vc) do
  table.insert(d,"+"..v)
 end
 for _,v in ipairs(rg) do
  table.insert(d,v)
 end
end

local function receive()
 local s,e=c:receive"*l"
 if s then
  s=s:gsub("\n","")
  local sender,command,params=s:match":(%S+) (%S+)%s*(.*)"
  if not sender then
   command,params=s:match"(%S+)%s*(.*)"
  end
  if s:match"(.-)NOTICE %* :%*%*%* Checking Ident" then
   c:send("NICK "..mynick.."\n")
   c:send("USER xstptirc _ _ :TPT login:"..n.."\n")
  elseif command:lower()=="join" then
   local channel=params:match"(%S+)"
   local nick=sender:match"([^!]+)"
   if nick:lower()==mynick:lower() then
    table.insert(tabs,{name=channel,nicks={},topic="",text={}})
    ctab=#tabs
    wprint(1,r(chancolor).."*** You have joined "..channel)
   else
    table.insert(tabs[findtab(channel)or 1].nicks,nick)
   end
   wprint(findtab(channel)or 1,r(chancolor).."*** "..nick.." has joined "..channel)
   resortnicks(findtab(channel)or 1)
  elseif command=="376" then
   for _,v in ipairs(conf.autojoin) do
    c:send("JOIN "..v.."\n")
   end
  elseif command:lower()=="privmsg" then
   local channel,text=params:match"(%S+) :(.*)"
   local nick=sender:match"([^!]+)"
   if text:find"\1" then
    local command,params=text:match"\1(%S+)%s*(.*)\1"
    if command:lower()=="action" then
     if channel:lower()==mynick:lower() then
      if not findtab(nick) then
       table.insert(tabs,{name=nick,nicks={},topic="Query with "..nick,text={}})
      end
     channel=nick
     end
     wprint(findtab(channel)or 1,"* "..nick.." "..params)
    else
     local ctcp={ping=function(p)return p end,time=function(p)return os.date"%c"end,version=function(p)return version end,userinfo=function(p)return "PING TIME VERSION USERINFO"end}
     if ctcp[command:lower()] then
      c:send("NOTICE "..nick.." :\1"..command.." "..ctcp[command:lower()](params).."\1\n")
     end
    end
   else
    if channel:lower()==mynick:lower() then
     if not findtab(nick) then
      table.insert(tabs,{name=nick,nicks={},topic="Query with "..nick,text={}})
     end
     channel=nick
    end
    for _,v in ipairs(tabs[ctab].nicks) do
     if v:gsub("[@+]",""):lower()==nick:lower() then
      if v:gsub("[^@+]","")=="@" then
       nick=r(opcolor)..nick..r(textcolor)
      elseif v:gsub("[^@+]","")=="+" then
       nick=r(voicecolor)..nick..r(textcolor)
      end
     end
    end
    wprint(findtab(channel)or 1,"<"..nick.."> "..text)
   end
  elseif command=="433" then
   mynick=mynick.."-"
   c:send("NICK "..mynick.."\n")
  elseif command=="332" then
   local channel,topic=params:match"%S+ (%S+) :(.*)"
   tabs[findtab(channel)or 1].topic=topic
   wprint(findtab(channel)or 1,r(chancolor).."*** Topic for "..channel.." is: ")
   wprint(findtab(channel)or 1,r(chancolor)..topic)
  elseif command=="333" then
   local channel,whom,when=params:match"%S+ (%S+) (%S+) (%S+)"
   wprint(findtab(channel)or 1,r(chancolor).."*** Topic set by "..whom.." at "..os.date("%c",when))
  elseif command=="353" then
   local channel,nicks=params:match"%S+ = (%S+) :(.*)"
   for nick in nicks:gmatch"(%S+)" do
    table.insert(tabs[findtab(channel)or 0].nicks,nick)
   end
   resortnicks(findtab(channel)or 0)
  elseif command=="366" then
  elseif command:lower()=="ping" then
   c:send("PONG\n")
  elseif command:lower()=="part" then
   local channel=params:match"(%S+)"
   local nick=sender:match"([^!]+)"
   for i,v in ipairs(tabs[findtab(channel)or 0].nicks) do
    if v:lower():gsub("[@+]","")==nick:lower() then
     table.remove(tabs[findtab(channel)or 1].nicks,i)
     break
    end
   end
   wprint(findtab(channel)or 1,r(chancolor).."*** "..nick.." has left "..channel)
   if nick:lower()==mynick:lower() then
    table.remove(tabs,findtab(channel)or -1)
    wprint(1,r(chancolor).."*** You have left "..channel)
   end
  elseif command:lower()=="quit" then
   local nick=sender:match"([^!]+)"
   local reason=params:match"%s*:(.*)"
   for i,v in ipairs(tabs) do
    for j,n in ipairs(v.nicks) do
     if n:lower():gsub("[@+]","")==nick:lower() then
      wprint(i,r(chancolor).."*** "..nick.." has quit ("..reason..")")
      table.remove(v.nicks,j)
      break
     end
    end
   end
  elseif command:lower()=="mode" then
   local nick=sender:match"([^!]+)"
   local whom,modes=params:match"(%S+) (.*)"
   wprint(findtab(whom)or 1,r(chancolor).."*** "..nick.." sets modes for "..whom.." : "..modes)
   if whom:match"#" then
    tabs[findtab(whom)or 1].nicks={}
    c:send("NAMES "..whom.."\n")
   end
  elseif command:lower()=="nick" then
   local nick=sender:match"([^!]+)"
   local newnick=params:match":(%S+)"
   for i,v in ipairs(tabs) do
    for j,n in ipairs(v.nicks) do
     if n:lower():gsub("[@+]","")==nick:lower() then
      wprint(i,r(chancolor).."*** "..nick.." has changed nick to "..newnick)
      v.nicks[j]=n:gsub("[^@+]","")..newnick
      break
     end
    end
   end
   if nick:lower()==mynick:lower() then
    mynick=newnick
   end
  elseif command:lower()=="notice" then
   local nick=sender:match"([^!]+)"
   local text=params:match"%S+ :(.*)"
   wprint(ctab,"-- "..nick.." -- "..text)
  else
   wprint(1,s)
  end
 else
  if e~="timeout" then
   wprint(ctab,"*** ERROR READING FROM SOCKET: "..e)
   c=nil
  end
 end
end

local function caps(a)
 local from="`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./"
 local to=  "~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?"
 local s,e=from:find(a,1,true)
 return to:sub(s,e) or a
end

local function key(a,b,c,d)
 if d==1 then
  if b>31 and b<127 then
   edit=edit:sub(1,cpos)..(c%4==0 and string.char(b) or caps(string.char(b)))..edit:sub(cpos+1,#edit)
   cpos=cpos+1
  elseif b==13 then
   send()
  elseif b==8 then
   edit=edit:sub(1,math.max(cpos-1,0))..edit:sub(cpos+1,#edit)
   cpos=cpos-1
   if cpos<0 then
    cpos=0
   end
  elseif b==127 then
   edit=edit:sub(1,cpos)..edit:sub(cpos+2,#edit)
  elseif b==9 then
   local txt,lw=edit:match"(.*%s)(%S+)"
   lw=lw or edit
   txt=txt or ""
   for _,v in ipairs(tabs[ctab].nicks) do
    if v:gsub("[@+]",""):sub(1,#lw):lower()==lw:lower() then
     edit=txt..v:gsub("[@+]","")
    end
   end
  elseif b==276 then
   cpos=cpos-1
   if cpos<0 then
    cpos=0
   end
  elseif b==275 then
   cpos=cpos+1
   if cpos>#edit then
    cpos=#edit
   end
  elseif b==274 then
   histp=histp-1
   if cpos<0 then
    cpos=0
   end
   edit=history[histp]
   cpos=#edit
  elseif b==273 then
   histp=histp+1
   if histp>#history then
    histp=#history
   end
   edit=history[histp]
   cpos=#edit
  else
   return true
  end
 end
 return false
end

local launched=false
local visible=false

local function appear()
 if not visible then
  tpt.register_step(ui)
  tpt.register_mouseclick(mouse)
  tpt.register_keypress(key)
 end
 visible=true
end

local function disappear()
 if visible then
  tpt.unregister_step(ui)
  tpt.unregister_mouseclick(mouse)
  tpt.unregister_keypress(key)
 end
 visible=false
end

local function start()
 if not launched then
  resetwindows()
  c=socket.tcp()
  c:connect(conf.server,conf.port)
  c:settimeout(0)
  tpt.register_step(receive)
 end
 launched=true
end

local function stop()
 if launched then
  c:send"QUIT\n"
  c:close()
  c=nil
  tpt.unregister_step(receive)
 end
 disappear()
 launched=false
end


local function keyhandler(a,b,c,d)
 if d==1 and (math.floor(c/64)%2==1 or math.floor(c/128)%2==1) then
  if a=="m" then
   if c%4==0 then
    if visible then
     disappear()
    else
     if not launched then
      start()
     end
     appear()
    end
   else
    disappear()
    stop()
   end
   return false
  end
 end
end

tpt.register_keypress(keyhandler)
