--Cracker1000's TPT unleashed script 
local version = 12
local toggle = Button:new(320,0,13,8, "V", "Toggle additional menus.")

local deletesparkButton =  Button:new(320,6,75,10,"Interface", "shows UI related stuff.")
local UIhidey = Button:new(410,6,40,20,"Hide", "Hides the UI")
local UIhiden = Button:new(410,26,40,20,"Show", "Shows the UI")

local FPS = Button:new(320,26,75,10, "Frame limiter", "Turns the frame limiter on/off.")
local FPS1 = Button:new(410,26,80,20, "On", "Turns the frame limiter on.")
local FPS2 = Button:new(410,46,80,20, "Off", "Turns the frame limiter off.")

local screen= Button:new(320,46,75,10,"Screenshot", "Take a screen shot.")
local screen1= Button:new(300,0,15,9,"Y", "Take the screen shot.")
local screen2= Button:new(325,0,15,9,"N", "Cancel the screen shot.")

local reset= Button:new(320,66,75,10,"Reset", "Reset everything.")

local info= Button:new(320,86,75,10,"Stack tools", "Usefull for subframe.")
local info1= Button:new(410,86,40,20,"Separate", "Removes top most particle from stack.")
local info2= Button:new(410,106,40,20,"Delete", "Leaves top particle and PHOT but remove everything else.")

local Ruler = Button:new(320,106,75,10, "Ruler", "Toggles in game ruler.")
local Ry = Button:new(410,106,40,20,"Hide", "Hides the Ruler")
local Rn = Button:new(410,126,40,20,"Show", "Shows the Ruler. (Press SHIFT)")

local mp = Button:new(320,126,75,10,"Theme", "Changes game's theme")
local mp1 = Button:new(410,126,75,10,"Dark", "Change the theme to default")
local mp2 = Button:new(410,136,75,10,"Fire", "Change the theme to Blue")
local mp3 = Button:new(410,146,75,10,"Aqua", "Change the theme to Red")
local mp4 = Button:new(410,156,75,10,"Forest", "Change the theme to Green")
local mp5 = Button:new(410,166,75,10,"Sun", "Change the theme to Yellow")
local mp6 = Button:new(410,176,75,10,"Orange", "Change the theme to Orange")
local mp7 = Button:new(410,186,75,10,"Vanilla", "Change the theme back to Plain white")
local mp8 = Button:new(410,196,75,10,"Pulse", "RBG makes everything better.")

local rc = Button:new(320,146,75,10,"Record", "Options for recording frames")
local rc1 = Button:new(410,146,40,20,"Start", "Starts recording")
local rc2 = Button:new(410,166,40,20,"Stop","Stops recording")

local bg = Button:new(320,166,75,10,"Backgrounds", "Sets different backgrounds.")
local bgI = Button:new(470,166,30,10,"^", "Increase brightness")
local bgD = Button:new(470,176,30,10,"v", "Decrease brightness")
local bg1 = Button:new(410,166,60,10,"BLACK", "Default")
local bg2 = Button:new(410,176,60,10,"BLUE", "Blue background")
local bg3 = Button:new(410,186,60,10,"RED", "Red background")
local bg4 = Button:new(410,196,60,10,"GREEN", "Green background")
local bg5 = Button:new(410,206,60,10,"YELLOW", "Yellow background")

local bug = Button:new(320,186,75,10,"Bug report", "Direct to Mod thread for bug report.")

local bar = Button:new(320,206,75,10,"Top bar", "Toggle top bar")
local bary = Button:new(410,206,75,20,"Show", "Shows the bar at top")
local barn = Button:new(410,226,75,20,"Hide", "Hides the bar")

local wiki  =  Button:new(320,226,75,10,"Wiki", "Element wiki!")
local wikin2 = Button:new(10,360,75,20,"Hide wiki ", " Close wiki!")

local bare = Button:new(320,246,75,10,"Hidden Elem.", "Toggle hidden elements.")
local barye = Button:new(410,246,75,20,"Show", "Shows hidden elements")
local barne = Button:new(410,266,75,20,"Hide", "Hides elements")

local hide= Button:new(320,266,15,10, "^", "Hide.")

function clearm()
interface.removeComponent(reset)
interface.removeComponent(screen)
interface.removeComponent(FPS)
interface.removeComponent(deletesparkButton)
interface.removeComponent(hide)
interface.removeComponent(info)
interface.removeComponent(Ruler)
interface.removeComponent(mp)
interface.removeComponent(rc)
interface.removeComponent(bg)
interface.removeComponent(bug)
interface.removeComponent(bar)
interface.removeComponent(bare)
interface.removeComponent(wiki)
end

function clearsb()
interface.removeComponent(UIhiden)
interface.removeComponent(UIhidey)
interface.removeComponent(Ry)
interface.removeComponent(Rn)
interface.removeComponent(rc1)
interface.removeComponent(rc2)
interface.removeComponent(bg1)
interface.removeComponent(bg2)
interface.removeComponent(bg3)
interface.removeComponent(bg4)
interface.removeComponent(bg5)
interface.removeComponent(bgI)
interface.removeComponent(bgD)
interface.removeComponent(screen1)
interface.removeComponent(screen2)
interface.removeComponent(FPS1)
interface.removeComponent(FPS2)
interface.removeComponent(mp1)
interface.removeComponent(mp2)
interface.removeComponent(mp3)
interface.removeComponent(mp4)
interface.removeComponent(mp5)
interface.removeComponent(mp6)
interface.removeComponent(mp7)
interface.removeComponent(mp8)
interface.removeComponent(bary)
interface.removeComponent(barn)
interface.removeComponent(barye)
interface.removeComponent(barne)
interface.removeComponent(info1)
interface.removeComponent(info2)
interface.removeComponent(wikin2)
end

clearm()
bug:action(function(sender)
platform.openLink("https://powdertoy.co.uk/Discussions/Thread/View.html?Thread=23279")
end)

bare:action(function(sender)
clearsb()
interface.addComponent(barye)
interface.addComponent(barne)
end)

barye:action(function(sender)
tpt.el.dyst.menusection=8
tpt.el.eqve.menusection=8
tpt.el.shd4.menusection=9
tpt.el.shd3.menusection=9
tpt.el.shd2.menusection=9
tpt.el.lolz.menusection=11
tpt.el.love.menusection=11
tpt.el.embr.menusection=4
tpt.el.spwn.menusection=11
tpt.el.spwn2.menusection=11
tpt.el.frzw.menusection=7
tpt.el.bizs.menusection=9
tpt.el.bizg.menusection=6
tpt.el.bray.menusection=9
tpt.el.psts.menusection=8
tpt.el.mort.menusection=5
tpt.el.dyst.menu=1
tpt.el.eqve.menu=1
tpt.el.shd4.menu=1
tpt.el.shd3.menu=1
tpt.el.shd2.menu=1
tpt.el.lolz.menu=1
tpt.el.love.menu=1
tpt.el.embr.menu=1
tpt.el.spwn.menu=1
tpt.el.spwn2.menu=1
tpt.el.frzw.menu=1
tpt.el.bizs.menu=1
tpt.el.bizg.menu=1
tpt.el.bray.menu=1
tpt.el.psts.menu=1
tpt.el.mort.menu=1
clearsb()
end)

barne:action(function(sender)
tpt.el.dyst.menu=0
tpt.el.eqve.menu=0
tpt.el.shd4.menu=0
tpt.el.shd3.menu=0
tpt.el.shd2.menu=0
tpt.el.lolz.menu=0
tpt.el.love.menu=0
tpt.el.embr.menu=0
tpt.el.spwn.menu=0
tpt.el.spwn2.menu=0
tpt.el.frzw.menu=0
tpt.el.bizs.menu=0
tpt.el.bizg.menu=0
tpt.el.bray.menu=0
tpt.el.psts.menu=0
tpt.el.mort.menu=0
clearsb()
end)

wiki:action(function(sender)
clearsb()
clearm()
tpt.hud(0)
interface.addComponent(wikin2)
tpt.register_step(wikii)
tpt.register_step(backb)
tpt.register_step(UIhide)
end)

function wikii()
tpt.unregister_step(verno)
gfx.drawText(10,10, " WELCOME TO IN GAME WIKI: \n\n WAll: Hybrid of walls and elements.\n VLSN: Velocity sensor.  Creates SPRK when nearby velocity's higher than it's temp, Configured with .tmp modes.\n TIMC: Time based convertor, converts into it's ctype when sparked with PSCN. Timer set using .tmp, default is 100.\n FUEL: FUEL. Fuel having high calorific value.\n THRM: Thermostat. Sets the temp of surrounding according to its own temp.\n CLNT: Coolant. Cools down the temp of the system, Use .tmp to configure the cooling/heating power. \n DMRN: Demron. Radioactive shielding material and a better insulator.\n FNTC & FPTC: Faster versions of NTCT and PTCT.\n PINV: Powered Invisible, allows particles to move through when activated.\n UV: Ultra violet rays, heals stkm and figh, grows plnt, can sprk pscn and evaporates watr.\n SUN.: Sun, PLNT grow in direction of sunlight, emits radiation, makes PSCN spark and heals STKMs.\n LITH: Lithium ion battery, Use with PSCN and NSCN. Charges with INST when deactivated. Life sets capacity.\n Reacts with different elements like O2, WATR, ACID etc as IRL.\n LED:  Light Emmiting Diode. Use with PSCN and NSCN. Temp sets the brightness.\n Different .tmp2 modes: 0 = white, 1= red, 2= green, 3 =blue, 4= yellow and 5 = pink. \n QGP: Quark Gluon Plasma, bursts out radiation afer sometime. Turns into Purple QGP when under 100C which is stable.\n TMPS: .tmp sensor, creats sprk when there is an element with higher .tmp than its temp.\n PHOS: White, slowly turns into red phosphorus with time. When in contact with O2, burns blue or red based on .tmp.\n Oil reverses the oxidation turning it back into white PHOS. Melts at 45C.\n CMNT: Cement, heats up when mixed with water and gets solidified, darkens when solidified.\n NTRG: Nitrogen gas, liquifies to LN2 when cooled or when under pressure, reacts with H2 to make NITR and puts out fire.\n PRMT: Promethium, radioactive. Catches fire at high velocity (>12), creats NEUT when in reacted with PLUT. \n Explodes at low temp and emits neut at high temp.\n CLUD: Cloud, creates WATR and LIGH with slight pressure at random points. React with CAUS to make it rain acid.\n BEE: Eats PLNT and WOOD, convert WOOD into WAX.  Attacks STKMs and FIGH. Eats PLNT to stay alive\n Gets aggresive if life gets below 30. Uses pressure waves to communicate!\n ECLR: Electronic eraser, clears the defined radius when sparked with PSCN. NSCN clears everything. Use.tmp as radius.")
end

info:action(function(sender)
clearsb()
interface.addComponent(info1)
interface.addComponent(info2)
end)

info1:action(function(sender)
for i in sim.parts() do
		local x,y = sim.partProperty(i, sim.FIELD_X),sim.partProperty(i, sim.FIELD_Y)
		if sim.pmap (x, y) == i then 
                                tpt.delete(i)
		end
	end
end)

info2:action(function(sender)
for i in sim.parts() do
		local x,y = sim.partProperty(i, sim.FIELD_X),sim.partProperty(i, sim.FIELD_Y)
		if sim.pmap(x, y) ~= i and bit.band(elem.property(sim.partProperty(i, sim.FIELD_TYPE), "Properties"), elem.TYPE_ENERGY) == 0 then
			tpt.delete(i)
		end
	end
end)


wikin2:action(function(sender)
clearsb()
clearm()
tpt.unregister_step(wikii)
tpt.unregister_step(backb)
interface.addComponent(toggle)
tpt.unregister_step(UIhide)
tpt.hud(1)
end)

bg:action(function(sender)
clearsb()
interface.addComponent(bg1)
interface.addComponent(bg2)
interface.addComponent(bg3)
interface.addComponent(bg4)
interface.addComponent(bg5)
interface.addComponent(bgI)
interface.addComponent(bgD)
end)

local as = 60

function backb()
tpt.fillrect(0,0,610,385,0,0,255,as)
end
function backr()
tpt.fillrect(0,0,610,385,255,0,0,as)
end
function backg()
tpt.fillrect(0,0,610,385,0,255,0,as)
end
function backy()
tpt.fillrect(0,0,610,385,255,255,0,as)
end

function clearbg()
interface.removeComponent(bgI)
interface.removeComponent(bgD)
interface.removeComponent(bg5)
interface.removeComponent(bg4)
interface.removeComponent(bg3)
interface.removeComponent(bg2)
interface.removeComponent(bg1)
tpt.unregister_step(backy)
tpt.unregister_step(backb)
tpt.unregister_step(backr)
tpt.unregister_step(backg)
end

tgr = 0
tgg  = 0
tgb = 200

bgI:action(function(sender)
as = as+30
end)
bgD:action(function(sender)
as = as-30
end)

bg1:action(function(sender)
clearbg()
tgr = 0
tgg  = 0
tgb = 200
end)
bg2:action(function(sender)
tgr = 0
tgg  = 0
tgb = 200
clearbg()
tpt.register_step(backb)
end)

bg3:action(function(sender)
tgr = 200
tgg  = 0
tgb = 0
clearbg()
tpt.register_step(backr)
end)

bg4:action(function(sender)
tgr = 0
tgg  = 200
tgb = 0
clearbg()
tpt.register_step(backg)
end)

bg5:action(function(sender)
tgr = 200
tgg  = 200
tgb = 0
clearbg()
tpt.register_step(backy)
end)

rc:action(function(sender)
clearsb()
interface.addComponent(rc1)
interface.addComponent(rc2)
end)


rc1:action(function(sender)
tpt.record(true)
interface.removeComponent(rc1)
interface.removeComponent(rc2)

end)

rc2:action(function(sender)
tpt.record(false)
interface.removeComponent(rc1)
interface.removeComponent(rc2)
end)

function theme()
if MANAGER.getsetting("CRK", "savergb") == "0" then 
ar = MANAGER.getsetting("CRK", "ar")
ag = MANAGER.getsetting("CRK", "ag")
ab = MANAGER.getsetting("CRK", "ab")
al = MANAGER.getsetting("CRK", "al")
tpt.drawrect(613,1,14,95,ar,ag,ab,al)
tpt.drawrect(613,136,14,269,ar,ag,ab,al)
tpt.drawline(612,408,612,421,ar,ag,ab,al)
tpt.drawline(187,409,187,422,ar,ag,ab,al)
tpt.drawline(469,408,469,421,ar,ag,ab,al)
tpt.drawline(487,408,487,421,ar,ag,ab,al)
tpt.drawline(507,408,507,421,ar,ag,ab,al)

tpt.drawline(241,408,241,421,ar,ag,ab,al)
tpt.drawline(36,408,36,421,ar,ag,ab,al)
tpt.drawline(18,408,18,421,ar,ag,ab,al)
tpt.drawline(580,409,580,422,ar,ag,ab,al)
tpt.drawline(596,409,596,422,ar,ag,ab,al)
tpt.drawrect(1,408,626,14,ar,ag,ab,al)

tpt.drawline(613,96,627,96,ar,ag,ab,al)
tpt.drawline(613,16,627,16,ar,ag,ab,al)
tpt.drawline(613,32,627,32,ar,ag,ab,al)
tpt.drawline(613,48,627,48,ar,ag,ab,al)
tpt.drawline(613,64,627,64,ar,ag,ab,al)
tpt.drawline(613,80,627,80,ar,ag,ab,al)

tpt.drawline(613,151,627,151,ar,ag,ab,al)
tpt.drawline(613,167,627,167,ar,ag,ab,al)
tpt.drawline(613,183,627,183,ar,ag,ab,al)
tpt.drawline(613,199,627,199,ar,ag,ab,al)
tpt.drawline(613,215,627,215,ar,ag,ab,al)
tpt.drawline(613,231,627,231,ar,ag,ab,al)
tpt.drawline(613,247,627,247,ar,ag,ab,al)
tpt.drawline(613,263,627,263,ar,ag,ab,al)
tpt.drawline(613,279,627,279,ar,ag,ab,al)
tpt.drawline(613,295,627,295,ar,ag,ab,al)
tpt.drawline(613,311,627,311,ar,ag,ab,al)
tpt.drawline(613,327,627,327,ar,ag,ab,al)
tpt.drawline(613,343,627,343,ar,ag,ab,al)
tpt.drawline(613,359,627,359,ar,ag,ab,al)
tpt.drawline(613,375,627,375,ar,ag,ab,al)
tpt.drawline(613,391,627,391,ar,ag,ab,al)
end
end

frameCount,colourRED,colourGRN,colourBLU = 0,0,0,0
function colourblender()
if MANAGER.getsetting("CRK", "savergb") == nil then
savergb = 1
end 

 colourRGB = {colourRED,colourGRN,colourBLU}
 if frameCount > 1529 then frameCount = 0 else frameCount = frameCount + 1 end
 if frameCount > 0 and frameCount < 255 then
  colourRED = 255
  if colourGRN > 254 then else colourGRN = colourGRN + 1 end
 end
 if frameCount > 254 and frameCount < 510 then
  colourGRN = 255
  if colourRED == 0 then else colourRED = colourRED - 1 end
 end
 if frameCount > 510 and frameCount < 765 then
  colourGRN = 255
  if colourBLU > 254 then else colourBLU = colourBLU + 1 end
 end
 if frameCount > 764 and frameCount < 1020 then
  colourBLU = 255
  if colourGRN == 0 then else colourGRN = colourGRN - 1 end
 end
 if frameCount > 1020 and frameCount < 1275 then
  colourBLU = 255
  if colourRED > 254 then else colourRED = colourRED + 1 end
 end
 if frameCount > 1274 and frameCount < 1530 then
  colourRED = 255
  if colourBLU == 0 then else colourBLU = colourBLU - 1 end
 end
tpt.drawrect(613,1,14,95,colourRED,colourGRN,colourBLU,255)
tpt.drawrect(613,136,14,269,colourRED,colourGRN,colourBLU,255)
tpt.drawline(612,408,612,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(187,409,187,422,colourRED,colourGRN,colourBLU,255)
tpt.drawline(469,408,469,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(487,408,487,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(507,408,507,421,colourRED,colourGRN,colourBLU,255)

tpt.drawline(241,408,241,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(36,408,36,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(18,408,18,421,colourRED,colourGRN,colourBLU,255)
tpt.drawline(580,409,580,422,colourRED,colourGRN,colourBLU,255)
tpt.drawline(596,409,596,422,colourRED,colourGRN,colourBLU,255)
tpt.drawrect(1,408,626,14,colourRED,colourGRN,colourBLU,255)

tpt.drawline(613,96,627,96,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,16,627,16,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,32,627,32,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,48,627,48,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,64,627,64,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,80,627,80,colourRED,colourGRN,colourBLU,255)

tpt.drawline(613,151,627,151,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,167,627,167,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,183,627,183,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,199,627,199,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,215,627,215,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,231,627,231,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,247,627,247,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,263,627,263,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,279,627,279,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,295,627,295,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,311,627,311,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,327,627,327,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,343,627,343,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,359,627,359,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,375,627,375,colourRED,colourGRN,colourBLU,255)
tpt.drawline(613,391,627,391,colourRED,colourGRN,colourBLU,255)
tpt.drawline(1, 0, 310, 0, colourRED,colourGRN,colourBLU,255)
tpt.drawline(340, 0, 610, 0,colourRED,colourGRN,colourBLU,255)
end

tpt.register_step(colourblender)
tpt.register_step(theme)
mp:action(function(sender)
clearsb()
fs.makeDirectory("scripts")
interface.addComponent(mp1)
interface.addComponent(mp2)
interface.addComponent(mp3)
interface.addComponent(mp4)
interface.addComponent(mp5)
interface.addComponent(mp6)
interface.addComponent(mp7)
interface.addComponent(mp8)
end)

function mpremove()
interface.removeComponent(mp1)
interface.removeComponent(mp2)
interface.removeComponent(mp3)
interface.removeComponent(mp4)
interface.removeComponent(mp5)
interface.removeComponent(mp6)
interface.removeComponent(mp7)
interface.removeComponent(mp8)
end

mp1:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",70)
MANAGER.savesetting("CRK","ag",70)
MANAGER.savesetting("CRK","ab",70)
MANAGER.savesetting("CRK","al",255)
tpt.unregister_step(colourblender)
tpt.register_step(theme)
mpremove()
end)

mp2:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",255)
MANAGER.savesetting("CRK","ag",0)
MANAGER.savesetting("CRK","ab",0)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp3:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",0)
MANAGER.savesetting("CRK","ag",0)
MANAGER.savesetting("CRK","ab",255)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp4:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",0)
MANAGER.savesetting("CRK","ag",255)
MANAGER.savesetting("CRK","ab",0)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp5:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",255)
MANAGER.savesetting("CRK","ag",255)
MANAGER.savesetting("CRK","ab",0)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp6:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",255)
MANAGER.savesetting("CRK","ag",150)
MANAGER.savesetting("CRK","ab",0)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp7:action(function(sender)
MANAGER.savesetting("CRK","savergb",0)
MANAGER.savesetting("CRK","ar",255)
MANAGER.savesetting("CRK","ag",255)
MANAGER.savesetting("CRK","ab",255)
MANAGER.savesetting("CRK","al",255)
tpt.register_step(theme)
tpt.unregister_step(colourblender)
mpremove()
end)

mp8:action(function(sender)
MANAGER.savesetting("CRK","savergb",1)
tpt.register_step(colourblender)
tpt.unregister_step(theme)
mpremove()
end)

function topbar()
tpt.drawline(1, 0, 310, 0, ar, ag, ab,150)
tpt.drawline(340, 0, 610, 0,ar, ag, ab,150)
end

tpt.register_step(topbar)

bar:action(function(sender)
clearsb()
interface.addComponent(bary)
interface.addComponent(barn)
end)

bary:action(function(sender)
tpt.register_step(topbar)
clearsb()
end)

barn:action(function(sender)
tpt.unregister_step(topbar)
clearsb()
end)


Ruler:action(function(sender)
clearsb()
interface.addComponent(Ry)
interface.addComponent(Rn)
end)

Ry:action(function(sender)
tpt.setdebug(0X0)
interface.removeComponent(Ry)
interface.removeComponent(Rn)
end)

Rn:action(function(sender)

tpt.setdebug(0X4)
interface.removeComponent(Ry)
interface.removeComponent(Rn)
end)

deletesparkButton:action(function(sender)
clearsb()
tpt.hud(0)
interface.addComponent(UIhidey)
interface.addComponent(UIhiden)
end)


function UIhide()
tpt.hud(0)
tpt.fillrect(-1,382,616,42,0,0,0,255)
tpt.fillrect(612,0,17,424,0,0,0,255)
end
UIhidey:action(function(sender)
tpt.hud(0)
interface.removeComponent(UIhiden)
interface.removeComponent(UIhidey)
tpt.unregister_step(topbar)
tpt.register_step(UIhide)
end)

UIhiden:action(function(sender)
tpt.hud(0)
interface.removeComponent(UIhiden)
interface.removeComponent(UIhidey)
tpt.unregister_step(UIhide)
tpt.register_step(topbar)
end)

FPS:action(function(sender)
clearsb()
interface.addComponent(FPS1)
interface.addComponent(FPS2)
end)


FPS1:action(function(sender)
interface.removeComponent(FPS1)
interface.removeComponent(FPS2)
tpt.setfpscap(60)
end)

FPS2:action(function(sender)
interface.removeComponent(FPS1)
interface.removeComponent(FPS2)
tpt.setfpscap(160)
end)

screen1:action(function(sender)
interface.removeComponent(screen1)
interface.removeComponent(screen2)
tpt.screenshot(0)
interface.addComponent(toggle)
end)

screen2:action(function(sender)
interface.removeComponent(screen1)
interface.removeComponent(screen2)
interface.addComponent(toggle)
end)

screen:action(function(sender)
tpt.unregister_step(verno)
clearsb()
clearm()
interface.addComponent(screen1)
interface.addComponent(screen2)
end)

reset:action(function(sender)
tgr = 0
tgg  = 0
tgb = 200
tpt.el.dyst.menu=0
tpt.el.eqve.menu=0
tpt.el.shd4.menu=0
tpt.el.shd3.menu=0
tpt.el.shd2.menu=0
tpt.el.lolz.menu=0
tpt.el.love.menu=0
tpt.el.embr.menu=0
tpt.el.spwn.menu=0
tpt.el.spwn2.menu=0
tpt.el.frzw.menu=0
tpt.el.bizs.menu=0
tpt.el.bizg.menu=0
tpt.el.bray.menu=0
tpt.el.psts.menu=0
tpt.el.mort.menu=0
tpt.unregister_step(verno)
tpt.unregister_step(wikii)
tpt.unregister_step(UIhide)
tpt.display_mode(3)
tpt.watertest(0)
sim.edgeMode(0) 
tpt.setfpscap(60)
tpt.setwindowsize(1)
tpt.register_step(colourblender)
tpt.register_step(topbar)
tpt.newtonian_gravity(0)
tpt.decorations_enable(0)
sim.resetPressure()
tpt.ambient_heat(0)
sim.resetTemp()
tpt.reset_velocity(1,380,300,300)
clearbg()
clearsb()
tpt.setdebug(0X0)
sim.clearSim()
end)

hide:action(function(sender)
interface.addComponent(toggle)
tpt.unregister_step(verno)
clearsb()
clearm()
tpt.hud(1)
tpt.set_pause(0)
end)

function verno()
gfx.drawText(10,10, "Script Version:\nBuild ID: 20k7m6") 
gfx.drawText(85,10, version) 
end

toggle:action(function(sender)
tpt.set_pause(1)
tpt.hud(0)
interface.removeComponent(screen1)
tpt.register_step(verno)
interface.removeComponent(screen2)
interface.addComponent(deletesparkButton)
interface.addComponent(FPS)
interface.addComponent(screen)
interface.addComponent(info)
interface.addComponent(reset)
interface.addComponent(hide)
interface.addComponent(Ruler)
interface.addComponent(rc)
interface.addComponent(bg)
interface.addComponent(mp)
interface.addComponent(bug)
interface.addComponent(bar)
interface.addComponent(bare)
interface.addComponent(wiki)
interface.removeComponent(toggle)
end)
interface.addComponent(toggle)




local jacobsmod = tpt.version.jacob1s_mod
local icons = {
	["delete1"] = "\xEE\x80\x85",
	["delete2"] = "\xEE\x80\x86",
	["folder"] = "\xEE\x80\x93"
}
if jacobsmod then
	icons = {
		["delete1"] = "\133",
		["delete2"] = "\134",
		["folder"] = "\147"
	}
end


if not socket then error("TPT version not supported") end
if MANAGER then error("manager is already running") end

local scriptversion = 13
MANAGER = {["version"] = "3.11", ["scriptversion"] = scriptversion, ["hidden"] = true}

local type = type -- people like to overwrite this function with a global a lot
local TPT_LUA_PATH = 'scripts'
local PATH_SEP = '\\'
local OS = "WIN32"
local CHECKUPDATE = false
local EXE_NAME
if platform then
	OS = platform.platform()
	if OS ~= "WIN32" and OS ~= "WIN64" then
		PATH_SEP = '/'
	end
	EXE_NAME = platform.exeName()
	local temp = EXE_NAME:reverse():find(PATH_SEP)
	EXE_NAME = EXE_NAME:sub(#EXE_NAME-temp+2)
else
	if os.getenv('HOME') then
		PATH_SEP = '/'
		if fs.exists("/Applications") then
			OS = "MACOSX"
		else
			OS = "LIN64"
		end
	end
	if OS == "WIN32" or OS == "WIN64" then
		EXE_NAME = jacobsmod and "Jacob1\'s Mod.exe" or "Powder.exe"
	elseif OS == "MACOSX" then
		EXE_NAME = "powder-x" --can't restart on OS X (if using < 91.0)
	else
		EXE_NAME = jacobsmod and "Jacob1\'s Mod" or "powder"
	end
end
local filenames = {}
local num_files = 0 --downloaded scripts aren't stored in filenames
local localscripts = {}
local onlinescripts = {}
local running = {}
local requiresrestart=false
local online = false
local first_online = true
local updatetable --temporarily holds info on script manager updates
local gen_buttons
local sidebutton
local download_file
local settings = {}
math.randomseed(os.time()) math.random() math.random() math.random() --some filler randoms

--get line that can be saved into scriptinfo file
local function scriptInfoString(info)
	--Write table into data format
	if type(info)~="table" then return end
	local t = {}
	for k,v in pairs(info) do
		table.insert(t,k..":\""..v.."\"")
	end
	local rstr = table.concat(t,","):gsub("\r",""):gsub("\n","\\n")
	return rstr
end

--read a scriptinfo line
local function readScriptInfo(list)
	if not list then return {} end
	local scriptlist = {}
	for i in list:gmatch("[^\n]+") do
		local t = {}
		local ID = 0
		for k,v in i:gmatch("(%w+):\"([^\"]*)\"") do
			t[k]= tonumber(v) or v:gsub("\r",""):gsub("\\n","\n")
		end
		scriptlist[t.ID] = t
	end
	return scriptlist
end

--save settings
local function save_last()
	local savestring=""
	for script,v in pairs(running) do
		savestring = savestring.." \""..script.."\""
	end
	savestring = "SAV "..savestring.."\nDIR "..TPT_LUA_PATH
	for k,t in pairs(settings) do
	for n,v in pairs(t) do
		savestring = savestring.."\nSET "..k.." "..n..":\""..v.."\""
	end
	end
	local f
	if TPT_LUA_PATH == "scripts" then
		f = io.open(TPT_LUA_PATH..PATH_SEP.."autorunsettings.txt", "w")
	else
		f = io.open("autorunsettings.txt", "w")
	end
	if f then
		f:write(savestring)
		f:close()
	end

	f = io.open(TPT_LUA_PATH..PATH_SEP.."downloaded"..PATH_SEP.."scriptinfo", "w")
	if f then
		for k,v in pairs(localscripts) do
			f:write(scriptInfoString(v).."\n")
		end
		f:close()
	end
end

local function load_downloaded()
	local f = io.open(TPT_LUA_PATH..PATH_SEP.."downloaded"..PATH_SEP.."scriptinfo","r")
	if f then
		local lines = f:read("*a")
		f:close()
		localscripts = readScriptInfo(lines)
		for k,v in pairs(localscripts) do
			if k ~= 1 then
				if not v["ID"] or not v["name"] or not v["description"] or not v["path"] or not v["version"] then
					localscripts[k] = nil
				elseif not fs.exists(TPT_LUA_PATH.."/"..v["path"]:gsub("\\","/")) then
					 localscripts[k] = nil
				end
			end
		end
	end
end

--load settings before anything else
local function load_last()
	local f = io.open(TPT_LUA_PATH..PATH_SEP.."autorunsettings.txt","r")
	if not f then
		f = io.open("autorunsettings.txt","r")
	end
	if f then
		local lines = {}
		local line = f:read("*l")
		while line do
			table.insert(lines,(line:gsub("\r","")))
			line = f:read("*l")
		end
		f:close()
		for i=1, #lines do
			local tok=lines[i]:sub(1,3)
			local str=lines[i]:sub(5)
			if tok=="SAV" then
				for word in string.gmatch(str, "\"(.-)\"") do running[word] = true end
			elseif tok=="EXE" then
				EXE_NAME=str
			elseif tok=="DIR" then
				TPT_LUA_PATH=str
			elseif tok=="SET" then
				local ident,name,val = string.match(str,"(.-) (.-):\"(.-)\"")
				if settings[ident] then settings[ident][name]=val
				else settings[ident]={[name]=val} end
			end
		end
	end

	load_downloaded()
end
load_last()
--get list of files in scripts folder
local function load_filenames()
	filenames = {}
	local function searchRecursive(directory)
		local dirlist = fs.list(directory)
		if not dirlist then return end
		for i,v in ipairs(dirlist) do
			local file = directory.."/"..v
			if fs.isDirectory(file) and v ~= "downloaded" then
				searchRecursive(file)
			elseif fs.isFile(file) then
				if file:find("%.lua$") then
					local toinsert = file:sub(#TPT_LUA_PATH+2)
					if OS == "WIN32" or OS == "WIN64" then
						toinsert = toinsert:gsub("/", "\\") --not actually required
					end
					table.insert(filenames, toinsert)
				end
			end
		end
	end
	searchRecursive(TPT_LUA_PATH)
	table.sort(filenames, function(first,second) return first:lower() < second:lower() end)
end
--ui object stuff
local ui_base local ui_box local ui_line local ui_text local ui_button local ui_scrollbar local ui_tooltip local ui_checkbox local ui_console local ui_window
local tooltip
ui_base = {
new = function()
	local b={}
	b.drawlist = {}
	function b:drawadd(f)
		table.insert(self.drawlist,f)
	end
	function b:draw(...)
		for _,f in ipairs(self.drawlist) do
			if type(f)=="function" then
				f(self,...)
			end
		end
	end
	b.movelist = {}
	function b:moveadd(f)
		table.insert(self.movelist,f)
	end
	function b:onmove(x,y)
		for _,f in ipairs(self.movelist) do
			if type(f)=="function" then
				f(self,x,y)
			end
		end
	end
	return b
end
}
ui_box = {
new = function(x,y,w,h,r,g,b)
	local box=ui_base.new()
	box.x=x box.y=y box.w=w box.h=h box.x2=x+w box.y2=y+h
	box.r= 70
                box.g=70
                box.b= 70
	function box:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	function box:setbackground(r,g,b,a) self.br=r self.bg=g self.bb=b self.ba=a end
	box.drawbox=true
	box.drawbackground=false
	box:drawadd(function(self) if self.drawbackground then tpt.fillrect(self.x,self.y,self.w+1,self.h+1,self.br,self.bg,self.bb,self.ba) end
								if self.drawbox then tpt.drawrect(self.x,self.y,self.w,self.h,self.r,self.g,self.b) end end)
	box:moveadd(function(self,x,y)
		if x then self.x=self.x+x self.x2=self.x2+x end
		if y then self.y=self.y+y self.y2=self.y2+y end
	end)
	return box
end
}
ui_line = {
new=function(x,y,x2,y2,r,g,b)
	local line=ui_box.new(x,y,x2-x,y2-y,r,g,b)
	--Line is essentially a box, but with a different draw
	line.drawlist={}
	line:drawadd(function(self) tpt.drawline(self.x,self.y,self.x2,self.y2,self.r,self.g,self.b) end)
	return line
	end
}
ui_text = {
new = function(text,x,y,r,g,b)
	local txt = ui_base.new()
	txt.text = text
	txt.x=x or 0 txt.y=y or 0 txt.r=r or 255 txt.g=g or 255 txt.b=b or 255
	function txt:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	txt:drawadd(function(self,x,y) tpt.drawtext(x or self.x,y or self.y,self.text,self.r,self.g,self.b) end)
	txt:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	function txt:process() return false end
	return txt
end,
--Scrolls while holding mouse over
newscroll = function(text,x,y,vis,r,g,b)
	local txt = ui_text.new(text,x,y,r,g,b)
	if tpt.textwidth(text)<vis then return txt end
	txt.visible=vis
	txt.length=string.len(text)
	txt.start=1
	txt.drawlist={} --reset draw
	txt.timer=socket.gettime()+3
	function txt:cuttext(self)
		local last = self.start+1
		while tpt.textwidth(self.text:sub(self.start,last))<txt.visible and last<=self.length do
			last = last+1
		end
		self.last=last-1
	end
	txt:cuttext(txt)
	txt.minlast=txt.last-1
	txt.ppl=((txt.visible-6)/(txt.length-txt.minlast+1))
	txt:drawadd(function(self,x,y)
		if socket.gettime() > self.timer then
			if self.last >= self.length then
				self.start = 1
				self:cuttext(self)
				self.timer = socket.gettime()+3
			else
				self.start = self.start + 1
				self:cuttext(self)
				if self.last >= self.length then
					self.timer = socket.gettime()+3
				else
					self.timer = socket.gettime()+.15
				end
			end
		end
		tpt.drawtext(x or self.x,y or self.y, self.text:sub(self.start,self.last) ,self.r,self.g,self.b)
	end)
	function txt:process(mx,my,button,event,wheel)
		if event==3 then
			local newlast = math.floor((mx-self.x)/self.ppl)+self.minlast
			if newlast<self.minlast then newlast=self.minlast end
			if newlast>0 and newlast~=self.last then
				local newstart=1
				while tpt.textwidth(self.text:sub(newstart,newlast))>= self.visible do
					newstart=newstart+1
				end
				self.start=newstart self.last=newlast
				self.timer = socket.gettime()+3
			end
		end
	end
	return txt
end
}
ui_scrollbar = {
new = function(x,y,h,t,m)
	local bar = ui_base.new() --use line object as base?
	bar.x=x bar.y=y bar.h=h
	bar.total=t
	bar.numshown=m
	bar.pos=0
	bar.length=math.floor((1/math.ceil(bar.total-bar.numshown+1))*bar.h)
	bar.soffset=math.floor(bar.pos*((bar.h-bar.length)/(bar.total-bar.numshown)))
	function bar:update(total,shown,pos)
		self.pos=pos or 0
		if self.pos<0 then self.pos=0 end
		self.total=total
		self.numshown=shown
		self.length= math.floor((1/math.ceil(self.total-self.numshown+1))*self.h)
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	function bar:move(wheel)
		self.pos = self.pos-wheel
		if self.pos < 0 then self.pos=0 end
		if self.pos > (self.total-self.numshown) then self.pos=(self.total-self.numshown) end
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	bar:drawadd(function(self)
		if self.total > self.numshown then
			tpt.drawline(self.x,self.y+self.soffset,self.x,self.y+self.soffset+self.length)
		end
	end)
	bar:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	function bar:process(mx,my,button,event,wheel)
		if wheel~=0 and not MANAGER.hidden then
			if self.total > self.numshown then
				local previous = self.pos
				self:move(wheel)
				if self.pos~=previous then
					return previous-self.pos
				end
			end
		end
		--possibly click the bar and drag?
		return false
	end
	return bar
end
}
ui_button = {
new = function(x,y,w,h,f,text)
	local b = ui_box.new(x,y,w,h)
	b.f=f
	b.t=ui_text.new(text,x+2,y+2)
	b.drawbox=false
	b.clicked=false
	b.almostselected=false
	b.invert=true
	b:setbackground(50,250,50,155)
	b:drawadd(function(self)
		if self.invert and self.almostselected then
			self.almostselected=false
			tpt.fillrect(self.x,self.y,self.w,self.h)
			local tr=self.t.r local tg=self.t.g local tb=self.t.b
			b.t:setcolor(0,0,0)
			b.t:draw()
			b.t:setcolor(tr,tg,tb)
		else
			if tpt.mousex>=self.x and tpt.mousex<=self.x2 and tpt.mousey>=self.y and tpt.mousey<=self.y2 then
				self.drawbackground=true
			else
				self.drawbackground=false
			end
			b.t:draw()
		end
	end)
	b:moveadd(function(self,x,y)
		self.t:onmove(x,y)
	end)
	function b:process(mx,my,button,event,wheel)
		local clicked = self.clicked
		if event==2 then self.clicked = false end
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2 then return false end
		if event==1 then
			self.clicked=true
		elseif clicked then
			if event==3 then self.almostselected=true end
			if event==2 then self:f() end
			return true
		end
	end
	return b
end
}
ui_tooltip = {
new = function(x,y,w,text)
	local b = ui_box.new(x,y-1,w,0)
	function b:updatetooltip(tooltip)
		self.tooltip = tooltip
		self.length = #tooltip
		self.lines = 1

		local linebreak,lastspace = 0,nil
		for i=0,#self.tooltip do
			local width = tpt.textwidth(tooltip:sub(linebreak,i+1))
			if width > self.w/2 and tooltip:sub(i,i):match("[%s,_%.%-?!]") then
				lastspace = i
			end
			local isnewline = (self.tooltip:sub(i,i) == '\n')
			if width > self.w or isnewline then
				local pos = (i==#tooltip or not lastspace) and i or lastspace
				self.lines = self.lines + 1
				if self.tooltip:sub(pos,pos) == ' ' then
					self.tooltip = self.tooltip:sub(1,pos-1).."\n"..self.tooltip:sub(pos+1)
				elseif not isnewline then
					self.length = self.length + 1
					self.tooltip = self.tooltip:sub(1,pos-1).."\n"..self.tooltip:sub(pos)
					i = i + 1
					pos = pos + 1
				end
				linebreak = pos+1
				lastspace = nil
			end
		end
		self.h = self.lines*12+2
		if self.y + self.h > gfx.HEIGHT then
			local movement = (gfx.HEIGHT-self.h-1)-self.y
			if self.y+movement < 0 then
				movement = -self.y
			end
			self:onmove(0, movement)
		end
		--self.w = tpt.textwidth(self.tooltip)+3
		self.drawbox = tooltip ~= ""
		self.drawbackground = tooltip ~= ""
	end
	function b:settooltip(tooltip_)
		tooltip:onmove(tpt.mousex+5-tooltip.x, tpt.mousey+5-tooltip.y)
		tooltip:updatetooltip(tooltip_)
	end
	b:updatetooltip(text)
	b:setbackground(0,0,0,255)
	b.drawbackground = true
	b:drawadd(function(self)
		if self.tooltip ~= "" then
			tpt.drawtext(self.x+1,self.y+2,self.tooltip)
		end
		self:updatetooltip("")
	end)
	function b:process(mx,my,button,event,wheel) end
	return b
end
}
ui_checkbox = {
up_button = function(x,y,w,h,f,text)
	local b=ui_button.new(x,y,w,h,f,text)
	b.canupdate=false
	return b
end,
new_button = function(x,y,w,h,splitx,f,f2,text,localscript)
	local b = ui_box.new(x,y,splitx,h)
	b.f=f b.f2=f2
	b.localscript=localscript
	b.splitx = splitx
	b.t=ui_text.newscroll(text,x+24,y+2,splitx-24)
	b.clicked=false
	b.selected=false
	b.checkbut=ui_checkbox.up_button(x+splitx+9,y,33,9,ui_button.scriptcheck,"Update")
	b.drawbox=false
	b:setbackground(127,127,127,100)
	b:drawadd(function(self)
		if self.t.text == "" then return end
		self.drawbackground = false
		if tpt.mousey >= self.y and tpt.mousey < self.y2 then
			if tpt.mousex >= self.x and tpt.mousex < self.x+8 then
				if self.localscript then
					tooltip:settooltip("delete this script")
				else
					tooltip:settooltip("view script in browser")
				end
			elseif tpt.mousex>=self.x and tpt.mousex<self.x2 then
				local script
				if online and onlinescripts[self.ID]["description"] then
					script = onlinescripts[self.ID]
				elseif not online and localscripts[self.ID] then
					script = localscripts[self.ID]
				end
				if script then
					tooltip:settooltip(script["name"].." by "..script["author"].."\n\n"..script["description"])
				end
				self.drawbackground = true
			elseif tpt.mousex >= self.x2 then
				if tpt.mousex < self.x2+9 and self.running then
					tooltip:settooltip(online and "downloaded" or "running")
				elseif tpt.mousex >= self.x2+9 and tpt.mousex < self.x2+43 and self.checkbut.canupdate and onlinescripts[self.ID] and onlinescripts[self.ID]["changelog"] then
					tooltip:settooltip(onlinescripts[self.ID]["changelog"])
				end
			end
		end
		self.t:draw()
		if self.localscript then
			local swapicon = tpt.version.jacob1s_mod_build and tpt.version.jacob1s_mod_build > 76
			local offsetX = swapicon and 1 or 0
			local offsetY = swapicon and 2 or 0
			local innericon = swapicon and icons["delete1"] or icons["delete2"]
			local outericon = swapicon and icons["delete2"] or icons["delete1"]
			if self.deletealmostselected then
				self.deletealmostselected = false
				tpt.drawtext(self.x+1+offsetX, self.y+1+offsetY, innericon, 255, 48, 32, 255)
			else
				tpt.drawtext(self.x+1+offsetX, self.y+1+offsetY, innericon, 160, 48, 32, 255)
			end
			tpt.drawtext(self.x+1+offsetX, self.y+1+offsetY, outericon, 255, 255, 255, 255)
		else
			tpt.drawtext(self.x+1, self.y+1, icons["folder"], 255, 200, 80, 255)
		end
		tpt.drawrect(self.x+12,self.y+1,8,8)
		if self.almostselected then self.almostselected=false tpt.fillrect(self.x+12,self.y+1,8,8,150,150,150)
		elseif self.selected then tpt.fillrect(self.x+12,self.y+1,8,8) end
		local filepath = self.ID and localscripts[self.ID] and localscripts[self.ID]["path"] or self.t.text
		if self.running then tpt.drawtext(self.x+self.splitx+2,self.y+2,online and "D" or "R") end
		if self.checkbut.canupdate then self.checkbut:draw() end
	end)
	b:moveadd(function(self,x,y)
		self.t:onmove(x,y)
		self.checkbut:onmove(x,y)
	end)
	function b:process(mx,my,button,event,wheel)
		if self.f2 and mx <= self.x+8 then
			if event==1 then
				self.clicked = 1
			elseif self.clicked == 1 then
				if event==3 then self.deletealmostselected = true end
				if event==2 then self:f2() end
			end
		elseif self.f and mx<=self.x+self.splitx then
			if event==1 then
				self.clicked = 2
			elseif self.clicked == 2 then
				if event==3 then self.almostselected=true end
				if event==2 then self:f() end
				self.t:process(mx,my,button,event,wheel)
			end
		else
			if self.checkbut.canupdate then self.checkbut:process(mx,my,button,event,wheel) end
		end
		return true
	end
	return b
end,
new = function(x,y,w,h)
	local box = ui_box.new(x,y,w,h)
	box.list={}
	box.numlist = 0
	box.max_lines = math.floor(box.h/10)-1
	box.max_text_width = math.floor(box.w*0.8)
	box.splitx=x+box.max_text_width
	box.scrollbar = ui_scrollbar.new(box.x2-2,box.y+11,box.h-12,0,box.max_lines)
	box.lines={
		ui_line.new(box.x+1,box.y+10,box.x2-1,box.y+10,170,170,170),
		ui_line.new(box.x+22,box.y+10,box.x+22,box.y2-1,170,170,170),
		ui_line.new(box.splitx,box.y+10,box.splitx,box.y2-1,170,170,170),
		ui_line.new(box.splitx+9,box.y+10,box.splitx+9,box.y2-1,170,170,170),
	}
	function box:updatescroll()
		self.scrollbar:update(self.numlist,self.max_lines)
	end
	function box:clear()
		self.list={}
		self.numlist=0
	end
	function box:add(f,f2,text,localscript)
		local but = ui_checkbox.new_button(self.x,self.y+1+((self.numlist+1)*10),tpt.textwidth(text)+4,10,self.max_text_width,f,f2,text,localscript)
		table.insert(self.list,but)
		self.numlist = #self.list
		return but
	end
	box:drawadd(function (self)
		tpt.drawtext(self.x+24,self.y+2,"Files in "..TPT_LUA_PATH.." folder")
		tpt.drawtext(self.splitx+11,self.y+2,"Update")
		for i,line in ipairs(self.lines) do
			line:draw()
		end
		self.scrollbar:draw()
		local restart = false
		for i,check in ipairs(self.list) do
			local filepath = check.ID and localscripts[check.ID] and localscripts[check.ID]["path"] or check.t.text
			if not check.selected and running[filepath] then
				restart = true
			end
			if i>self.scrollbar.pos and i<=self.scrollbar.pos+self.max_lines then
				check:draw()
			end
		end
		requiresrestart = restart and not online
	end)
	box:moveadd(function(self,x,y)
		for i,line in ipairs(self.lines) do
			line:onmove(x,y)
		end
		for i,check in ipairs(self.list) do
			check:onmove(x,y)
		end
	end)
	function box:scroll(amount)
		local move = amount*10
		if move==0 then return end
		for i,check in ipairs(self.list) do
			check:onmove(0,move)
		end
	end
	function box:process(mx,my,button,event,wheel)
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2-7 then return false end
		local scrolled = self.scrollbar:process(mx,my,button,event,wheel)
		if scrolled then self:scroll(scrolled) end
		local which = math.floor((my-self.y-11)/10)+1
		if which>0 and which<=self.numlist then self.list[which+self.scrollbar.pos]:process(mx,my,button,event,wheel) end
		if event == 2 then
			for i,v in ipairs(self.list) do v.clicked = false end
		end
		return true
	end
	return box
end
}
ui_console = {
new = function(x,y,w,h)
	local con = ui_box.new(x,y,w,h)
	con.shown_lines = math.floor(con.h/10)
	con.max_lines = 300
	con.max_width = con.w-4
	con.lines = {}
	con.scrollbar = ui_scrollbar.new(con.x2-2,con.y+1,con.h-2,0,con.shown_lines)
	con:drawadd(function(self)
		self.scrollbar:draw()
		local count=0
		for i,line in ipairs(self.lines) do
			if i>self.scrollbar.pos and i<= self.scrollbar.pos+self.shown_lines then
				line:draw(self.x+3,self.y+3+(count*10))
				count = count+1
			end
		end
	end)
	con:moveadd(function(self,x,y)
		self.scrollbar:onmove(x,y)
	end)
	function con:clear()
		self.lines = {}
		self.scrollbar:update(0,con.shown_lines)
	end
	function con:addstr(str,r,g,b)
		str = tostring(str)
		local nextl = str:find('\n')
		while nextl do
			local line = str:sub(1,nextl-1)
			self:addline(line,r,g,b)
			str = str:sub(nextl+1)
			nextl = str:find('\n')
		end
		self:addline(str,r,g,b) --anything leftover
	end
	function con:addline(line,r,g,b)
		if not line or line=="" then return end --No blank lines
		table.insert(self.lines,ui_text.newscroll(line,self.x,0,self.max_width,r,g,b))
		if #self.lines>self.max_lines then table.remove(self.lines,1) end
		self.scrollbar:update(#self.lines,self.shown_lines,#self.lines-self.shown_lines)
	end
	function con:process(mx,my,button,event,wheel)
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2 then return false end
		self.scrollbar:process(mx,my,button,event,wheel)
		local which = math.floor((my-self.y-1)/10)+1
		if which>0 and which<=self.shown_lines and self.lines[which+self.scrollbar.pos] then self.lines[which+self.scrollbar.pos]:process(mx,my,button,event,wheel) end
		return true
	end
	return con
end
}
ui_window = {
new = function(x,y,w,h)
	local w=ui_box.new(x,y,w,h)
	w.sub={}
	function w:add(m,name)
		if name then w[name]=m end
		table.insert(self.sub,m)
	end
	w:drawadd(function(self)
		for i,sub in ipairs(self.sub) do
			sub:draw()
		end
	end)
	w:moveadd(function(self,x,y)
		for i,sub in ipairs(self.sub) do
			sub:onmove(x,y)
		end
	end)
	function w:process(mx,my,button,event,wheel)
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2 then if button == 0 then return end ui_button.sidepressed() return true end
		local ret
		for i,sub in ipairs(self.sub) do
			if sub:process(mx,my,button,event,wheel) then ret = true end
		end
		return ret
	end
	return w
end
}
--Main window with everything!
local mainwindow = ui_window.new(50,50,525,300)
mainwindow:setbackground(10,10,10,235) mainwindow.drawbackground=true
mainwindow:add(ui_console.new(275,148,300,189),"menuconsole")
mainwindow:add(ui_checkbox.new(50,80,225,257),"checkbox")
tooltip = ui_tooltip.new(0,1,250,"")

--Some API functions you can call from other scripts
--put 'using_manager=MANAGER ~= nil' or similar in your scripts, using_manager will be true if the manager is active
--Print a message to the manager console, can be colored
function MANAGER.print(msg,...)
	mainwindow.menuconsole:addstr(msg,...)
end
--downloads and returns a file, so you can do whatever...
local download_file
function MANAGER.download(url)
	return download_file(url)
end
function MANAGER.scriptinfo(id)
	local url = "http://starcatcher.us/scripts/main.lua"
	if id then
		url = url.."?info="..id
	end
	local info = download_file(url)
	infotable = readScriptInfo(info)
	return id and infotable[id] or infotable
end
--Get various info about the system (operating system, script directory, path seperator, if socket is loaded)
function MANAGER.sysinfo()
	return {["OS"]=OS, ["scriptDir"]=TPT_LUA_PATH, ["pathSep"]=PATH_SEP, ["exeName"] = EXE_NAME}
end
--Save a setting in the autorun settings file, ident should be your script name no one else would use.
--Name is variable name, val is the value which will be saved/returned as a string
function MANAGER.savesetting(ident,name,val)
	ident = tostring(ident)
	name = tostring(name)
	val = tostring(val)
	if settings[ident] then settings[ident][name]=val
	else settings[ident]={[name]=val} end
	save_last()
end
--Get a previously saved value, if it has one
function MANAGER.getsetting(ident,name)
	if settings[ident] then return settings[ident][name] end
	return nil
end
--delete a setting, leave name nil to delete all of ident
function MANAGER.delsetting(ident,name)
	if settings[ident] then
	if name then settings[ident][name]=nil
	else settings[ident]=nil end
	save_last()
	end
end

--mniip's download thing (mostly)
local pattern = "http://w*%.?(.-)(/.*)"
function download_file(url)
	local _,_,host,rest = url:find(pattern)
	if not host or not rest then MANAGER.print("Bad link") return end
	local conn=socket.tcp()
	if not conn then return end
	local succ=pcall(conn.connect,conn,host,80)
	conn:settimeout(5)
	if not succ then return end
	local userAgent = "PowderToy/"..tpt.version.major.."."..tpt.version.minor.."."..tpt.version.build.." ("..((OS == "WIN32" or OS == "WIN64") and "WIN; " or (os == "MACOSX" and "OSX; " or "LIN; "))..(jacobsmod and "M1" or "M0")..") SCRIPT/"..MANAGER.version
	succ,resp,something=pcall(conn.send,conn,"GET "..rest.." HTTP/1.1\r\nHost: "..host.."\r\nConnection: close\r\nUser-Agent: "..userAgent.."\r\n\r\n")
	if not succ then return end
	local data=""
	local c=""
	while c do
		c=conn:receive("*l")
		if c then
			data=data.."\n"..c
		end
	end
	if data=="" then MANAGER.print("no data") return end
	local first,last,code = data:find("HTTP/1%.1 (.-) .-\n")
	while last do
		data = data:sub(last+1)
		first,last,header = data:find("^([^\n]-:.-)\n")
		--read something from headers?
		if header then
			if tonumber(code)==302 then
				local _,_,new = header:find("^Location: (.*)")
				if new then return download_file(new) end
			end
		end
	end
	if host:find("pastebin.com") then --pastebin adds some weird numbers
		_,_,data=data:find("\n[^\n]*\n(.*)\n.+\n$")
	end
	return data
end
--Downloads to a location
local function download_script(ID,location)
	local file = download_file("http://starcatcher.us/scripts/main.lua?get="..ID)
	if file then
		f=io.open(location,"w")
		f:write(file)
		f:close()
		return true
	end
	return false
end
--Restart exe (if named correctly)
local function do_restart()
	save_last()
	if platform then
		platform.restart()
	end
	if OS == "WIN32" or OS == "WIN64" then
		os.execute("TASKKILL /IM \""..EXE_NAME.."\" /F &&START .\\\""..EXE_NAME.."\"")
	elseif OS == "OSX" then
		MANAGER.print("Can't restart on OS X when using game versions less than 91.0, please manually close and reopen The Powder Toy")
		return
	else
		os.execute("killall -s KILL \""..EXE_NAME.."\" && ./\""..EXE_NAME.."\"")
	end
	MANAGER.print("Restart failed, do you have the exe name right?",255,0,0)
end
local function open_link(url)
	if platform then
		platform.openLink(url)
	else
		local command = (OS == "WIN32" or OS == "WIN64") and "start" or (OS == "MACOSX" and "open" or "xdg-open")
		os.execute(command.." "..url)
	end
end
--TPT interface
local function step()
	if jacobsmod then
		tpt.fillrect(0,0,gfx.WIDTH,gfx.HEIGHT,0,0,0,150)
	else
		tpt.fillrect(-1,-1,gfx.WIDTH,gfx.HEIGHT,0,0,0,150)
	end
	mainwindow:draw()
	tpt.drawtext(280,140,"Console Output:")
	if requiresrestart then
		tpt.drawtext(280,88,"Disabling a script requires a restart for effect!",255,50,50)
	end
	tpt.drawtext(55,55,"Click a script to toggle, hit DONE when finished")
	tpt.drawtext(474,55,"Script Manager v"..MANAGER.version)--479 for simple versions
	tooltip:draw()
end
local function mouseclick(mousex,mousey,button,event,wheel)
	sidebutton:process(mousex,mousey,button,event,wheel)
	if MANAGER.hidden then return true end

	if mousex>612 or mousey>384 then return false end
	mainwindow:process(mousex,mousey,button,event,wheel)
	return false
end
local jacobsmod_old_menu_check = false
local function keypress(key,nkey,modifier,event)
	if jacobsmod and (key == 'o' or nkey == 96) and event == 1 then jacobsmod_old_menu_check = true end
	if nkey==27 and not MANAGER.hidden then MANAGER.hidden=true return false end
	if MANAGER.hidden then return end

	if event == 1 then
		if key == "[" then
			mainwindow:process(mainwindow.x+30, mainwindow.y+30, 0, 2, 1)
		elseif key == "]" then
			mainwindow:process(mainwindow.x+30, mainwindow.y+30, 0, 2, -1)
		end
	end
	return false
end
--small button on right to bring up main menu
local WHITE = {70,70,70,255}
local BLACK = {0,0,0,255}
local ICON = math.random(2) --pick a random icon
local lua_letters= {{{2,2,2,7},{2,7,4,7},{6,7,6,11},{6,11,8,11},{8,7,8,11},{10,11,12,11},{10,11,10,15},{11,13,11,13},{12,11,12,15},},
	{{2,3,2,13},{2,14,7,14},{4,3,4,12},{4,12,7,12},{7,3,7,12},{9,3,12,3},{9,3,9,14},{10,8,11,8},{12,3,12,14},}}
local function smallstep()
	gfx.drawRect(sidebutton.x, sidebutton.y+1, sidebutton.w+1, sidebutton.h+1,200,200,200)
	local color=WHITE
	if not MANAGER.hidden then
		step()
		gfx.fillRect(sidebutton.x, sidebutton.y+1, sidebutton.w+1, sidebutton.h+1)
		color=BLACK
	end
	for i,dline in ipairs(lua_letters[ICON]) do
		tpt.drawline(dline[1]+sidebutton.x,dline[2]+sidebutton.y,dline[3]+sidebutton.x,dline[4]+sidebutton.y,color[1],color[2],color[3])
	end
	if jacobsmod_old_menu_check then
		local ypos = 134
		if jacobsmod and tpt.oldmenu and tpt.oldmenu()==1 then
			ypos = 390
		elseif tpt.num_menus then
			ypos = 390-16*tpt.num_menus()-(not jacobsmod and 16 or 0)
		end
		sidebutton:onmove(0, ypos-sidebutton.y)
		jacobsmod_old_menu_check = false
	end
end
--button functions on click
function ui_button.reloadpressed(self)
	load_filenames()
	load_downloaded()
	gen_buttons()
	mainwindow.checkbox:updatescroll()
	if num_files == 0 then
		MANAGER.print("No scripts found in '"..TPT_LUA_PATH.."' folder",255,255,0)
		fs.makeDirectory(TPT_LUA_PATH)
	else
		MANAGER.print("Reloaded file list, found "..num_files.." scripts")
	end
end
function ui_button.selectnone(self)
	for i,but in ipairs(mainwindow.checkbox.list) do
		but.selected = false
	end
end
function ui_button.consoleclear(self)
	mainwindow.menuconsole:clear()
end
function ui_button.changedir(self)
	local last = TPT_LUA_PATH
	local new = tpt.input("Change search directory","Enter the folder where your scripts are",TPT_LUA_PATH,TPT_LUA_PATH)
	if new~=last and new~="" then
		fs.removeFile(last..PATH_SEP.."autorunsettings.txt")
		MANAGER.print("Directory changed to "..new,255,255,0)
		TPT_LUA_PATH = new
	end
	ui_button.reloadpressed()
	save_last()
end
function ui_button.uploadscript(self)
	if not online then
		local command = (OS == "WIN32" or OS == "WIN64") and "start" or (OS == "MACOSX" and "open" or "xdg-open")
		os.execute(command.." "..TPT_LUA_PATH)
	else
		open_link("https://starcatcher.us/scripts/paste.lua")
	end
end
local lastpaused
function ui_button.sidepressed(self)
	if TPTMP and TPTMP.chatHidden == false then print("minimize TPTMP before opening the manager") return end
	MANAGER.hidden = not MANAGER.hidden
	ui_button.localview()
	if not MANAGER.hidden then
		lastpaused = tpt.set_pause()
		tpt.set_pause(1)
		ui_button.reloadpressed()
	else
		tpt.set_pause(lastpaused)
	end
end
local donebutton
function ui_button.donepressed(self)
	MANAGER.hidden = true
	for i,but in ipairs(mainwindow.checkbox.list) do
		local filepath = but.ID and localscripts[but.ID]["path"] or but.t.text
		if but.selected then
			if requiresrestart then
				running[filepath] = true
			else
				if not running[filepath] then
					local status,err = pcall(dofile,TPT_LUA_PATH..PATH_SEP..filepath)
					if not status then
						MANAGER.print(err,255,0,0)
						print(err)
						but.selected = false
					else
						MANAGER.print("Started "..filepath)
						running[filepath] = true
					end
				end
			end
		elseif running[filepath] then
			running[filepath] = nil
		end
	end
	if requiresrestart then do_restart() return end
	save_last()
end
function ui_button.downloadpressed(self)
	for i,but in ipairs(mainwindow.checkbox.list) do
		if but.selected then
			--maybe do better display names later
			local displayName
			local function get_script(butt)
				local script = download_file("http://starcatcher.us/scripts/main.lua?get="..butt.ID)
				displayName = "downloaded"..PATH_SEP..butt.ID.." "..onlinescripts[butt.ID].author:gsub("[^%w _-]", "_").."-"..onlinescripts[butt.ID].name:gsub("[^%w _-]", "_")..".lua"
				local name = TPT_LUA_PATH..PATH_SEP..displayName
				if not fs.exists(TPT_LUA_PATH..PATH_SEP.."downloaded") then
					fs.makeDirectory(TPT_LUA_PATH..PATH_SEP.."downloaded")
				end
				local file = io.open(name, "w")
				if not file then error("could not open "..name) end
				file:write(script)
				file:close()
				if localscripts[butt.ID] and localscripts[butt.ID]["path"] ~= displayName then
					local oldpath = localscripts[butt.ID]["path"]
					fs.removeFile(TPT_LUA_PATH.."/"..oldpath:gsub("\\","/"))
					running[oldpath] = nil
				end
				localscripts[butt.ID] = onlinescripts[butt.ID]
				localscripts[butt.ID]["path"] = displayName
				dofile(name)
			end
			local status,err = pcall(get_script, but)
			if not status then
				MANAGER.print(err,255,0,0)
				print(err)
				but.selected = false
			else
				MANAGER.print("Downloaded and started "..but.t.text)
				running[displayName] = true
			end
		end
	end
	MANAGER.hidden = true
	ui_button.localview()
	save_last()
end

function ui_button.pressed(self)
	self.selected = not self.selected
end
function ui_button.delete(self)
	--there is no tpt.confirm() yet
	if tpt.input("Delete File", "Delete "..self.t.text.."?", "yes", "no") == "yes" then
		local filepath = self.ID and localscripts[self.ID]["path"] or self.t.text
		fs.removeFile(TPT_LUA_PATH.."/"..filepath:gsub("\\","/"))
		if running[filepath] then running[filepath] = nil end
		if localscripts[self.ID] then localscripts[self.ID] = nil end
		save_last()
		ui_button.localview()
		load_filenames()
		gen_buttons()
	end
end
function ui_button.viewonline(self)
	open_link("https://starcatcher.us/scripts?view="..self.ID)
end
function ui_button.scriptcheck(self)
	local oldpath = localscripts[self.ID]["path"]
	local newpath = "downloaded"..PATH_SEP..self.ID.." "..onlinescripts[self.ID].author:gsub("[^%w _-]", "_").."-"..onlinescripts[self.ID].name:gsub("[^%w _-]", "_")..".lua"
	if download_script(self.ID,TPT_LUA_PATH..PATH_SEP..newpath) then
		self.canupdate = false
		localscripts[self.ID] = onlinescripts[self.ID]
		localscripts[self.ID]["path"] = newpath
		if oldpath:gsub("\\","/") ~= newpath:gsub("\\","/") then
			fs.removeFile(TPT_LUA_PATH.."/"..oldpath:gsub("\\","/"))
			if running[oldpath] then
				running[newpath],running[oldpath] = running[oldpath],nil
			end
		end
		if running[newpath] then
			do_restart()
		else
			save_last()
			MANAGER.print("Updated "..onlinescripts[self.ID]["name"])
		end
	end
end
function ui_button.doupdate(self)
	if jacobsmod and jacobsmod >= 30 then
		fileSystem.move("scriptmanager.lua", "scriptmanagerold.lua")
		download_script(1, 'scriptmanager.lua')
	else
		fileSystem.move("autorun.lua", "autorunold.lua")
		download_script(1, 'autorun.lua')
	end
	localscripts[1] = updatetable[1]
	do_restart()
end
local uploadscriptbutton
function ui_button.localview(self)
	if online then
		online = false
		gen_buttons()
		donebutton.t.text = "DONE"
		donebutton.w = 29 donebutton.x2 = donebutton.x + donebutton.w
		donebutton.f = ui_button.donepressed
		uploadscriptbutton.t.text = icons["folder"].." Script Folder"
	end
end
function ui_button.onlineview(self)
	if not online then
		online = true
		gen_buttons()
		donebutton.t.text = "DOWNLOAD"
		donebutton.w = 55 donebutton.x2 = donebutton.x + donebutton.w
		donebutton.f = ui_button.downloadpressed
		uploadscriptbutton.t.text = "Upload Script"
	end
end
--add buttons to window
donebutton = ui_button.new(55,339,29,10,ui_button.donepressed,"DONE")
mainwindow:add(donebutton)
mainwindow:add(ui_button.new(134,339,40,10,ui_button.sidepressed,"CANCEL"))
--mainwindow:add(ui_button.new(152,339,29,10,ui_button.selectnone,"NONE"))
local nonebutton = ui_button.new(62,81,8,8,ui_button.selectnone,"")
nonebutton.drawbox = true
mainwindow:add(nonebutton)
mainwindow:add(ui_button.new(538,339,33,10,ui_button.consoleclear,"CLEAR"))
mainwindow:add(ui_button.new(278,67,39,10,ui_button.reloadpressed,"RELOAD"))
mainwindow:add(ui_button.new(378,67,51,10,ui_button.changedir,"Change dir"))
uploadscriptbutton = ui_button.new(478,67,79,10,ui_button.uploadscript, icons["folder"].." Script Folder")
mainwindow:add(uploadscriptbutton)
local tempbutton = ui_button.new(60, 65, 30, 10, ui_button.localview, "Local")
tempbutton.drawbox = true
mainwindow:add(tempbutton)
tempbutton = ui_button.new(100, 65, 35, 10, ui_button.onlineview, "Online")
tempbutton.drawbox = true
mainwindow:add(tempbutton)
local ypos = 134
if jacobsmod and tpt.oldmenu and tpt.oldmenu()==1 then
	ypos = 390
elseif tpt.num_menus then
	ypos = 390-16*tpt.num_menus()-(not jacobsmod and 16 or 0)
end
sidebutton = ui_button.new(gfx.WIDTH-16,ypos,14,15,ui_button.sidepressed,'')

local function gen_buttons_local()
	local count = 0
	local sorted = {}
	for k,v in pairs(localscripts) do if v.ID ~= 1 then table.insert(sorted, v) end end
	table.sort(sorted, function(first,second) return first.name:lower() < second.name:lower() end)
	for i,v in ipairs(sorted) do
		local check = mainwindow.checkbox:add(ui_button.pressed,ui_button.delete,v.name,true)
		check.ID = v.ID
		if running[v.path] then
			check.running = true
			check.selected = true
		end
		count = count + 1
	end
	if #sorted >= 5 and #filenames >= 5 then
		mainwindow.checkbox:add(nil, nil, "", false) --empty space to separate things
	end
	for i=1,#filenames do
		local check = mainwindow.checkbox:add(ui_button.pressed,ui_button.delete,filenames[i],true)
		if running[filenames[i]] then
			check.running = true
			check.selected = true
		end
	end
	num_files = count + #filenames
end
local function gen_buttons_online()
	local list = download_file("http://starcatcher.us/scripts/main.lua")
	onlinescripts = readScriptInfo(list)
	local sorted = {}
	for k,v in pairs(onlinescripts) do table.insert(sorted, v) end
	table.sort(sorted, function(first,second) return first.ID < second.ID end)
	for k,v in pairs(sorted) do
		local check = mainwindow.checkbox:add(ui_button.pressed, ui_button.viewonline, v.name, false)
		check.ID = v.ID
		check.checkbut.ID = v.ID
		if localscripts[v.ID] then
			check.running = true
			if tonumber(v.version) > tonumber(localscripts[check.ID].version) then
				check.checkbut.canupdate = true
			end
		end
	end
	if first_online then
		first_online = false
		local updateinfo = download_file("http://starcatcher.us/scripts/main.lua?info=1")
		updatetable = readScriptInfo(updateinfo)
		if not updatetable[1] then return end
		if tonumber(updatetable[1].version) > scriptversion then
			local updatebutton = ui_button.new(278,127,40,10,ui_button.doupdate,"UPDATE")
			updatebutton.t:setcolor(25,255,25)
			mainwindow:add(updatebutton)
			MANAGER.print("A script manager update is available! Click UPDATE",25,255,55)
			MANAGER.print(updatetable[1].changelog,25,255,55)
		end
	end
end
gen_buttons = function()
	mainwindow.checkbox:clear()
	if online then
		gen_buttons_online()
	else
		gen_buttons_local()
	end
	mainwindow.checkbox:updatescroll()
end
gen_buttons()

--register manager first
tpt.register_step(smallstep)
--load previously running scripts
local started = ""
for prev,v in pairs(running) do
	local status,err = pcall(dofile,TPT_LUA_PATH..PATH_SEP..prev)
	if not status then
		MANAGER.print(err,255,0,0)
		running[prev] = nil
	else
		started=started.." "..prev
		local newbut = mainwindow.checkbox:add(ui_button.pressed,prev,nil,false)
		newbut.selected=true
	end
end
save_last()
if started~="" then
	MANAGER.print("Auto started"..started)
end
tpt.register_mouseevent(mouseclick)
tpt.register_keypress(keypress)


--Cracker64's Powder Toy Multiplayer
--I highly recommend to use my Autorun Script Manager

local versionstring = "0.101"

--TODO's
--FIGH,STKM,STK2,LIGH need a few more creation adjustments
--Some more server functions
-------------------------------------------------------

--CHANGES:
--Lots of Fixes
--More colors!
--ESC key will unfocus, then minimize chat
--Changes from jacob, including: Support jacobsMod, keyrepeat
--Support replace mode

if TPTMP then if TPTMP.version <= 4 then TPTMP.disableMultiplayer() else error("newer version already running") end end local get_name = tpt.get_name -- if script already running, replace it
TPTMP = {["version"] = 4, ["versionStr"] = versionstring} -- script version sent on connect to ensure server protocol is the same
local issocket,socket = pcall(require,"socket")
if not sim.clearRect then error"Tpt version not supported" end
local using_manager = false
local type = type -- people like to overwrite this function with a global a lot
local _print = print
if MANAGER ~= nil or MANAGER_EXISTS then
	using_manager = true
	_print = MANAGER and MANAGER.print or MANAGER_PRINT
else
	_print = print
end
local hooks_enabled = false --hooks only enabled once you maximize the button

local PORT = 34403 --Change 34403 to your desired port
local KEYBOARD = 1 --only change if you have issues. Only other option right now is 2(finnish).
--Local player vars we need to keep
local L = {mousex=0, mousey=0, realMouseX=0, realMouseY=0, brushx=0, brushy=0, sell=1, sela=296, selr=0, selrep=0, replacemode = 0, mButt=0, mEvent=0, isDrawing=false, dcolour=0, stick2=false, chatHidden=true, flashChat=false,
shift=false, alt=false, ctrl=false, tabs = false, z=false, skipClick=false, pauseNextFrame=false, copying=false, stamp=false, placeStamp=false, lastStamp=nil, lastCopy=nil, smoved=false, rotate=false, sendScreen=false}

local tptversion = tpt.version.build
local jacobsmod = tpt.version.jacob1s_mod~=nil
math.randomseed(os.time())
local username = get_name()
if username == "" then
	username = "Guest"..math.random(10000,99999)
end
local chatwindow
local con = {connected = false,
		 socket = nil,
		 members = nil,
		 pingTime = os.time()+60}
local function disconnected(reason)
	if con.socket then
		con.socket:close()
	end
	if reason then
		chatwindow:addline(reason,255,50,50)
	else
		chatwindow:addline("Connection was closed",255,50,50)
	end
	con.connected = false
	con.members = {}
end
local function conSend(cmd,msg,endNull)
	if not con.connected then return false,"Not connected" end
	msg = msg or ""
	if endNull then msg = msg.."\0" end
	if cmd then msg = string.char(cmd)..msg end
	con.socket:settimeout(10)
	con.socket:send(msg)
	con.socket:settimeout(0)
end
local function joinChannel(chan)
	conSend(16,chan,true)
	--send some things to new channel
	conSend(34,string.char(L.brushx,L.brushy))
	conSend(37,string.char(math.floor(L.sell/256),L.sell%256))
	conSend(37,string.char(math.floor(64 + L.sela/256),L.sela%256))
	conSend(37,string.char(math.floor(128 + L.selr/256),L.selr%256))
	conSend(37,string.char(math.floor(192 + L.selrep/256),L.selrep%256))
	conSend(38,L.replacemode)
	conSend(65,string.char(math.floor(L.dcolour/16777216),math.floor(L.dcolour/65536)%256,math.floor(L.dcolour/256)%256,L.dcolour%256))
end
local function connectToServer(ip,port,nick)
	if con.connected then return false,"Already connected" end
	ip = ip or "tptmp.starcatcher.us"
	port = port or PORT
	local sock = socket.tcp()
	sock:settimeout(10)
	local s,r = sock:connect(ip,port)
	if not s then return false,r end
	sock:settimeout(0)
	sock:setoption("keepalive",true)
	sock:send(string.char(tpt.version.major)..string.char(tpt.version.minor)..string.char(TPTMP.version)..nick.."\0")
	local c,r
	while not c do
	c,r = sock:receive(1)
	if not c and r~="timeout" then break end
	end
	if not c and r~="timeout" then return false,r end

	if c~= "\1" then
	if c=="\0" then
		local err=""
		c,r = sock:receive(1)
		while c~="\0" do
		err = err..c
		c,r = sock:receive(1)
		end
		if err=="This nick is already on the server" then
			nick = nick:gsub("(.)$",function(s) local n=tonumber(s) if n and n+1 <= 9 then return n+1 else return nick:sub(-1)..'0' end end)
			return connectToServer(ip,port,nick)
		end
		return false,err
	end
	return false,"Bad Connect"
	end

	con.socket = sock
	con.connected = true
	username = nick
	conSend(34,string.char(L.brushx,L.brushy))
	conSend(37,string.char(math.floor(L.sell/256),L.sell%256))
	conSend(37,string.char(math.floor(64 + L.sela/256),L.sela%256))
	conSend(37,string.char(math.floor(128 + L.selr/256),L.selr%256))
	conSend(37,string.char(math.floor(192 + L.selrep/256),L.selrep%256))
	conSend(38,L.replacemode)
	conSend(65,string.char(math.floor(L.dcolour/16777216),math.floor(L.dcolour/65536)%256,math.floor(L.dcolour/256)%256,L.dcolour%256))
	return true
end
--get up to a null (\0)
local function conGetNull()
	con.socket:settimeout(nil)
	local c,r = con.socket:receive(1)
	if not c and r ~= "timeout" then disconnected() return nil end
	local rstring=""
	while c~="\0" do
	rstring = rstring..c
	c,r = con.socket:receive(1)
	if not c and r ~= "timeout" then disconnected() return nil end
	end
	con.socket:settimeout(0)
	return rstring
end
--get next char/byte
local function cChar()
	con.socket:settimeout(nil)
	local c,r = con.socket:receive(1)
	con.socket:settimeout(0)
	if not c then disconnected() end
	return c
end
local function cByte()
	local byte = cChar()
	return byte and byte:byte() or nil
end
--return table of arguments
local function getArgs(msg)
	if not msg then return {} end
	local args = {}
	for word in msg:gmatch("([^%s%c]+)") do
	table.insert(args,word)
	end
	return args
end

local ui_base local ui_box local ui_text local ui_button local ui_scrollbar local ui_inputbox local ui_chatbox
ui_base = {
new = function()
	local b={}
	b.drawlist = {}
	function b:drawadd(f)
		table.insert(self.drawlist,f)
	end
	function b:draw(...)
		for _,f in ipairs(self.drawlist) do
			if type(f)=="function" then
				f(self,...)
			end
		end
	end
	b.movelist = {}
	function b:moveadd(f)
		table.insert(self.movelist,f)
	end
	function b:onmove(x,y)
		for _,f in ipairs(self.movelist) do
			if type(f)=="function" then
				f(self,x,y)
			end
		end
	end
	return b
end
}
ui_box = {
new = function(x,y,w,h,r,g,b)
	local box=ui_base.new()
	box.x=x box.y=y box.w=w box.h=h box.x2=x+w box.y2=y+h
	box.r=70 box.g=70 box.b= 70
	function box:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	function box:setbackground(r,g,b,a) self.br=r self.bg=g self.bb=b self.ba=a end
	box.drawbox=true
	box.drawbackground=false
	box:drawadd(function(self) if self.drawbackground then gfx.fillRect(self.x,self.y,self.w+1,self.h+1,self.br,self.bg,self.bb,self.ba) end
								if self.drawbox then gfx.drawRect(self.x,self.y,self.w+1,self.h+1,self.r,self.g,self.b) end end)
	box:moveadd(function(self,x,y)
		if x then self.x=self.x+x self.x2=self.x2+x end
		if y then self.y=self.y+y self.y2=self.y2+y end
	end)
	return box
end
}
ui_text = {
new = function(text,x,y,r,g,b)
	local txt = ui_base.new()
	txt.text = text
	txt.x=x or 0 txt.y=y or 0 txt.r=r or 255 txt.g=g or 255 txt.b=b or 255
	function txt:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	txt:drawadd(function(self,x,y) gfx.drawText(x or self.x,y or self.y,self.text,self.r,self.g,self.b) end)
	txt:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	return txt
end,
--Scrolls while holding mouse over
newscroll = function(text,x,y,vis,force,r,g,b)
	local txt = ui_text.new(text,x,y,r,g,b)
	if not force and tpt.textwidth(text)<vis then return txt end
	txt.visible=vis
	txt.length=string.len(text)
	txt.start=1
	local last=2
	while tpt.textwidth(text:sub(1,last))<vis and last<=txt.length do
		last=last+1
	end
	txt.last=last-1
	txt.minlast=last-1
	txt.ppl=((txt.visible-6)/(txt.length-txt.minlast+1))
	function txt:update(text,pos)
		if text then
			self.text=text
			self.length=string.len(text)
			local last=2
			while tpt.textwidth(text:sub(1,last))<self.visible and last<=self.length do
				last=last+1
			end
			self.minlast=last-1
			self.ppl=((self.visible-6)/(self.length-self.minlast+1))
			if not pos then self.last=self.minlast end
		end
		if pos then
			if pos>=self.last and pos<=self.length then --more than current visible
				local newlast = pos
				local newstart=1
				while tpt.textwidth(self.text:sub(newstart,newlast))>= self.visible do
					newstart=newstart+1
				end
				self.start=newstart self.last=newlast
			elseif pos<self.start and pos>0 then --position less than current visible
				local newstart=pos
				local newlast=pos+1
				while tpt.textwidth(self.text:sub(newstart,newlast))<self.visible and newlast<self.length do
						newlast=newlast+1
				end
				self.start=newstart self.last=newlast-1
			end
			--keep strings as long as possible (pulls from left)
			local newlast=self.last
			if newlast<self.minlast then newlast=self.minlast end
			local newstart=1
			while tpt.textwidth(self.text:sub(newstart,newlast))>= self.visible do
					newstart=newstart+1
			end
			self.start=newstart self.last=newlast
		end
	end
	txt.drawlist={} --reset draw
	txt:drawadd(function(self,x,y)
		gfx.drawText(x or self.x,y or self.y, self.text:sub(self.start,self.last) ,self.r,self.g,self.b)
	end)
	function txt:mouseMove(mx,my,dX,dY)
		local newlast = math.floor((mx-self.x)/self.ppl)+self.minlast
		if newlast<self.minlast then newlast=self.minlast end
		if newlast>0 and newlast~=self.last then
			local newstart=1
			while tpt.textwidth(self.text:sub(newstart,newlast))>= self.visible do
				newstart=newstart+1
			end
			self.start=newstart self.last=newlast
		end
	end
	return txt
end
}
ui_inputbox = {
new=function(x,y,w,h)
	local intext=ui_box.new(x,y,w,h)
	intext.cursor=0
	intext.line=1
	intext.currentline = ""
	intext.focus=false
	intext.t=ui_text.newscroll("",x+2,y+2,w-2,true)
	intext.history={}
	intext.max_history=200
	intext.ratelimit = 0
	intext:drawadd(function(self)
		local cursoradjust=tpt.textwidth(self.t.text:sub(self.t.start,self.cursor))+2
		gfx.drawLine(self.x+cursoradjust,self.y,self.x+cursoradjust,self.y+10,255,255,255)
		self.t:draw()
	end)
	intext:moveadd(function(self,x,y) self.t:onmove(x,y) end)
	function intext:setfocus(focus)
		self.focus=focus
		if focus then self:setcolor(255,255,0)
		else self:setcolor(255,255,255) end
	end
	function intext:movecursor(amt)
		self.cursor = self.cursor+amt
		if self.cursor>self.t.length then self.cursor = self.t.length end
		if self.cursor<0 then self.cursor = 0 return end
	end
	function intext:addhistory(str)
		self.history[#self.history+1] = str
		if #self.history >= self.max_history then
			table.remove(self.history, 1)
		end
	end
	function intext:moveline(amt)
		self.line = self.line+amt
		local max = #self.currentline and #self.history+2 or #self.history+1
		if self.line>max then self.line=max
		elseif self.line<1 then self.line=1 end
		local history = self.history[self.line] or ""
		if self.line == #self.history+1 then history = self.currentline end
		self.cursor = string.len(history)
		self.t:update(history, self.cursor)
	end
	function intext:keypress(key, scan, rep, shift, ctrl, alt)
		if not self.focus then
			if key == 13 then
				self:setfocus(true)
			end
			return
		end
		-- Esc
		if key == 27 then
			self:setfocus(false)
		-- Enter
		elseif key == 13 and not rep then
			if socket.gettime() < self.ratelimit then
				return
			end
			local text = self.t.text
			if text == "" then
				self:setfocus(false)
			else
				self.cursor = 0
				self.t.text = ""
				self:addhistory(text)
				self.line = #self.history + 1
				self.currentline = ""
				self.ratelimit = socket.gettime() + 1
				return text
			end
		-- Up
		elseif key == 1073741906 then
			if socket.gettime() < self.ratelimit then
				return
			end
			self:moveline(-1)
		-- Down
		elseif key == 1073741905 then
			self:moveline(1)
		-- Right
		elseif key == 1073741903 then
			self:movecursor(1)
			self.t:update(nil, self.cursor)
		-- Left
		elseif key == 1073741904 then
			self:movecursor(-1)
			self.t:update(nil, self.cursor)
		end

		local newstr
		-- Backspace
		if key == 8 then
			if self.cursor > 0 then
				newstr = self.t.text:sub(1,self.cursor-1) .. self.t.text:sub(self.cursor+1)
				self:movecursor(-1)
			end
		-- Delete
		elseif key == 127 then
			newstr=self.t.text:sub(1,self.cursor) .. self.t.text:sub(self.cursor+2)
		-- Tab
		elseif key == 9 then
			local nickstart, nickend, nick = self.t.text:sub(1,self.cursor+1):find("([^%s%c]+)"..(self.cursor == #self.t.text and "" or " ").."$")
			if con.members and nick then
				for k, v in pairs(con.members) do
					if v.name:sub(1, #nick) == nick then
						nick = v.name
						if nickstart == 1 then
							nick = nick .. ":"
						end
						newstr = self.t.text:sub(1,nickstart-1)..nick.." "..self.t.text:sub(nickend+1, #self.t.text)
						self.cursor = nickstart + #nick
					end
				end
			end
		end
		if newstr then
			self.t:update(newstr,self.cursor)
		end
	end
	function intext:textinput(text)
		if not self.focus then
			return
		end
		-- TPT font has no unicode characters at the moment, nor does it have a good enough api to support them
		if #text > 1 or string.byte(text) < 20 or string.byte(text) > 126 then return end
		newstr = self.t.text:sub(1, self.cursor) .. text .. self.t.text:sub(self.cursor + 1)
		self.currentline = newstr
		self.t:update(newstr, self.cursor + 1)
		self:movecursor(1)
	end
	return intext
end
}
ui_scrollbar = {
new = function(x,y,h,t,m)
	local bar = ui_base.new() --use line object as base?
	bar.x=x bar.y=y bar.h=h
	bar.total=t
	bar.numshown=m
	bar.pos=0
	bar.length=math.floor((1/math.ceil(bar.total-bar.numshown+1))*bar.h)
	bar.soffset=math.floor(bar.pos*((bar.h-bar.length)/(bar.total-bar.numshown)))
	function bar:update(total,shown,pos)
		self.pos=pos or 0
		if self.pos<0 then self.pos=0 end
		self.total=total
		self.numshown=shown
		self.length= math.floor((1/math.ceil(self.total-self.numshown+1))*self.h)
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	function bar:move(wheel)
		self.pos = self.pos-wheel
		if self.pos < 0 then self.pos=0 end
		if self.pos > (self.total-self.numshown) then self.pos=(self.total-self.numshown) end
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	bar:drawadd(function(self)
		if self.total > self.numshown then
			gfx.drawLine(self.x,self.y+self.soffset,self.x,self.y+self.soffset+self.length)
		end
	end)
	bar:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	function bar:mouseWheel(mx,my,wheel)
		if wheel~=0 and not hidden_mode then
			if self.total > self.numshown then
				local previous = self.pos
				self:move(wheel)
				if self.pos~=previous then
					return wheel
				end
			end
		end
	end
	return bar
end
}
ui_button = {
new = function(x,y,w,h,f,text)
	local b = ui_box.new(x,y,w,h)
	b.f=f
	b.t=ui_text.new(text,x+2,y+2)
	b.drawbox=false
	b.clicked = false
	b.invert = false
	b:drawadd(function(self)
		if self.clicked or self.invert then
			tpt.fillrect(self.x,self.y,self.w,self.h)
			local tr=self.t.r local tg=self.t.g local tb=self.t.b
			b.t:setcolor(0,0,0)
			b.t:draw()
			b.t:setcolor(tr,tg,tb)
		else
			b.t:draw()
		end
	end)
	b:moveadd(function(self,x,y)
		self.t:onmove(x,y)
	end)
	function b:mouseDown(mouseX, mouseY, button, reason)
		if mouseX >= self.x and mouseX <= self.x2 and mouseY >= self.y and mouseY <= self.y2 then
			self.clicked = true
			return true
		end
	end
	function b:mouseMove(mouseX, mouseY, dX, dY)
		if not (mouseX >= self.x and mouseX <= self.x2 and mouseY >= self.y and mouseY <= self.y2) then
			self.clicked = false
		end
	end
	function b:mouseUp(mouseX, mouseY, button, reason)
		if self.clicked and mouseX >= self.x and mouseX <= self.x2 and mouseY >= self.y and mouseY <= self.y2 then
			self:f()
			return true
		end
	end
	return b
end
}
ui_chatbox = {
new=function(x,y,w,h)
	local chat=ui_box.new(x,y,w,h)
	chat.moving=false
	chat.lastx=0
	chat.lasty=0
	chat.relx=0
	chat.rely=0
	chat.shown_lines=math.floor(chat.h/10)-2 --one line for top, one for chat
	chat.max_width=chat.w-4
	chat.max_lines=200
	chat.lines = {}
	chat.scrollbar = ui_scrollbar.new(chat.x2-2,chat.y+11,chat.h-22,0,chat.shown_lines)
	chat.inputbox = ui_inputbox.new(x,chat.y2-10,w,10)
	chat.minimize = ui_button.new(chat.x2-15,chat.y,15,10,function() chat.moving=false chat.inputbox:setfocus(false) L.chatHidden=true TPTMP.chatHidden=true end,">>")
	chat:drawadd(function(self)
		if self.w > 175 and jacobsmod then
			gfx.drawText(self.x+self.w/2-tpt.textwidth("TPT Multiplayer, by cracker64")/2,self.y+2,"TPT Multiplayer, by cracker64")
		elseif self.w > 100 then
			gfx.drawText(self.x+self.w/2-tpt.textwidth("TPT Multiplayer")/2,self.y+2,"TPT Multiplayer")
		end
		gfx.drawLine(self.x+1,self.y+10,self.x2-1,self.y+10,120,120,120)
		self.scrollbar:draw()
		local count=0
		for i,line in ipairs(self.lines) do
			if i>self.scrollbar.pos and i<= self.scrollbar.pos+self.shown_lines then
				line:draw(self.x+3,self.y+12+(count*10))
				count = count+1
			end
		end
		self.inputbox:draw()
		self.minimize:draw()
	end)
	chat:moveadd(function(self,x,y)
		for i,line in ipairs(self.lines) do
			line:onmove(x,y)
		end
		self.scrollbar:onmove(x,y)
		self.inputbox:onmove(x,y)
		self.minimize:onmove(x,y)
	end)
	function chat:addline(line,r,g,b,noflash)
		if not line or line=="" then return end --No blank lines
		local linebreak,lastspace = 0,nil
		for i=0,#line do
			local width = tpt.textwidth(line:sub(linebreak,i+1))
			if width > self.max_width/2 and line:sub(i,i):match("[%s,_%.%-?!]") then
				lastspace = i
			end
			if width > self.max_width or i==#line then
				local pos = (i==#line or not lastspace) and i or lastspace
				table.insert(self.lines,ui_text.new(line:sub(linebreak,pos),self.x,0,r,g,b))
				linebreak = pos+1
				lastspace = nil
			end
		end
		while #self.lines>self.max_lines do table.remove(self.lines,1) end
		self.scrollbar:update(#self.lines,self.shown_lines,#self.lines-self.shown_lines)
		if L.chatHidden and not noflash then L.flashChat=true end
	end
	chat:addline("TPTMP v"..versionstring..": Type '/connect' to join server, or /list for a list of commands.",200,200,200,true)
	function chat:mouseDown(mouseX, mouseY, button)
		if L.chatHidden then return false end
		self.minimize:mouseDown(mouseX, mouseY, button)
		
		local selectedLine = math.floor((mouseY - self.y) / 10)
		-- Mouse outside chat window, defocus it
		if mouseX < self.x or mouseX > self.x2 or mouseY < self.y or mouseY > self.y2 then
			self.inputbox:setfocus(false)
			return false
		end

		-- header was grabbed, enable window movement
		if selectedLine == 0 and mouseX < self.minimize.x then
			self.moving = true
			self.lastx = mx
			self.lasty = my
			self.relx = mouseX - self.x
			self.rely = mouseY - self.y
			return true
		-- Textbox clicked
		elseif selectedLine == self.shown_lines + 1 then
			self.inputbox:setfocus(true)
			return true
		end

		-- At this point we know chatbox is selected, ensure window is focused then block mouse events
		if not self.inputbox.focus then
			self.inputbox:setfocus(true)
		end
		return true
	end
	function chat:mouseMove(mouseX, mouseY, dX, dY)
		if self.moving then
			local newx, newy = self.x + dX, self.y + dY
			if newx < 0 then dX = dX - newx end
			if newy < 0 then dY = dY - newy end
			if (newx + self.w) >= sim.XRES then dX = dX - (newx + self.w - sim.XRES) end
			if (newy + self.h) >= sim.YRES then dY = dY - (newy + self.h - sim.YRES) end

			if dX < 0 and mouseX > self.relx + self.x then dX = 0 end
			if dX > 0 and mouseX < self.relx + self.x then dX = 0 end
			if dY < 0 and mouseY > self.rely + self.y then dY = 0 end
			if dY > 0 and mouseY < self.rely + self.y then dY = 0 end
			
			self:onmove(dX, dY)
		end
		self.minimize:mouseMove(mouseX, mouseY, dX, dY)
	end
	function chat:mouseUp(mouseX, mouseY, button, reason)
		self.minimize:mouseUp(mouseX, mouseY, button, reason)
		if self.moving then
			self.moving = false
			return true
		end
	end
	function chat:mouseWheel(mouseX, mouseY, wheel)
		self.scrollbar:mouseWheel(mouseX, mouseY, wheel)
	end
	--commands for chat window
	chatcommands = {
	connect = function(self,msg,args)
		if not issocket then self:addline("No luasockets found") return end
		local newname = pcall(string.dump, get_name) and "Gue".."st"..math["random"](1111,9888) or get_name()
		local s,r = connectToServer(args[1],tonumber(args[2]), newname~="" and newname or username)
		if not s then self:addline(r,255,50,50) end
		pressedKeys = nil
	end,
	send = function(self,msg,args)
		if tonumber(args[1]) and args[2] then
		local withNull=false
		if args[2]=="true" then withNull=true end
		msg = msg:sub(#args[1]+1+(withNull and #args[2]+2 or 0))
		conSend(tonumber(args[1]),msg,withNull)
		end
	end,
	quit = function(self,msg,args)
		disconnected("Disconnected")
	end,
	disconnect = function(self,msg,args)
		disconnected("Disconnected")
	end,
	join = function(self,msg,args)
		if args[1] then
			joinChannel(args[1])
			self:addline("joined channel "..args[1],50,255,50)
		end
	end,
	sync = function(self,msg,args)
		if con.connected then L.sendScreen=true end --need to send 67 clear screen
		self:addline("Synced screen to server",255,255,50)
	end,
	help = function(self,msg,args)
		if not args[1] then self:addline("/help <command>, type /list for a list of commands") end
		if args[1] == "connect" then self:addline("(/connect [ip] [port]) -- connect to a TPT multiplayer server, or no args to connect to the default one")
		--elseif args[1] == "send" then self:addline("(/send <something> <somethingelse>) -- send raw data to the server") -- send a raw command
		elseif args[1] == "quit" or args[1] == "disconnect" then self:addline("(/quit, no arguments) -- quit the game")
		elseif args[1] == "join" then self:addline("(/join <channel> -- joins a room on the server")
		elseif args[1] == "sync" then self:addline("(/sync, no arguments) -- syncs your screen to everyone else in the room")
		elseif args[1] == "me" then self:addline("(/me <message>) -- say something in 3rd person") -- send a raw command
		elseif args[1] == "kick" then self:addline("(/kick <nick> <reason>) -- kick a user, only works if you have been in a channel the longest")
		elseif args[1] == "size" then self:addline("(/size <width> <height>) -- sets the size of the chat window")
		end
	end,
	list = function(self,msg,args)
		local list = ""
		for name in pairs(chatcommands) do
			list=list..name..", "
		end
		self:addline("Commands: "..list:sub(1,#list-2))
	end,
	me = function(self, msg, args)
		if not con.connected then return end
		self:addline("* " .. username .. " ".. table.concat(args, " "),200,200,200)
		conSend(20,table.concat(args, " "),true)
	end,
	kick = function(self, msg, args)
		if not con.connected then return end
		if not args[1] then self:addline("Need a nick! '/kick <nick> [reason]'") return end
		conSend(21, args[1].."\0"..table.concat(args, " ", 2),true)
	end,
	size = function(self, msg, args)
		if args[2] then
			local w, h = tonumber(args[1]), tonumber(args[2])
			if w < 75 or h < 50 then self:addline("size too small") return
			elseif w > sim.XRES-100 or h > sim.YRES-100 then self:addline("size too large") return
			end
			chatwindow = ui_chatbox.new(100,100,w,h)
			chatwindow:setbackground(10,10,10,235) chatwindow.drawbackground=true
			if using_manager then
				MANAGER.savesetting("tptmp", "width", w)
				MANAGER.savesetting("tptmp", "height", h)
			end
		end
	end
	}
	function chat:keypress(key, scan, rep, shift, ctrl, alt)
		if L.chatHidden then return nil end
		local text = self.inputbox:keypress(key, scan, rep, shift, ctrl, alt)
		if text and text~="" then
			local cmd = text:match("^/([^%s]+)")
			if cmd then
				local msg=text:sub(#cmd+3)
				local args = getArgs(msg)
				if chatcommands[cmd] then
					chatcommands[cmd](self,msg,args)
					--self:addline("Executed "..cmd.." "..rest)
					return
				end
			end
			--normal chat
			if con.connected then
				conSend(19,text,true)
				self:addline(username .. ": ".. text,200,200,200)
			else
				self:addline("Not connected to server!",255,50,50)
			end
		end
		if self.inputbox.focus then
			return true
		end
	end
	function chat:textinput(text)
		if L.chatHidden then return end
		self.inputbox:textinput(text)
	end
	return chat
end
}
local fadeText = {}
--A little text that fades away, (align text (left/center/right)?)
local function newFadeText(text,frames,x,y,r,g,b,noremove)
	local t = {ticks=frames,max=frames,text=text,x=x,y=y,r=r,g=g,b=b,keep=noremove}
	t.reset = function(self,text) self.ticks=self.max if text then self.text=text end end
	table.insert(fadeText,t)
	return t
end
--Some text locations for repeated usage
local infoText = newFadeText("",150,245,370,255,255,255,true)
local cmodeText = newFadeText("",120,250,180,255,255,255,true)

local function getypos()
	local ypos = 136
	if jacobsmod and tpt.oldmenu and tpt.oldmenu()==1 then
		ypos = 392
	elseif tpt.num_menus then
		ypos = 392-16*tpt.num_menus()-(not jacobsmod and 16 or 0)
	end
	if using_manager then ypos = ypos - 17 end
	return ypos
end
local jacobsmod_old_menu_check = false
local showbutton = ui_button.new(613,getypos(),14,14,function() if using_manager and not MANAGER.hidden then _print("minimize the manager before opening TPTMP") return end if not hooks_enabled then TPTMP.enableMultiplayer() end L.chatHidden=false TPTMP.chatHidden=false L.flashChat=false end,"Mp")
local flashCount=0
showbutton.drawbox = true showbutton:drawadd(function(self) if L.flashChat then self.almostselected=true flashCount=flashCount+1 if flashCount%25==0 then self.invert=not self.invert end end end)
if using_manager then
	local loadsettings = function() chatwindow = ui_chatbox.new(100, 100, tonumber(MANAGER.getsetting("tptmp", "width")), tonumber(MANAGER.getsetting("tptmp", "height"))) end
	if not pcall(loadsettings) then chatwindow = ui_chatbox.new(100, 100, 225, 150) end
else
	chatwindow = ui_chatbox.new(100, 100, 225, 150)
end
chatwindow:setbackground(10,10,10,235) chatwindow.drawbackground=true

local eleNameTable = {
["DEFAULT_PT_LIFE_GOL"] = 256,["DEFAULT_PT_LIFE_HLIF"] = 257,["DEFAULT_PT_LIFE_ASIM"] = 258,["DEFAULT_PT_LIFE_2x2"] = 259,["DEFAULT_PT_LIFE_DANI"] = 260,
["DEFAULT_PT_LIFE_AMOE"] = 261,["DEFAULT_PT_LIFE_MOVE"] = 262,["DEFAULT_PT_LIFE_PGOL"] = 263,["DEFAULT_PT_LIFE_DMOE"] = 264,["DEFAULT_PT_LIFE_34"] = 265,
["DEFAULT_PT_LIFE_LLIF"] = 276,["DEFAULT_PT_LIFE_STAN"] = 267,["DEFAULT_PT_LIFE_SEED"] = 268,["DEFAULT_PT_LIFE_MAZE"] = 269,["DEFAULT_PT_LIFE_COAG"] = 270,
["DEFAULT_PT_LIFE_WALL"] = 271,["DEFAULT_PT_LIFE_GNAR"] = 272,["DEFAULT_PT_LIFE_REPL"] = 273,["DEFAULT_PT_LIFE_MYST"] = 274,["DEFAULT_PT_LIFE_LOTE"] = 275,
["DEFAULT_PT_LIFE_FRG2"] = 276,["DEFAULT_PT_LIFE_STAR"] = 277,["DEFAULT_PT_LIFE_FROG"] = 278,["DEFAULT_PT_LIFE_BRAN"] = 279,
--walls
["DEFAULT_WL_ERASE"] = 280,["DEFAULT_WL_CNDTW"] = 281,["DEFAULT_WL_EWALL"] = 282,["DEFAULT_WL_DTECT"] = 283,["DEFAULT_WL_STRM"] = 284,
["DEFAULT_WL_FAN"] = 285,["DEFAULT_WL_LIQD"] = 286,["DEFAULT_WL_ABSRB"] = 287,["DEFAULT_WL_WALL"] = 288,["DEFAULT_WL_AIR"] = 289,["DEFAULT_WL_POWDR"] = 290,
["DEFAULT_WL_CNDTR"] = 291,["DEFAULT_WL_EHOLE"] = 292,["DEFAULT_WL_GAS"] = 293,["DEFAULT_WL_GRVTY"] = 294,["DEFAULT_WL_ENRGY"] = 295,
["DEFAULT_WL_NOAIR"] = 296,["DEFAULT_WL_ERASEA"] = 297,["DEFAULT_WL_STASIS"] = 298,
--special tools
["DEFAULT_UI_SAMPLE"] = 299,["DEFAULT_UI_SIGN"] = 300,["DEFAULT_UI_PROPERTY"] = 301,["DEFAULT_UI_WIND"] = 302,
--tools
["DEFAULT_TOOL_HEAT"] = 303,["DEFAULT_TOOL_COOL"] = 304,["DEFAULT_TOOL_AIR"] = 305,["DEFAULT_TOOL_VAC"] = 306,["DEFAULT_TOOL_PGRV"] = 307,["DEFAULT_TOOL_NGRV"] = 308, ["DEFAULT_TOOL_MIX"] = 309,
--decoration tools
["DEFAULT_DECOR_SET"] = 310,["DEFAULT_DECOR_CLR"] = 311,["DEFAULT_DECOR_ADD"] = 312,["DEFAULT_DECOR_SUB"] = 313,["DEFAULT_DECOR_MUL"] = 314,["DEFAULT_DECOR_DIV"] = 315,["DEFAULT_DECOR_SMDG"] = 316,
["DEFAULT_DECOR_LIGH"] = 317, ["DEFAULT_DECOR_DARK"] = 318
}
local gravList= {[0]="Vertical",[1]="Off",[2]="Radial"}
local airList= {[0]="On",[1]="Pressure Off",[2]="Velocity Off",[3]="Off",[4]="No Update"}
local noFlood = {[15]=true,[55]=true,[87]=true,[128]=true,[158]=true}
local noShape = {[55]=true,[87]=true,[128]=true,[158]=true}
local createOverride = {
	[55] = function(rx,ry,c) return 0,0,c end,
	[87] = function(rx,ry,c) local tmp=rx+ry if tmp>55 then tmp=55 end return 0,0,c+bit.lshift(tmp,8) end,
	[88] = function(rx,ry,c) local tmp=rx*4+ry*4+7 if tmp>300 then tmp=300 end return rx,ry,c+bit.lshift(tmp,8) end,
	[128] = function(rx,ry,c) return 0,0,c end,
	[158] = function(rx,ry,c) return 0,0,c end}
local golStart,golEnd=256,279
local wallStart,wallEnd=280,298
local toolStart,toolEnd=303,309
local decoStart,decoEnd=310,318

--Functions that do stuff in powdertoy
local function createPartsAny(x,y,rx,ry,c,brush,user)
	if c>=wallStart then
		if c<= wallEnd then
			if c == 284 then rx,ry = 0,0 end
			sim.createWalls(x,y,rx,ry,c-wallStart,brush)
		elseif c<=toolEnd then
			if c>=toolStart then sim.toolBrush(x,y,rx,ry,c-toolStart,brush) end
		elseif c<= decoEnd then
			sim.decoBrush(x,y,rx,ry,user.dcolour[2],user.dcolour[3],user.dcolour[4],user.dcolour[1],c-decoStart,brush)
		end
		return
	elseif c>=golStart then
		c = 78+(c-golStart)*bit.lshift(1, sim.PMAPBITS)
	end
	if createOverride[c] then
		rx,ry,c = createOverride[c](rx,ry,c)
	end
	sim.createParts(x,y,rx,ry,c,brush,user.replacemode)
end
local function createLineAny(x1,y1,x2,y2,rx,ry,c,brush,user)
	if noShape[c] then return end
	if jacobsmod and c == tpt.element("ball") and not user.shift then return end
	if c>=wallStart then
		if c<= wallEnd then
			if c == 284 then rx,ry = 0,0 end
			sim.createWallLine(x1,y1,x2,y2,rx,ry,c-wallStart,brush)
		elseif c<=toolEnd then
			if c>=toolStart then local str=1.0 if user.drawtype==4 then if user.shift then str=10.0 elseif user.alt then str=0.1 end end sim.toolLine(x1,y1,x2,y2,rx,ry,c-toolStart,brush,str) end
		elseif c<= decoEnd then
			sim.decoLine(x1,y1,x2,y2,rx,ry,user.dcolour[2],user.dcolour[3],user.dcolour[4],user.dcolour[1],c-decoStart,brush)
		end
		return
	elseif c>=golStart then
		c = 78+(c-golStart)*bit.lshift(1, sim.PMAPBITS)
	end
	if createOverride[c] then
		rx,ry,c = createOverride[c](rx,ry,c)
	end
	sim.createLine(x1,y1,x2,y2,rx,ry,c,brush,user.replacemode)
end
local function createBoxAny(x1,y1,x2,y2,c,user)
	if noShape[c] then return end
	if c>=wallStart then
		if c<= wallEnd then
			sim.createWallBox(x1,y1,x2,y2,c-wallStart)
		elseif c<=toolEnd then
			if c>=toolStart then sim.toolBox(x1,y1,x2,y2,c-toolStart) end
		elseif c<= decoEnd then
			sim.decoBox(x1,y1,x2,y2,user.dcolour[2],user.dcolour[3],user.dcolour[4],user.dcolour[1],c-decoStart)
		end
		return
	elseif c>=golStart then
		c = 78+(c-golStart)*bit.lshift(1, sim.PMAPBITS)
	end
	if createOverride[c] then
		_,_,c = createOverride[c](user.brushx,user.brushy,c)
	end
	sim.createBox(x1,y1,x2,y2,c,user and user.replacemode)
end
local function floodAny(x,y,c,cm,bm,user)
	if noFlood[c] then return end
	if c>=wallStart then
		if c<= wallEnd then
			sim.floodWalls(x,y,c-wallStart,bm)
		end
		--other tools shouldn't flood
		return
	elseif c>=golStart then --GoL adjust
		c = 78+(c-golStart)*bit.lshift(1, sim.PMAPBITS)
	end
	if createOverride[c] then
		_,_,c = createOverride[c](user.brushx,user.brushy,c)
	end
	sim.floodParts(x,y,c,cm,user.replacemode)
end
local function lineSnapCoords(x1,y1,x2,y2)
	local nx,ny
	local snapAngle = math.floor(math.atan2(y2-y1, x2-x1)/(math.pi*0.25)+0.5)*math.pi*0.25;
	local lineMag = math.sqrt(math.pow(x2-x1,2)+math.pow(y2-y1,2));
	nx = math.floor(lineMag*math.cos(snapAngle)+x1+0.5);
	ny = math.floor(lineMag*math.sin(snapAngle)+y1+0.5);
	return nx,ny
end

local function rectSnapCoords(x1,y1,x2,y2)
	local nx,ny
	local snapAngle = math.floor((math.atan2(y2-y1, x2-x1)+math.pi*0.25)/(math.pi*0.5)+0.5)*math.pi*0.5 - math.pi*0.25;
	local lineMag = math.sqrt(math.pow(x2-x1,2)+math.pow(y2-y1,2));
	nx = math.floor(lineMag*math.cos(snapAngle)+x1+0.5);
	ny = math.floor(lineMag*math.sin(snapAngle)+y1+0.5);
	return nx,ny
end
local function wallSnapCoords(x, y)
	return math.floor(x / 4) * 4, math.floor(y / 4) * 4
end
local renModes = {[0xff00f270]=1,[-16715152]=1,[0x0400f381]=2,[0xf382]=4,[0xf388]=8,[0xf384]=16,[0xfff380]=32,[1]=0xff00f270,[2]=0x0400f381,[4]=0xf382,[8]=0xf388,[16]=0xf384,[32]=0xfff380}
local function getViewModes()
	local t={0,0,0}
	for k,v in pairs(ren.displayModes()) do
		t[1] = t[1]+v
	end
	for k,v in pairs(ren.renderModes()) do
		t[2] = t[2]+(renModes[v] or 0)
	end
	t[3] = ren.colorMode()
	return t
end

--clicky click
local function playerMouseClick(id,btn,ev)
	local user = con.members[id]
	local createE, checkBut

	--_print(tostring(btn)..tostring(ev))
	if ev==0 then return end
	-- Mouse up event, TPT will "draw" whatever element was last clicked, even if we are releasing a different button
	-- This covers the case where we start drawing a line with rmb, switch to lmb, but then release rmb. The lmb element is drawn
	if ev == 2 then
		if user.lbtn then
			createE,checkBut=user.selectedl,user.lbtn
		elseif user.abtn then
			createE,checkBut=user.selecteda,user.abtn
		elseif user.rbtn then
			createE,checkBut=user.selectedr,user.rbtn
		else
			return
		end
	else
		if btn==1 then
			user.rbtn,user.abtn = false,false
			createE,checkBut=user.selectedl,user.lbtn
		elseif btn==2 then
			user.rbtn,user.lbtn = false,false
			createE,checkBut=user.selecteda,user.abtn
		elseif btn==3 then
			user.lbtn,user.abtn = false,false
			createE,checkBut=user.selectedr,user.rbtn
		else
			return
		end
	end

	--if user.mousex>=sim.XRES or user.mousey>=sim.YRES then user.drawtype=false return end

	if ev==1 then
		if user.mousex >= 0 and user.mousey >= 0 and user.mousex < sim.XRES and user.mousey < sim.YRES then
			user.pmx,user.pmy = user.mousex,user.mousey
			--left box
			if user.ctrl and not user.shift then user.drawtype = 2 return end
			--left line
			if user.shift and not user.ctrl then user.drawtype = 1 return end
			--floodfill
			if user.ctrl and user.shift then floodAny(user.mousex,user.mousey,createE,-1,-1,user) user.drawtype = 3 return end
			--an alt click
			if user.alt then return end
			user.drawtype=4 --normal hold
			createPartsAny(user.mousex,user.mousey,user.brushx,user.brushy,createE,user.brush,user)
		end
	elseif ev==2 and checkBut and user.drawtype then
		local releaseX, releaseY = user.mousex, user.mousey
		
		if user.drawtype==2 then
			if user.alt then user.mousex,user.mousey = rectSnapCoords(user.pmx,user.pmy,releaseX,releaseY) end
			createBoxAny(releaseX,releaseY,user.pmx,user.pmy,createE,user)
		elseif user.drawtype ~= 3 then
			if user.alt then user.mousex,user.mousey = lineSnapCoords(user.pmx,user.pmy,user.mousex,user.mousey) end
			createLineAny(releaseX,releaseY,user.pmx,user.pmy,user.brushx,user.brushy,createE,user.brush,user)
		end
		user.drawtype=false
		user.pmx,user.pmy = user.mousex,user.mousey
	end
end
--To draw continued lines
local function playerMouseMove(id)
	local user = con.members[id]
	local createE, checkBut
	if user.lbtn then
		createE,checkBut=user.selectedl,user.lbtn
	elseif user.rbtn then
		createE,checkBut=user.selectedr,user.rbtn
	elseif user.abtn then
		createE,checkBut=user.selecteda,user.abtn
	else return end
	if user.drawtype~=4 then if user.drawtype==3 then floodAny(user.mousex,user.mousey,createE,-1,-1,user) end return end
	
	if user.mousex>=sim.XRES then user.mousex=sim.XRES-1 end
	if user.mousey>=sim.YRES then user.mousey=sim.YRES-1 end
	createLineAny(user.mousex,user.mousey,user.pmx,user.pmy,user.brushx,user.brushy,createE,user.brush,user)
	user.pmx,user.pmy = user.mousex,user.mousey
end
local function loadStamp(size,x,y,reset)
	con.socket:settimeout(10)
	local s = con.socket:receive(size)
	con.socket:settimeout(0)
	if s then
		local f = io.open(".tmp.stm","wb")
		f:write(s)
		f:close()
		if reset then sim.clearSim() end
		if not sim.loadStamp(".tmp.stm",x,y) then
			infoText:reset("Error loading stamp")
		end
		os.remove".tmp.stm"
	else
		infoText:reset("Error loading empty stamp")
	end
end
local function saveStamp(x, y, w, h)
	local stampName = sim.saveStamp(x, y, w, h) or "errorsavingstamp"
	local fullName = "stamps/"..stampName..".stm"
	return stampName, fullName
end
local function deleteStamp(name)
	if sim.deleteStamp then
		sim.deleteStamp(name)
	else
		os.remove("stamps/"..name..".stm")
	end
end

local dataCmds = {
	[16] = function()
	--room members
		con.members = {}
		local amount = cByte()
		local peeps = {}
		for i=1,amount do
			local id = cByte()
			con.members[id]={name=conGetNull(),mousex=0,mousey=0,brushx=4,brushy=4,brush=0,selectedl=1,selectedr=0,selecteda=296,replacemode=0,dcolour={0,0,0,0},lbtn=false,abtn=false,rbtn=false,ctrl=false,shift=false,alt=false}
			local name = con.members[id].name
			table.insert(peeps,name)
		end
		chatwindow:addline("Online: "..table.concat(peeps," "),255,255,50)
	end,
	[17]= function()
		local id = cByte()
		con.members[id] ={name=conGetNull(),mousex=0,mousey=0,brushx=4,brushy=4,brush=0,selectedl=1,selectedr=0,selecteda=296,replacemode=0,dcolour={0,0,0,0},lbtn=false,abtn=false,rbtn=false,ctrl=false,shift=false,alt=false}
		chatwindow:addline(con.members[id].name.." has joined",100,255,100)
	end,
	[18] = function()
		local id = cByte()
		chatwindow:addline(con.members[id].name.." has left",255,255,100)
		con.members[id]=nil
	end,
	[19] = function()
		chatwindow:addline(con.members[cByte()].name .. ": " .. conGetNull())
	end,
	[20] = function()
		chatwindow:addline("* "..con.members[cByte()].name .. " " .. conGetNull())
	end,
	[22] = function()
		chatwindow:addline("[SERVER] "..conGetNull(), cByte(), cByte(), cByte())
	end,
	--Mouse Position
	[32] = function()
		local id = cByte()
		local b1,b2,b3=cByte(),cByte(),cByte()
		con.members[id].mousex,con.members[id].mousey=((b1*16)+math.floor(b2/16)),((b2%16)*256)+b3
		playerMouseMove(id)
	end,
	--Mouse Click
	[33] = function()
		local id = cByte()
		local d=cByte()
		local btn,ev=math.floor(d/16),d%16
		-- Fake mouseup due to either blur or zoom window
		if btn == 0 then
			local user = con.members[id]
			user.lbtn, user.rbtn, user.abtn, user.drawtype = nil, nil, nil, nil
			return
		end
		playerMouseClick(id,btn,ev)
		if ev==0 then return end
		if ev==2 then ev = nil end
		if btn==1 then
			con.members[id].lbtn=ev
		elseif btn==2 then
			con.members[id].abtn=ev
		elseif btn==3 then
			con.members[id].rbtn=ev
		end
	end,
	--Brush size
	[34] = function()
		local id = cByte()
		con.members[id].brushx,con.members[id].brushy=cByte(),cByte()
	end,
	--Brush Shape change, no args
	[35] = function()
		local id = cByte()
		con.members[id].brush=(con.members[id].brush+1)%3
	end,
	--Modifier (mod and state)
	[36] = function()
		local id = cByte()
		local d=cByte()
		local mod,state=math.floor(d/16),d%16~=0
		if mod==0 then
			con.members[id].ctrl=state
		elseif mod==1 then
			con.members[id].shift=state
		elseif mod==2 then
			con.members[id].alt=state
		end
	end,
	--selected elements (2 bits button, 14-element)
	[37] = function()
		local id = cByte()
		local b1,b2=cByte(),cByte()
		local btn,el=math.floor(b1/64),(b1%64)*256+b2
		if btn==0 then
			con.members[id].selectedl=el
		elseif btn==1 then
			con.members[id].selecteda=el
		elseif btn==2 then
			con.members[id].selectedr=el
		elseif btn==3 then
			--sync replace mode element between all players since apparently you have to set tpt.selectedreplace to use replace mode ...
			tpt.selectedreplace = elem.property(el, "Identifier")
		end
	end,
	--replace mode / specific delete
	[38] = function()
		local id = cByte()
		local mod = cByte()
		con.members[id].replacemode = mod
	end,
	--cmode defaults (1 byte mode)
	[48] = function()
		local id = cByte()
		tpt.display_mode(cByte())
		cmodeText:reset(con.members[id].name.." set:")
	end,
	--pause set (1 byte state)
	[49] = function()
		local id = cByte()
		local p,str = cByte(),"Pause"
		tpt.set_pause(p)
		if p==0 then str="Unpause" end
		infoText:reset(str.." from "..con.members[id].name)
	end,
	--step frame, no args
	[50] = function()
		local id = cByte()
		tpt.set_pause(0)
		L.pauseNextFrame=true
	end,
	--deco mode, (1 byte state)
	[51] = function()
		local id = cByte()
		tpt.decorations_enable(cByte())
		cmodeText:reset(con.members[id].name.." set:")
	end,
	--[[HUD mode, (1 byte state), deprecated
	[52] = function()
		local id = cByte()
		local hstate = cByte()
		tpt.hud(hstate)
	end,
	--]]
	--amb heat mode, (1 byte state)
	[53] = function()
		local id = cByte()
		tpt.ambient_heat(cByte())
	end,
	--newt_grav mode, (1 byte state)
	[54] = function()
		local id = cByte()
		tpt.newtonian_gravity(cByte())
	end,

	--[[
	--debug mode (1 byte state?) can't implement
	[55] = function()
		local id = cByte()
		--local dstate = cByte()
		tpt.setdebug()
	end,
	--]]
	--legacy heat mode, (1 byte state)
	[56] = function()
		local id = cByte()
		tpt.heat(cByte())
	end,
	--water equal, (1 byte state)
	[57] = function()
		local id = cByte()
		sim.waterEqualisation(cByte())
	end,

	--grav mode, (1 byte state)
	[58] = function()
		local id = cByte()
		local mode = cByte()
		sim.gravityMode(mode)
		cmodeText:reset(con.members[id].name.." set: Gravity: "..gravList[mode])
	end,
	--air mode, (1 byte state)
	[59] = function()
		local id = cByte()
		local mode=cByte()
		sim.airMode(mode)
		cmodeText:reset(con.members[id].name.." set: Air: "..airList[mode])
	end,

	--clear sparks (no args)
	[60] = function()
		local id = cByte()
		tpt.reset_spark()
	end,
	--clear pressure/vel (no args)
	[61] = function()
		local id = cByte()
		tpt.reset_velocity()
		tpt.set_pressure()
	end,
	--invert pressure (no args)
	[62] = function()
		local id = cByte()
		for x=0,152 do
			for y=0,95 do
				sim.pressure(x,y,-sim.pressure(x,y))
			end
		end
	end,
	--Clearsim button (no args)
	[63] = function()
		local id = cByte()
		sim.clearSim()
		L.lastSave=nil
		infoText:reset(con.members[id].name.." cleared the screen")
	end,
	--Full graphics view mode (for manual changes in display menu) (3 bytes)
	[64] = function()
		local id = cByte()
		local disM,renM,colM = cByte(),cByte(),cByte()
		ren.displayModes({disM})
		local t,i={},1
		while i<=32 do
			if bit.band(renM,i)>0 then table.insert(t,renModes[i]) end
			i=i*2
		end
		ren.renderModes(t)
		ren.colorMode(colM)
	end,
	--Selected deco colour (4 bytes)
	[65] = function()
		local id = cByte()
		con.members[id].dcolour = {cByte(),cByte(),cByte(),cByte()}
	end,
	--Recieve a stamp, with location (6 bytes location(3),size(3))
	[66] = function()
		local id = cByte()
		local b1,b2,b3=cByte(),cByte(),cByte()
		local x,y =((b1*16)+math.floor(b2/16)),((b2%16)*256)+b3
		local d = cByte()*65536+cByte()*256+cByte()
		loadStamp(d,x,y,false)
		infoText:reset("Stamp from "..con.members[id].name)
	end,
	--Clear an area, helper for cut (6 bytes, start(3), end(3))
	[67] = function()
		local id = cByte()
		local b1,b2,b3,b4,b5,b6=cByte(),cByte(),cByte(),cByte(),cByte(),cByte()
		local x1,y1 =((b1*16)+math.floor(b2/16)),((b2%16)*256)+b3
		local x2,y2 =((b4*16)+math.floor(b5/16)),((b5%16)*256)+b6
		sim.clearRect(x1,y1,x2-x1+1,y2-y1+1)
	end,
	--Edge mode (1 byte state)
	[68] = function()
		local id = cByte()
		sim.edgeMode(cByte())
	end,
	--Load a save ID (3 bytes ID)
	[69] = function()
		local id = cByte()
		local saveID = cByte()*65536+cByte()*256+cByte()
		L.lastSave=saveID
		sim.loadSave(saveID,1)
	end,
	--Reload sim(from a stamp right now, no args)
	[70] = function()
		local id = cByte()
		sim.reloadSave()
		infoText:reset(con.members[id].name.." reloaded the save")
	end,
	--A request to sync a player, from server, send screen, and various settings
	[128] = function()
		local id = cByte()
		conSend(130,string.char(id,49,tpt.set_pause()))
		local stampName,fullName = saveStamp(0,0,sim.XRES-1,sim.YRES-1)
		local f = assert(io.open(fullName,"rb"))
		local s = f:read"*a"
		f:close()
		deleteStamp(stampName)
		local d = #s
		conSend(128,string.char(id,math.floor(d/65536),math.floor(d/256)%256,d%256)..s)
		conSend(130,string.char(id,53,tpt.ambient_heat()))
		conSend(130,string.char(id,54,tpt.newtonian_gravity()))
		conSend(130,string.char(id,56,tpt.heat()))
		conSend(130,string.char(id,57,sim.waterEqualisation()))
		conSend(130,string.char(id,58,sim.gravityMode()))
		conSend(130,string.char(id,59,sim.airMode()))
		conSend(130,string.char(id,68,sim.edgeMode()))
		conSend(64,string.char(unpack(getViewModes())))
		conSend(34,string.char(tpt.brushx,tpt.brushy))
	end,
	--Recieve sync stamp
	[129] = function()
		local d = cByte()*65536+cByte()*256+cByte()
		loadStamp(d,0,0,true)
	end,
}

local function connectThink()
	if not con.connected then return end
	if not con.socket then disconnected() return end
	--read all messages
	while 1 do
		local s,r = con.socket:receive(1)
		if s then
			local cmd = string.byte(s)
			--_print("GOT "..tostring(cmd))
			if dataCmds[cmd] then dataCmds[cmd]() else _print("TPTMP: Unknown protocol "..tostring(cmd),255,20,20) end
		else
			if r ~= "timeout" then disconnected() end
			break
		end
	end

	--ping every minute
	if os.time()>con.pingTime then conSend(2) con.pingTime=os.time()+60 end
end
--Track if we have STKM2 out, for WASD key changes
elements.property(128,"Update",function() L.stick2=true end)

local function drawStuff()
	if con.members then
		for i,user in pairs(con.members) do
			local x,y = user.mousex,user.mousey
			local brx,bry=user.brushx,user.brushy
			local brush,drawBrush=user.brush,true
			gfx.drawText(x,y,("%s %dx%d"):format(user.name,brx,bry),0,255,0,192)

			-- Draw player cursors
			if user.drawtype==1 then
				if user.alt then x,y = lineSnapCoords(user.pmx,user.pmy,x,y) end
				gfx.drawLine(user.pmx,user.pmy,x,y,0,255,0,128)
			elseif user.drawtype==2 then
				if user.alt then x,y = rectSnapCoords(user.pmx,user.pmy,x,y) end
				local tpmx,tpmy = user.pmx,user.pmy
				if tpmx>x then tpmx,x=x,tpmx end
				if tpmy>y then tpmy,y=y,tpmy end
				gfx.drawRect(tpmx,tpmy,x-tpmx+1,y-tpmy+1,0,255,0,128)
				drawBrush=false
			elseif user.drawtype==3 or (user.shift and user.ctrl) then
				gfx.drawLine(x,y,x+5,y,0,255,0,128)
				gfx.drawLine(x,y,x-5,y,0,255,0,128)
				gfx.drawLine(x,y,x,y+5,0,255,0,128)
				gfx.drawLine(x,y,x,y-5,0,255,0,128)
				drawBrush=false
			end

			if drawBrush then
				if user.selectedl >= wallStart and user.selectedl <= wallEnd then
					local blockX, blockY = wallSnapCoords(x, y)
					local blockRadX, blockRadY = wallSnapCoords(brx, bry)
					
					local x1, y1 = blockX - blockRadX, blockY - blockRadY
					local x2, y2 = blockX + blockRadX + 3, blockY + blockRadY + 3
					gfx.drawRect(x1, y1, x2 - x1, y2 - y1)
				elseif brush==0 then
					gfx.drawCircle(x,y,brx,bry,0,255,0,128)
				elseif brush==1 then
					gfx.drawRect(x-brx,y-bry,brx*2+1,bry*2+1,0,255,0,128)
				elseif brush==2 then
					gfx.drawLine(x-brx,y+bry,x,y-bry,0,255,0,128)
					gfx.drawLine(x-brx,y+bry,x+brx,y+bry,0,255,0,128)
					gfx.drawLine(x,y-bry,x+brx,y+bry,0,255,0,128)
				end
			end
		end
	end
	for k,v in pairs(fadeText) do
		if v.ticks > 0 then
			local a = math.floor(255*(v.ticks/v.max))
			gfx.drawText(v.x,v.y,v.text,v.r,v.g,v.b,a)
			v.ticks = v.ticks-1
		else if not v.keep then table.remove(fadeText,k) end
		end
	end
end

local function sendStuff()
	if not con.connected then return end
	if tpt.brushx > 255 then tpt.brushx = 255 end
	if tpt.brushy > 255 then tpt.brushy = 255 end
	local nbx,nby = tpt.brushx,tpt.brushy
	if L.brushx~=nbx or L.brushy~=nby then
		L.brushx,L.brushy = nbx,nby
		conSend(34,string.char(L.brushx,L.brushy))
	end
	--check selected elements
	local nsell,nsela,nselr,nselrep = elements[tpt.selectedl] or eleNameTable[tpt.selectedl],elements[tpt.selecteda] or eleNameTable[tpt.selecteda],elements[tpt.selectedr] or eleNameTable[tpt.selectedr],elements[tpt.selectedreplace] or eleNameTable[tpt.selectedreplace]
	if L.sell~=nsell then
		L.sell=nsell
		if nsell == nil then
			_print("Unsupported wall/tool "..tpt.selectedl)
		else
			conSend(37,string.char(math.floor(L.sell/256),L.sell%256))
		end
	elseif L.sela~=nsela then
		L.sela=nsela
		if nsela == nil then
			_print("Unsupported wall/tool "..tpt.selecteda)
		else
			conSend(37,string.char(math.floor(64 + L.sela/256),L.sela%256))
		end
	elseif L.selr~=nselr then
		L.selr=nselr
		if nselr == nil then
			_print("Unsupported wall/tool "..tpt.selectedr)
		else
			conSend(37,string.char(math.floor(128 + L.selr/256),L.selr%256))
		end
	elseif L.selrep~=nselrep then
		L.selrep=nselrep
		if nselrep == nil then
			_print("Unsupported wall/tool "..tpt.selectedreplace)
		else
			conSend(37,string.char(math.floor(192 + L.selrep/256),L.selrep%256))
		end
	end
	local ncol = sim.decoColour()
	if L.dcolour~=ncol then
		L.dcolour=ncol
		conSend(65,string.char(math.floor(ncol/16777216),math.floor(ncol/65536)%256,math.floor(ncol/256)%256,ncol%256))
	end

	--Tell others to open this save ID, or send screen if opened local browser
	if L.browseMode==1 then
		--loaded online save
		local id=sim.getSaveID()
		if L.lastSave~=id then
			L.lastSave=id
			conSend(69,string.char(math.floor(id/65536),math.floor(id/256)%256,id%256))
		end
		L.browseMode=nil
	elseif L.browseMode==2 then
		--loaded local save (should probably clear sim first instead?)
		L.sendScreen=true
		L.browseMode=nil
	end

	--Send screen (or an area for known size) for stamps
	if jacobsmod and L.sendScreen == 2 then
		L.sendScreen = true
	elseif L.sendScreen then
		local x,y,w,h = 0,0,sim.XRES-1,sim.YRES-1
		if L.smoved then
			local stm
			if L.copying then stm=L.lastCopy else stm=L.lastStamp end
			if L.rotate then stm.w,stm.h=stm.h,stm.w end
			x,y,w,h = math.floor((L.mousex-stm.w/2)/4)*4,math.floor((L.mousey-stm.h/2)/4)*4,stm.w,stm.h
			L.smoved=false
			L.copying=false
		end
		L.sendScreen=false
		local stampName,fullName = saveStamp(x,y,w,h)
		local f = assert(io.open(fullName,"rb"))
		local s = f:read"*a"
		f:close()
		deleteStamp(stampName)
		local d = #s
		local b1,b2,b3 = math.floor(x/16),((x%16)*16)+math.floor(y/256),(y%256)
		conSend(67,string.char(math.floor(x/16),((x%16)*16)+math.floor(y/256),(y%256),math.floor((x+w)/16),(((x+w)%16)*16)+math.floor((y+h)/256),((y+h)%256)))
		conSend(66,string.char(b1,b2,b3,math.floor(d/65536),math.floor(d/256)%256,d%256)..s)
		conSend(49,string.char(tpt.set_pause()))
	end

	--Check if custom modes were changed
	if jacobsmod and L.checkRen == 2 then
		L.checkRen = true
	elseif L.checkRen then
		L.checkRen=false
		local t,send=getViewModes(),false
		for k,v in pairs(t) do
			if v~=L.pModes[k] then
				send=true break
			end
		end
		if send then conSend(64,string.char(t[1],t[2],t[3])) end
	end

	--Send option menu settings
	if L.checkOpt then
		L.checkOpt=false
		conSend(49,string.char(tpt.set_pause()))
		conSend(56,string.char(tpt.heat()))
		conSend(53,string.char(tpt.ambient_heat()))
		conSend(54,string.char(tpt.newtonian_gravity()))
		conSend(57,string.char(sim.waterEqualisation()))
		conSend(58,string.char(sim.gravityMode()))
		conSend(59,string.char(sim.airMode()))
		conSend(68,string.char(sim.edgeMode()))
	end

end
local function updatePlayers()
	if con.members then
		for k,v in pairs(con.members) do
			playerMouseMove(k)
		end
	end
	--Keep last frame of stick2
	L.lastStick2=L.stick2
	L.stick2=false
end

local pressedKeys
local function step()
	if jacobsmod_old_menu_check then showbutton:onmove(0, getypos()-showbutton.y) end
	if not L.chatHidden then chatwindow:draw() else showbutton:draw() end
	if hooks_enabled then
		drawStuff()
		sendStuff()
		if L.pauseNextFrame then L.pauseNextFrame=false tpt.set_pause(1) end
		connectThink()
		updatePlayers()
	end
end

--some button locations that emulate tpt, return false will disable button
local tpt_buttons = {
	["open"] = {x1=1, y1=408, x2=17, y2=422, f=function() if not L.ctrl then L.browseMode=1 else L.browseMode=2 end L.lastSave=sim.getSaveID() end},
	["rload"] = {x1=19, y1=408, x2=35, y2=422, f=function()
		if L.lastSave then
			if L.ctrl then
				infoText:reset("If you re-opened the save, please type /sync")
			else
				conSend(70)
				infoText:reset("Sent sync")
			end
		else
			--[[infoText:reset("Reloading local saves is not synced currently. Type /sync")]]
			L.checkOpt = true
			L.sendScreen = true
		end
	end},
	["clear"] = {x1=470, y1=408, x2=486, y2=422, f=function() conSend(63) L.lastSave=nil end},
	["opts"] = {x1=581, y1=408, x2=595, y2=422, f=function() L.checkOpt=true end},
	["disp"] = {x1=597, y1=408, x2=611, y2=422, f=function() L.checkRen=true L.pModes=getViewModes() end},
	["pause"] = {x1=gfx.WIDTH-16, y1=408, x2=gfx.WIDTH-2, y2=422, f=function() conSend(49,tpt.set_pause()==0 and "\1" or "\0") end},
	["deco"] = {x1=gfx.WIDTH-16, y1=33, x2=gfx.WIDTH-2, y2=47, f=function() if jacobsmod and (L.tabs or L.ctrl) then return end conSend(51,tpt.decorations_enable()==0 and "\1" or "\0") end},
	["newt"] = {x1=gfx.WIDTH-16, y1=49, x2=gfx.WIDTH-2, y2=63, f=function() if jacobsmod and (L.tabs or L.ctrl) then return end conSend(54,tpt.newtonian_gravity()==0 and "\1" or "\0") end},
	["ambh"] = {x1=gfx.WIDTH-16, y1=65, x2=gfx.WIDTH-2, y2=79, f=function() if jacobsmod and (L.tabs or L.ctrl) then return end conSend(53,tpt.ambient_heat()==0 and "\1" or "\0") end},
}
if jacobsmod then
	tpt_buttons["tab"] = {x1=gfx.WIDTH-16, y1=1, x2=gfx.WIDTH-2, y2=15, f=function() L.tabs = not L.tabs end}
	tpt_buttons["tabs"] = {x1=gfx.WIDTH-16, y1=17, x2=gfx.WIDTH-2, y2=147, f=function() if L.tabs or L.ctrl then L.sendScreen = true end end}
	tpt_buttons["opts"] = {x1=465, y1=408, x2=479, y2=422, f=function() L.checkOpt=true end}
	tpt_buttons["clear"] = {x1=481, y1=408, x2=497, y2=422, f=function() conSend(63) L.lastSave=nil end}
	tpt_buttons["disp"] = {x1=595, y1=408, x2=611, y2=422,f=function() L.checkRen=2 L.pModes=getViewModes() end}
	tpt_buttons["open"] = {x1=1, y1=408, x2=17, y2=422, f=function() if not L.ctrl then L.browseMode=1 else L.browseMode=2 end L.lastSave=sim.getSaveID() end}
end

local function inZoomWindow(x, y)
	if not L.isDrawing and (x < 0 or x >= sim.XRES or y < 0 or y >= sim.YRES) then
		return false
	end
	local snappedX, snappedY = x, y
	-- When the mouse is outside the window, TPT will snap coords to simulation area then check for zoom window
	if snappedX < 0 then snappedX = 0 elseif snappedX >= sim.XRES then snappedX = sim.XRES end
	if snappedY < 0 then snappedY = 0 elseif snappedY >= sim.YRES then snappedY = sim.YRES end
	local zoomX, zoomY = sim.adjustCoords(snappedX, snappedY)
	return zoomX ~= x or zoomY ~= y
end

local function sendMouseUpdate(mouseX, mouseY)
	L.realMouseX, L.realMouseY = mouseX, mouseY
	if inZoomWindow(mouseX, mouseY) then
		mouseX, mouseY = sim.adjustCoords(mouseX, mouseY)	
	else
		if mouseX < 0 then mouseX = 0 end
		if mouseY < 0 then mouseY = 0 end
		if mouseX > gfx.WIDTH then mouseX = gfx.WIDTH end
		if mouseY > gfx.HEIGHT then mouseY = gfx.HEIGHT end
	end

	if L.mousex ~= mouseX or L.mousey ~= mouseY then
		local b1, b2, b3 = math.floor(mouseX / 16), ((mouseX % 16) * 16) + math.floor(mouseY / 256), (mouseY % 256)
		conSend(32, string.char(b1, b2, b3))
		L.mousex, L.mousey = mouseX, mouseY
	end
end

local function mouseDown(mouseX, mouseY, button)
	if L.chatHidden then
		showbutton:mouseDown(mouseX, mouseY, button)
		if not hooks_enabled then
			return true
		end
	end
	if L.stamp and button == 1 then
		L.stampx, L.stampy = mouseX, mouseY
		return true
	end
	if L.stamp or L.placeStamp then
		return true
	end
	if L.skipClick then
		L.skipClick = false
		return true
	end
	if chatwindow:mouseDown(mouseX, mouseY, button) then
		return false
	end

	sendMouseUpdate(mouseX, mouseY)
	local obut, oevnt = L.mButt, L.mEvent
	if button ~= obut or 1 ~= oevnt then
		L.mButt, L.mEvent = button, 1
		if mouseX >= 0 and mouseY >= 0 and mouseX < sim.XRES and mouseY < sim.YRES then
			L.isDrawing = true
		end
		conSend(33, string.char(L.mButt * 16 + L.mEvent))
	end
	
	-- Click inside button first
	if button==1 then
		for k, v in pairs(tpt_buttons) do
			if mouseX >= v.x1 and mouseX <= v.x2 and mouseY >= v.y1 and mouseY <= v.y2 then
				v.downInside = true
			end
		end
	end
end

local function mouseUp(mouseX, mouseY, button, reason)
	if L.chatHidden then
		showbutton:mouseUp(mouseX, mouseY, button)
		if not hooks_enabled then
			return true
		end
	end
	if L.stamp then
		if L.skipClick then
			L.skipClick = false
			return true
		end
		--stamp has been saved, make our own copy
		if button==1 then
			--save stamp ourself for data, delete it
			local sx,sy = mouseX, mouseY
			if sx<L.stampx then L.stampx,sx=sx,L.stampx end
			if sy<L.stampy then L.stampy,sy=sy,L.stampy end
			--cheap cut hook to send a clear
			if L.copying==1 then
				--maybe this is ctrl+x? 67 is clear area
				conSend(67,string.char(math.floor(L.stampx/16),((L.stampx%16)*16)+math.floor(L.stampy/256),(L.stampy%256),math.floor(sx/16),((sx%16)*16)+math.floor(sy/256),(sy%256)))
			end
			local w,h = sx-L.stampx,sy-L.stampy
			local stampName,fullName = saveStamp(L.stampx,L.stampy,w,h)
			sx,sy,L.stampx,L.stampy = math.ceil((sx+1)/4)*4,math.ceil((sy+1)/4)*4,math.floor(L.stampx/4)*4,math.floor(L.stampy/4)*4
			w,h = sx-L.stampx, sy-L.stampy
			local f = assert(io.open(fullName,"rb"))
			if L.copying then L.lastCopy = {data=f:read"*a",w=w,h=h} else L.lastStamp = {data=f:read"*a",w=w,h=h} end
			f:close()
			deleteStamp(stampName)
		end
		L.stamp=false
		L.copying=false
	end
	if L.placeStamp and reason == 0 then
		if L.skipClick then
			L.skipClick=false
			return true
		end
		if button==1 then
			local stm
			if L.copying then stm=L.lastCopy else stm=L.lastStamp end
			if stm then
				if not stm.data then
					--unknown stamp, send full screen on next step, how can we read last created stamp, timestamps on files?
					L.sendScreen = (jacobsmod and 2 or true)
				else
					--send the stamp
					if L.smoved then
						--moved from arrows or rotate, send area next frame
						L.placeStamp=false
						L.sendScreen=true
						return true
					end
					local sx,sy = mouseX-math.floor(stm.w/2),mouseY-math.floor((stm.h)/2)
					if sx<0 then sx=0 end
					if sy<0 then sy=0 end
					if sx+stm.w>sim.XRES-1 then sx=sim.XRES-stm.w end
					if sy+stm.h>sim.YRES-1 then sy=sim.YRES-stm.h end
					local b1,b2,b3 = math.floor(sx/16),((sx%16)*16)+math.floor(sy/256),(sy%256)
					local d = #stm.data
					conSend(66,string.char(b1,b2,b3,math.floor(d/65536),math.floor(d/256)%256,d%256)..stm.data)
				end
			end
		end
		L.placeStamp=false
		L.copying=false
	end
	
	if L.skipClick then
		L.skipClick = false
		return true
	end
	if chatwindow:mouseUp(mouseX, mouseY, button, reason) then
		return false
	end

	-- Ignore fake mouseups due to blur (don't send 0, 0 coordinate)
	-- Also ignore fake mouseups due to going into / outside of zoom window
	-- In both cases, tell other clients we're no longer holding the mouse, without causing it to draw the line
	if reason == 1 or reason == 2 then
		L.mButt, L.mEvent = button, 2
		L.isDrawing = false
		conSend(33, string.char(0 * 16 + L.mEvent))
		return
	end

	sendMouseUpdate(mouseX, mouseY)
	local obut, oevnt = L.mButt, L.mEvent
	if button ~= obut or 2 ~= oevnt then
		L.isDrawing = false
		L.mButt, L.mEvent = button, 2
		conSend(33, string.char(L.mButt * 16 + L.mEvent))
	end

	-- Up inside the button we started with
	if button == 1 then
		local ret = true
		for k,v in pairs(tpt_buttons) do
			if v.downInside and (mouseX>=v.x1 and mouseX<=v.x2 and mouseY>=v.y1 and mouseY<=v.y2) then
				if v.f() == false then ret = false end
			end
			v.downInside = nil
		end
		return ret
	end
end

local function mouseMove(mouseX, mouseY, dX, dY)
	if L.chatHidden then
		showbutton:mouseMove(mouseX, mouseY, dX, dY)
		if not hooks_enabled then
			return true
		end
	end
	if chatwindow:mouseMove(mouseX, mouseY, dX, dY) then
		return false
	end

	-- Always send mouse update, unless we're currently drawing and entered/exited the zoom window
	-- TPT sends a fake mouseup event in this case, but the mouse move happens first so we need to stop it ourselves
	local shouldBlockMove = L.isDrawing and not inZoomWindow(mouseX, mouseY) ~= inZoomWindow(L.realMouseX, L.realMouseY)
	if shouldBlockMove then
		L.isDrawing = false
	else
		sendMouseUpdate(mouseX, mouseY)
	end

	--Mouse hold, we MUST stay inside button or don't trigger on up
	if button == 1 then
		for k, v in pairs(tpt_buttons) do
			if v.downInside and (mouseX < v.x1 or mouseX > v.x2 or mouseY < v.y1 or mouseY > v.y2) then
				v.downInside = nil
			end
		end
	end
end

local function mouseWheel(mouseX, mouseY, wheel)
	if chatwindow.inputbox.focus and chatwindow:mouseWheel(mousex, mousey, wheel) then
		return false
	end
end

local keyFuncs = {
	--TAB
	[9] = function() if not jacobsmod or not L.ctrl then conSend(35) end end,

	--View modes 0-9
	[48] = function() conSend(48,"\10") end,
	[49] = function() if L.shift then conSend(48,"\9") tpt.display_mode(9)--[[force local display mode, screw debug check for now]] return false end conSend(48,"\0") end,
	[50] = function() conSend(48,"\1") end,
	[51] = function() conSend(48,"\2") end,
	[52] = function() conSend(48,"\3") end,
	[53] = function() conSend(48,"\4") end,
	[54] = function() conSend(48,"\5") end,
	[55] = function() conSend(48,"\6") end,
	[56] = function() conSend(48,"\7") end,
	[57] = function() conSend(48,"\8") end,

	-- ins / del for replace mode
	[1073741897] = function() L.replacemode = bit.bxor(L.replacemode, 1) conSend(38, L.replacemode) end,
	[127] = function() L.replacemode = bit.bxor(L.replacemode, 2) conSend(38, L.replacemode) end,

	--Arrows for stamp adjust
	[273] = function() if L.placeStamp then L.smoved=true end end,
	[274] = function() if L.placeStamp then L.smoved=true end end,
	[275] = function() if L.placeStamp then L.smoved=true end end,
	[276] = function() if L.placeStamp then L.smoved=true end end
}

local scanFuncs = {
	--`, console
	[53] = function() if not L.shift and con.connected then infoText:reset("Console does not sync, use shift+` to open instead") return false else jacobsmod_old_menu_check = true end end,

	--b, deco, pauses sim
	[5] = function() if L.ctrl then conSend(51,tpt.decorations_enable()==0 and "\1" or "\0") else conSend(49,"\1") conSend(51,"\1") end end,

	--c, copy
	[6] = function() if L.ctrl then L.stamp=true L.copying=true L.stampx = -1 L.stampy = -1 end end,

	--d key, debug, api broken right now
	--[7] = function() conSend(55) end,

	--F, frame step
	[9] = function() if not jacobsmod or not L.ctrl then conSend(50) end end,

	--H, HUD and intro text
	[11] = function() if L.ctrl and jacobsmod then return false end end,

	--I, invert pressure
	[12] = function() conSend(62) end,

	--K, stamp menu, abort our known stamp, who knows what they picked, send full screen?
	[14] = function() L.lastStamp={data=nil,w=0,h=0} L.placeStamp=true end,

	--L, last Stamp
	[15] = function() if L.lastStamp then L.placeStamp=true end end,

	--N, newtonian gravity or new save
	[17] = function() if jacobsmod and L.ctrl then L.sendScreen=2 L.lastSave=nil else conSend(54,tpt.newtonian_gravity()==0 and "\1" or "\0") end end,

	--O, old menu in jacobs mod
	[18] = function() if jacobsmod and not L.ctrl then jacobsmod_old_menu_check = true end end,

	--R, for stamp rotate or save reload
	[21] = function()
		if L.placeStamp then
			L.smoved = true
			if L.shift then
				return
			end
			L.rotate = not L.rotate
		elseif L.ctrl then
			if L.lastSave then
				conSend(70)
				infoText:reset("Sent reload command") 
			else
				L.sendScreen = true
				infoText:reset("Send sync") 
			end
		end
	end,

	--S, stamp
	[22] = function() if (L.lastStick2 and not L.ctrl) or (jacobsmod and L.ctrl) then return end L.stamp=true L.stampx = -1 L.stampy = -1 end,

	--T, tabs
	[23] = function() if jacobsmod then L.tabs = not L.tabs end end,

	--U, ambient heat toggle
	[24] = function() conSend(53,tpt.ambient_heat()==0 and "\1" or "\0") end,

	--V, paste the copystamp
	[25] = function() if L.ctrl and L.lastCopy then L.placeStamp=true L.copying=true end end,

	--W, gravity mode
	[26] = function() if L.lastStick2 and not L.ctrl then return end conSend(58,string.char((sim.gravityMode()+1)%3)) return true end,

	--X, cut a copystamp and clear
	[27] = function() if L.ctrl then L.stamp=true L.copying=1 L.stampx = -1 L.stampy = -1 end end,

	--Y, air mode
	[28] = function() if L.ctrl then L.sendScreen = true else conSend(59,string.char((sim.airMode()+1)%5)) return true end end,

	--Z
	[29] = function() if L.ctrl then L.sendScreen = true else L.skipClick=true end end,

	--ESC
	[41] = function() if not L.chatHidden then L.chatHidden = true TPTMP.chatHidden = true return false end end,

	--space, pause toggle
	[44] = function() conSend(49,tpt.set_pause()==0 and "\1" or "\0") end,

	--= key, pressure/spark reset
	[46] = function() if L.ctrl then conSend(60) else conSend(61) end end,

	--;, replace mode or specific delete
	[59] = function() if L.ctrl then  L.replacemode = bit.bxor(L.replacemode, 2) else  L.replacemode = bit.bxor(L.replacemode, 1) end conSend(38, L.replacemode) end,

	--F1 , intro text
	[58] = function() if jacobsmod then return false end end,

	--F5 , save reload
	[62] = function()
		if L.lastSave then
			conSend(70)
			infoText:reset("Sent reload command") 
		else
			L.sendScreen = true
			infoText:reset("Sent sync") 
		end
	end,
}
local scanUnpressFuncs = {
	--Z
	[29] = function() L.skipClick=false if L.alt then L.skipClick=true end end,
}

local function keypress(key, scan, rep, shift, ctrl, alt)
	if shift and not L.shift then
		L.shift = true
		conSend(36, string.char(17))
	end
	if ctrl and not L.ctrl then
		L.ctrl = true
		conSend(36, string.char(1))
	end
	if alt and not L.alt then
		L.alt = true
		conSend(36, string.char(33))
	end

	local check = chatwindow:keypress(key, scan, rep, shift, ctrl, alt)
	if type(check) == "boolean" then
		return not check
	end
	
	if rep then return end

	if scanFuncs[scan] then
		ret = scanFuncs[scan]()
		if ret ~= nil then
			return ret
		end
	elseif keyFuncs[key] then
		ret = keyFuncs[key]()
		if ret ~= nil then
			return ret
		end
	end
end
local function keyrelease(key, scan, rep, shift, ctrl, alt)
	if not shift and L.shift then
		L.shift = false
		conSend(36, string.char(16))
	end
	if not ctrl and L.ctrl then
		L.ctrl = false
		conSend(36, string.char(0))
	end
	if not alt and L.alt then
		L.alt = false
		conSend(36, string.char(32))
	end

	if rep then return end

	if scanUnpressFuncs[scan] then
		ret = scanUnpressFuncs[scan]()
		if ret~= nil then
			return ret
		end
	end
end
local function textinput(text)
	chatwindow:textinput(text)
end
local function blur()
	if L.shift then
		L.shift = false
		conSend(36, string.char(16))
	end
	if L.ctrl then
		L.ctrl = false
		conSend(36, string.char(0))
	end
	if L.alt then
		L.alt = false
		conSend(36, string.char(32))
	end
end
function TPTMP.disableMultiplayer()
	evt.unregister(evt.tick, step)
	evt.unregister(evt.mousedown, mouseDown)
	evt.unregister(evt.mouseup, mouseUp)
	evt.unregister(evt.mousemove, mouseMove)
	evt.unregister(evt.mousewheel, mouseQheel)
	evt.unregister(evt.keypress, keypress)
	evt.unregister(evt.keyrelease, keyrelease)
	evt.unregister(evt.textinput, textinput)
	evt.unregister(evt.blur, blur)
	TPTMP = nil
	disconnected("TPTMP unloaded")
end

function TPTMP.enableMultiplayer()
	hooks_enabled = true
	TPTMP.enableMultiplayer = nil
	debug.sethook(nil,"",0)
	if jacobsmod then
		--clear intro text tooltip
		gfx.toolTip("", 0, 0, 0, 4)
	end
end
TPTMP.con = con
TPTMP.chatHidden = true
evt.register(evt.tick, step)
evt.register(evt.mousedown, mouseDown)
evt.register(evt.mouseup, mouseUp)
evt.register(evt.mousemove, mouseMove)
evt.register(evt.mousewheel, mouseWheel)
evt.register(evt.keypress, keypress)
evt.register(evt.keyrelease, keyrelease)
evt.register(evt.textinput, textinput)
evt.register(evt.blur, blur)