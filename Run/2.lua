-- Mniip's GRaphics system --
-- Interface:
--
--
--
-- mgr.drawln(line,x,y,type of particles)
-- draws line of letters
-- returns line length
--
-- mgr.putpixel(x,y,type of particle)
-- draws 1 pixel at (x;y)
--
-- mgr.getpixel(x,y)
-- returns type of pixel at (x;y) - numeric
--
-- mgr.line(x1,y1,x2,y2,type of particles)
-- draws line from (x1;y1) to (x2;y2)
--
-- mgr.circle(x,y,radius,type of particles)
-- draws circle at (x;y) with preset radius
--
-- mgr.rectangle(x1,y1,x2,y2,type of particles)
-- draws rectangle with 2 corners at (x1;y1) and (x2;y2)
--
-- mgr.floodfill(x,y,type of particles)
-- SHOULD floodfill the area BUT DONT RUN
-- DOESNT WORKS, it may CRASH TPT


mgr={}
print("This can only be run in TPT Lua Console")

function mgr.putpixel(x,y,t)
if x>-1 and y>-1 and x<612 and y<384 then
tpt.delete(x,y)
tpt.create(x,y,t)
end
end

function mgr.getpixel(x,y,t)
if x>-1 and y>-1 and x<612 and y<384 then
return tpt.get_property("type",x,y)
end
end

function mgr.drawletter(l,x,y,t)
local matrix={0,0,0,0,0,0,0}
local w=4
if l=="A" then
 matrix={6,9,9,15,9,9,9}
elseif l=="a" then
 matrix={0,0,14,1,7,9,7}
elseif l=="B" then
 matrix={14,9,9,14,9,9,14}
elseif l=="b" then
 matrix={8,8,8,14,9,9,14}
elseif l=="C" then
 matrix={6,9,8,8,8,9,6}
elseif l=="c" then
 matrix={0,0,3,4,4,4,3}
 w=3
elseif l=="D" then
 matrix={14,9,9,9,9,9,14}
elseif l=="d" then
 matrix={1,1,1,7,9,9,7}
elseif l=="E" then
 matrix={15,8,8,14,8,8,15}
elseif l=="e" then
 matrix={0,0,6,9,15,8,7}
elseif l=="F" then
 matrix={15,8,8,14,8,8,8}
elseif l=="f" then
 matrix={3,4,4,6,4,4,4}
 w=3
elseif l=="G" then
 matrix={7,8,8,11,9,9,6}
elseif l=="g" then
 matrix={0,0,6,9,6,1,14}
elseif l=="H" then
 matrix={9,9,9,15,9,9,9}
elseif l=="h" then
 matrix={8,8,8,14,9,9,9}
elseif l=="I" then
 matrix={31,4,4,4,4,4,31}
 w=5
elseif l=="i" then
 matrix={1,0,1,1,1,1,1}
 w=1
elseif l=="J" then
 matrix={1,1,1,1,9,9,6}
elseif l=="j" then
 matrix={1,0,1,1,1,1,2}
 w=2
elseif l=="K" then
 matrix={9,9,9,14,9,9,9}
elseif l=="k" then
 matrix={8,8,9,10,12,10,9}
elseif l=="L" then
 matrix={8,8,8,8,8,8,15}
elseif l=="l" then
 matrix={1,1,1,1,1,1,1}
 w=1
elseif l=="M" then
 matrix={17,27,21,17,17,17,17}
 w=5
elseif l=="m" then
 matrix={0,0,26,21,21,21,21}
 w=5
elseif l=="N" then
 matrix={17,25,21,19,17,17,17}
 w=5
elseif l=="n" then
 matrix={0,0,14,9,9,9,9}
elseif l=="O" then
 matrix={6,9,9,9,9,9,6}
elseif l=="o" then
 matrix={0,0,6,9,9,9,6}
elseif l=="P" then
 matrix={14,9,9,14,8,8,8}
elseif l=="p" then
 matrix={0,0,6,5,6,4,4}
 w=3
elseif l=="Q" then
 matrix={14,17,17,17,21,18,13}
 w=5
elseif l=="q" then
 matrix={0,0,14,17,21,18,13}
 w=5
elseif l=="R" then
 matrix={14,9,9,14,9,9,9}
elseif l=="r" then
 matrix={0,0,11,12,8,8,8}
elseif l=="S" then
 matrix={7,8,8,6,1,1,14}
elseif l=="s" then
 matrix={0,0,3,4,2,1,6}
 w=3
elseif l=="T" then
 matrix={31,4,4,4,4,4,4}
 w=5
elseif l=="t" then
 matrix={4,4,4,6,4,4,3}
 w=3
elseif l=="U" then
 matrix={9,9,9,9,9,9,6}
elseif l=="u" then
 matrix={0,0,9,9,9,9,6}
elseif l=="V" then
 matrix={17,17,17,17,17,10,4}
 w=5
elseif l=="v" then
 matrix={0,0,17,17,17,10,4}
 w=5
elseif l=="W" then
 matrix={65,65,65,65,73,85,34}
 w=7
elseif l=="w" then
 matrix={0,0,17,17,17,21,10}
 w=5
elseif l=="X" then
 matrix={17,17,10,4,10,17,17}
 w=5
elseif l=="x" then
 matrix={0,0,5,5,2,5,5}
 w=3
elseif l=="Y" then
 matrix={17,17,10,4,4,4,4}
 w=5
elseif l=="y" then
 matrix={0,0,5,5,2,2,2}
 w=3
elseif l=="Z" then
 matrix={31,17,2,4,8,17,31}
 w=5
elseif l=="z" then
 matrix={0,0,7,1,2,4,7}
 w=3
elseif l=="1" then
 matrix={1,3,5,1,1,1,1}
 w=3
elseif l=="2" then
 matrix={6,9,1,2,4,8,15}
elseif l=="3" then
 matrix={6,9,1,2,1,9,6}
elseif l=="4" then
 matrix={9,9,9,15,1,1,1}
elseif l=="5" then
 matrix={15,8,8,14,1,1,14}
elseif l=="6" then
 matrix={6,9,8,14,9,9,6}
elseif l=="7" then
 matrix={15,1,2,4,4,4,4}
elseif l=="8" then
 matrix={6,9,9,6,9,9,6}
elseif l=="9" then
 matrix={6,9,9,7,1,1,14}
elseif l=="0" then
 matrix={6,9,9,9,9,9,6}
elseif l=="!" then
 matrix={1,1,1,1,1,0,1}
 w=1
elseif l=="@" then
 matrix={62,65,77,85,74,64,62}
 w=7
elseif l=="#" then
 matrix={10,10,31,10,31,10,10}
 w=5
elseif l=="$" then
 matrix={4,15,16,14,1,30,4}
 w=5
elseif l=="%" then
 matrix={25,26,2,4,8,11,19}
 w=5
elseif l=="^" then
 matrix={4,10,17,0,0,0,0}
 w=5
elseif l=="&" then
 matrix={12,18,20,8,21,18,13}
 w=5
elseif l=="*" then
 matrix={5,2,5,0,0,0,0}
 w=3
elseif l=="(" then
 matrix={1,2,2,2,2,2,1}
 w=2
elseif l==")" then
 matrix={2,1,1,1,1,1,2}
 w=2
elseif l=="[" then
 matrix={7,4,4,4,4,4,7}
 w=3
elseif l=="]" then
 matrix={7,1,1,1,1,1,7}
 w=3
elseif l=="{" then
 matrix={3,4,4,8,4,4,3}
elseif l=="}" then
 matrix={12,2,2,1,2,2,12}
elseif l=="<" then
 matrix={1,2,4,8,4,2,1}
elseif l==">" then
 matrix={8,4,2,1,2,4,8}
elseif l=="'" then
 matrix={2,1,0,0,0,0,0}
 w=2
elseif l=="." then
 matrix={0,0,0,0,0,0,1}
 w=1
elseif l=="," then
 matrix={0,0,0,0,0,1,2}
 w=2
elseif l==":" then
 matrix={0,0,1,0,0,1,0}
 w=1
elseif l==";" then
 matrix={0,0,1,0,0,1,2}
 w=2
elseif l=="\"" then
 matrix={10,5,0,0,0,0,0}
elseif l=="_" then
 matrix={0,0,0,0,0,0,7}
 w=3
elseif l=="+" then
 matrix={0,0,2,7,2,0,0}
 w=3
elseif l=="-" then
 matrix={0,0,0,7,0,0,0}
 w=3
elseif l=="=" then
 matrix={0,0,7,0,7,0,0}
 w=3
elseif l=="/" then
 matrix={1,2,2,4,8,8,16}
 w=5
elseif l=="\\" then
 matrix={16,8,8,4,2,2,1}
 w=5
elseif l=="|" then
 matrix={1,1,1,0,1,1,1}
 w=1
elseif l=="?" then
 matrix={6,1,1,1,2,0,2}
 w=3
end
for i=1,7 do
 for j=0,w do
  if math.floor(matrix[i]/math.pow(2,w-j))%2==1 then
   mgr.putpixel(x+j,y+i,t)
  end
 end
end
return w
end

function mgr.circle(x,y,r,t)
for i=x,math.ceil(math.sqrt(1/2)*r)+x do
 mgr.putpixel(i,y-math.ceil(r*math.sqrt(1-(i-x)*(i-

x)/r/r)),t)
 mgr.putpixel(i,y+math.ceil(r*math.sqrt(1-(i-x)*(i-

x)/r/r)),t)
 mgr.putpixel(x+x-i,y-math.ceil(r*math.sqrt(1-(i-x)*(i-

x)/r/r)),t)
 mgr.putpixel(x+x-i,y+math.ceil(r*math.sqrt(1-(i-x)*(i-

x)/r/r)),t)
end
for i=y,math.ceil(math.sqrt(1/2)*r)+y do
 mgr.putpixel(x-math.ceil(r*math.sqrt(1-(i-y)*(i-

y)/r/r)),i,t)
 mgr.putpixel(x+math.ceil(r*math.sqrt(1-(i-y)*(i-

y)/r/r)),i,t)
 mgr.putpixel(x-math.ceil(r*math.sqrt(1-(i-y)*(i-

y)/r/r)),y+y-i,t)
 mgr.putpixel(x+math.ceil(r*math.sqrt(1-(i-y)*(i-

y)/r/r)),y+y-i,t)
end
end

function mgr.line(x1,y1,x2,y2,t)
if math.abs(x2-x1)>math.abs(y2-y1) then
 for i=x1,x2 do
  mgr.putpixel(i,math.floor(y1+(i-x1)*(y2-y1)/(x2-x1)),t)
 end
else
 for i=y1,y2 do
  mgr.putpixel(math.floor(x1+(i-y1)*(x2-x1)/(y2-y1)),i,t)
 end
end
end

function mgr.rectangle(x1,y1,x2,y2,t)
mgr.line(x1,y1,x2,y1,t)
mgr.line(x1,y1,x1,y2,t)
mgr.line(x2,y1,x2,y2,t)
mgr.line(x1,y2,x2,y2,t)
end

function mgr.floodfill(x,y,t)
local function doflood(x,y,t,b)
 mgr.putpixel(x,y,t)
 if mgr.getpixel(x+1,y)==b then doflood(x+1,y,t,b) end
 if mgr.getpixel(x-1,y)==b then doflood(x-1,y,t,b) end
 if mgr.getpixel(x,y+1)==b then doflood(x,y+1,t,b) end
 if mgr.getpixel(x,y-1)==b then doflood(x,y-1,t,b) end
end
doflood(x,y,t,mgr.getpixel(x,y))
end

function mgr.drawln(l,x,y,t)
local ox=0
for i=1,string.len(l) do
 ox=1+ox+mgr.drawletter(string.sub(l,i,i),x+ox,y,t)
end
return string.len(l)
end