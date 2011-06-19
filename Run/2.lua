-- Mniip and UBERNESS's Lua text drawing system --
mniipstext={}
function mniipstext.drawletter(l,x,y,t)
local matrix1={0,0,0}
local matrix2={0,0,0}
local matrix3={0,0,0}
local matrix4={0,0,0}
local matrix5={0,0,0}
local w=3
if l=="a" then
 matrix1={0,1,0}
 matrix2={1,0,1} 
 matrix3={1,1,1}
 matrix4={1,0,1}
 matrix5={1,0,1}
elseif l=="b" then
 matrix1={1,1,0}
 matrix2={1,0,1}
 matrix3={1,1,0}
 matrix4={1,0,1}
 matrix5={1,1,0}
elseif l=="c" then
 matrix1={0,1}
 matrix2={1,0}
 matrix3={1,0}
 matrix4={1,0}
 matrix5={0,1}
 w=2
elseif l=="d" then
 matrix1={1,1,0}
 matrix2={1,0,1}
 matrix3={1,0,1}
 matrix4={1,0,1}
 matrix5={1,1,0}
elseif l=="e" then
 matrix1={1,1}
 matrix2={1,0}
 matrix3={1,1}
 matrix4={1,0}
 matrix5={1,1}
 w=2
elseif l=="f" then
 matrix1={1,1}
 matrix2={1,0}
 matrix3={1,1}
 matrix4={1,0}
 matrix5={1,0}
 w=2
elseif l=="g" then
 matrix1={0,1,1,1}
 matrix2={1,0,0,0}
 matrix3={1,0,1,1}
 matrix4={1,0,0,1}
 matrix5={0,1,1,0}
 w=4
elseif l=="h" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={1,1,1}
 matrix4={1,0,1}
 matrix5={1,0,1}
elseif l=="i" then
 matrix1={1,1,1}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={1,1,1}
 w=3
elseif l=="j" then
 matrix1={0,1}
 matrix2={0,1}
 matrix3={0,1}
 matrix4={0,1}
 matrix5={1,0}
 w=2
elseif l=="k" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={1,1,0}
 matrix4={1,0,1}
 matrix5={1,0,1}
elseif l=="l" then
 matrix1={1,0,0}
 matrix2={1,0,0}
 matrix3={1,0,0}
 matrix4={1,0,0}
 matrix5={1,1,1}
 w=2
elseif l=="m" then
 matrix1={1,0,0,0,1}
 matrix2={1,1,0,1,1}
 matrix3={1,0,1,0,1}
 matrix4={1,0,0,0,1}
 matrix5={1,0,0,0,1}
 w=5
elseif l=="n" then
 matrix1={1,0,0,1}
 matrix2={1,1,0,1}
 matrix3={1,0,1,1}
 matrix4={1,0,0,1}
 matrix5={1,0,0,1}
 w=4
elseif l=="o" or l=="0" then
 matrix1={0,1,0}
 matrix2={1,0,1}
 matrix3={1,0,1}
 matrix4={1,0,1}
 matrix5={0,1,0}
elseif l=="p" then
 matrix1={1,1,0}
 matrix2={1,0,1}
 matrix3={1,1,0}
 matrix4={1,0,0}
 matrix5={1,0,0}
elseif l=="q" then
 matrix1={0,1,1,1,0}
 matrix2={1,0,0,0,1}
 matrix3={1,0,1,0,1}
 matrix4={1,0,0,1,0}
 matrix5={0,1,1,0,1}
 w=5
elseif l=="r" then
 matrix1={1,1,0}
 matrix2={1,0,1}
 matrix3={1,1,0}
 matrix4={1,0,1}
 matrix5={1,0,1}
elseif l=="s" then
 matrix1={0,1,1}
 matrix2={1,0,0}
 matrix3={0,1,0}
 matrix4={0,0,1}
 matrix5={1,1,0}
elseif l=="t" then
 matrix1={1,1,1}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={0,1,0}
elseif l=="u" then
 matrix1={1,0,0,1}
 matrix2={1,0,0,1}
 matrix3={1,0,0,1}
 matrix4={1,0,0,1}
 matrix5={0,1,1,0}
 w=4
elseif l=="v" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={1,0,1}
 matrix4={1,0,1}
 matrix5={0,1,0}
elseif l=="w" then
 matrix1={1,0,1,0,1}
 matrix2={1,0,1,0,1}
 matrix3={1,0,1,0,1}
 matrix4={1,0,1,0,1}
 matrix5={0,1,0,1,0}
 w=5
elseif l=="x" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={0,1,0}
 matrix4={1,0,1}
 matrix5={1,0,1}
elseif l=="y" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={0,1,0}
elseif l=="z" then
 matrix1={1,1,1}
 matrix2={0,0,1}
 matrix3={0,1,0}
 matrix4={1,0,0}
 matrix5={1,1,1}
elseif l=="1" then
 matrix1={0,1,0}
 matrix2={1,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={1,1,1}
 w=2
elseif l=="2" then
 matrix1={1,1,0}
 matrix2={0,0,1}
 matrix3={0,1,0}
 matrix4={1,0,0}
 matrix5={1,1,1}
elseif l=="3" then
 matrix1={1,1,0}
 matrix2={0,0,1}
 matrix3={0,1,0}
 matrix4={0,0,1}
 matrix5={1,1,0}
elseif l=="4" then
 matrix1={1,0,1}
 matrix2={1,0,1}
 matrix3={1,1,1}
 matrix4={0,0,1}
 matrix5={0,0,1}
elseif l=="5" then
 matrix1={1,1,1}
 matrix2={1,0,0}
 matrix3={1,1,0}
 matrix4={0,0,1}
 matrix5={1,1,0}
elseif l=="6" then
 matrix1={0,1,1}
 matrix2={1,0,0}
 matrix3={1,1,0}
 matrix4={1,0,1}
 matrix5={0,1,0}
elseif l=="7" then
 matrix1={1,1,1}
 matrix2={0,0,1}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={0,1,0}
elseif l=="8" then
 matrix1={0,1,0}
 matrix2={1,0,1}
 matrix3={0,1,0}
 matrix4={1,0,1}
 matrix5={0,1,0}
elseif l=="9" then
 matrix1={0,1,0}
 matrix2={1,0,1}
 matrix3={0,1,1}
 matrix4={0,0,1}
 matrix5={1,1,0}
 elseif l=="!" then
 matrix1={0,1,0}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,0,0}
 matrix5={0,1,0}
 elseif l=="(" then
 matrix1={0,0,1}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={0,0,1}
 elseif l==")" then
 matrix1={1,0,0}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={1,0,0}
 elseif l=="-" then
 matrix1={0,0,0}
 matrix2={0,0,0}
 matrix3={1,1,1}
 matrix4={0,0,0}
 matrix5={0,0,0}
 elseif l=="_" then
 matrix1={0,0,0}
 matrix2={0,0,0}
 matrix3={0,0,0}
 matrix4={0,0,0}
 matrix5={1,1,1}
 elseif l=="=" then
 matrix1={0,0,0}
 matrix2={1,1,1}
 matrix3={0,0,0}
 matrix4={1,1,1}
 matrix5={0,0,0}
 elseif l=="+" then
 matrix1={0,0,0}
 matrix2={0,1,0}
 matrix3={1,1,1}
 matrix4={0,1,0}
 matrix5={0,0,0}
 elseif l=="[" then
 matrix1={1,1,0}
 matrix2={1,0,0}
 matrix3={1,0,0}
 matrix4={1,0,0}
 matrix5={1,1,0}
 elseif l=="]" then
 matrix1={0,1,1}
 matrix2={0,0,1}
 matrix3={0,0,1}
 matrix4={0,0,1}
 matrix5={0,1,1}
 elseif l=="{" then
 matrix1={0,1,1}
 matrix2={0,1,0}
 matrix3={1,0,0}
 matrix4={0,1,0}
 matrix5={0,1,1}
 elseif l=="}" then
 matrix1={1,1,0}
 matrix2={0,1,0}
 matrix3={0,0,1}
 matrix4={0,1,0}
 matrix5={1,1,0}
 elseif l=="|" then
 matrix1={0,1,0}
 matrix2={0,1,0}
 matrix3={0,1,0}
 matrix4={0,1,0}
 matrix5={0,1,0}
 elseif l==":" then
 matrix1={0,0,0}
 matrix2={0,1,0}
 matrix3={0,0,0}
 matrix4={0,1,0}
 matrix5={0,0,0}
 elseif l==";" then
 matrix1={0,0,0}
 matrix2={0,1,0}
 matrix3={0,0,0}
 matrix4={0,1,0}
 matrix5={1,0,0}
 elseif l=="'" then
 matrix1={0,1,0}
 matrix2={0,1,0}
 matrix3={0,0,0}
 matrix4={0,0,0}
 matrix5={0,0,0}
 elseif l=="," then
 matrix1={0,0,0}
 matrix2={0,0,0}
 matrix3={0,0,0}
 matrix4={0,1,0}
 matrix5={1,0,0}
 elseif l=="." then
 matrix1={0}
 matrix2={0}
 matrix3={0}
 matrix4={0}
 matrix5={1}
 elseif l=="<" then
 matrix1={0,0,1}
 matrix2={0,1,0}
 matrix3={1,0,0}
 matrix4={0,1,0}
 matrix5={0,0,1}
 elseif l==">" then
 matrix1={1,0,0}
 matrix2={0,1,0}
 matrix3={0,0,1}
 matrix4={0,1,0}
 matrix5={1,0,0}
 elseif l=="/" then
 matrix1={0,0,0,0,1}
 matrix2={0,0,0,1,0}
 matrix3={0,0,1,0,0}
 matrix4={0,1,0,0,0}
 matrix5={1,0,0,0,0}
 elseif l=="?" then
 matrix1={1,1,1}
 matrix2={0,0,1}
 matrix3={0,1,0}
 matrix4={0,0,0}
 matrix5={0,1,0}
 elseif l=="^" then
 matrix1={0,1,0}
 matrix2={1,0,1}
 matrix3={0,0,0}
 matrix4={0,0,0}
 matrix5={0,0,0}
 elseif l=="#" then
 matrix1={0,1,0,1,0}
 matrix2={1,1,1,1,1}
 matrix3={0,1,0,1,0}
 matrix4={1,1,1,1,1}
 matrix5={0,1,0,1,0}
 elseif l=="%" then
 matrix1={1,1,0,0,1}
 matrix2={1,1,0,1,0}
 matrix3={0,0,1,0,0}
 matrix4={0,1,0,1,1}
 matrix5={1,0,0,1,1}
 elseif l=="%" then
 matrix1={1,1,0,0,1}
 matrix2={1,1,0,1,0}
 matrix3={0,0,1,0,0}
 matrix4={0,1,0,1,1}
 matrix5={1,0,0,1,1} 
 elseif l=="&" then
 matrix1={0,0,1,1,1}
 matrix2={0,0,1,0,0}
 matrix3={0,1,1,1,1}
 matrix4={1,0,0,1,0}
 matrix5={0,1,1,0,1} 
 elseif l=="@" then
 matrix1={0,1,0,0,1,1,1}
 matrix2={1,0,1,0,0,1,0}
 matrix3={1,1,1,0,0,1,0}
 matrix4={1,0,1,0,0,1,0}
 matrix5={1,0,1,0,0,1,0} 
 end            
for i=1,7 do
 if matrix1[i]==1 then
  tpt.create(x+i-1,y,t)
 end
end
for i=1,7 do
 if matrix2[i]==1 then
  tpt.create(x+i-1,y+1,t)
 end
end
for i=1,7 do
 if matrix3[i]==1 then
  tpt.create(x+i-1,y+2,t)
 end
end
for i=1,7 do
 if matrix4[i]==1 then
  tpt.create(x+i-1,y+3,t)
 end
end
for i=1,7 do
 if matrix5[i]==1 then
  tpt.create(x+i-1,y+4,t)
 end
end
return w
end

function mniipstext.drawln(l,x,y,t)
local ox=0
for i=1,string.len(l) do
 ox=1+ox+mniipstext.drawletter(string.sub(string.lower(l),i,i),x+ox,y,t)
end
return string.len(l)
end