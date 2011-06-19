-- This line is a comment. Anything written after the -- is considered a Comment and will not be read by Lua.

-- Set the console's state to 0. This will hide the console.
tpt.set_console(0)

-- Here we define our main function for the script
function ClassicPowder()
	local ox = 75 -- This will be our offset for the different elements we will create.125 = default
	local y = 15 -- where on the y (vertical) axis where we will create our elements.
	
	local x = ox -- where on the x (horizontal) axis where we will create our elements. we will start the x value with what ever ox is above.
	for i=0, 10 do -- this is a for loop. everything between the do and end will loop until i hits 10. i increases by 1 every loop.
		tpt.create(x + i, y, "DUST") --create a dust particle
	end
	
	x = x + ox -- increase the x axis value by the offset x (ox) value above.
	for i=0, 10 do
		tpt.create(x + i, y, "WATR") --create a water particle
	end
	
	x = x + ox
	for i=0, 10 do
		tpt.create(x + i, y, "SALT")
	end
	
	x = x + ox
	for i=0, 10 do
		tpt.create(x + i, y, "OIL")
	end
        x = x + ox
	for i=0, 10 do
		tpt.create(x + i, y, "NITR")
	end
        x = x + ox
	for i=0, 10 do
		tpt.create(x + i, y, "SOAP")
	end
        x = x + ox
	for i=0, 10 do
		tpt.create(x + i, y, "DEUT")
	end
	tpt.reset_velocity(0, 0, 612, 384)
	tpt.reset_gravity_field(0, 0, 612, 384)
	return false
end
function ClassicText()
	local ox = 75 -- This will be our offset for the different elements we will create.125 = default
	local ytext = 5
	local x = ox -- where on the x (horizontal) axis where we will create our elements. we will start the x value with what ever ox is above.
	tpt.drawtext(x, ytext, "DUST")
	x = x + ox -- increase the x axis value by the offset x (ox) value above.
	tpt.drawtext(x, ytext, "WATR")
	x = x + ox
	tpt.drawtext(x, ytext, "SALT")
	x = x + ox
	tpt.drawtext(x, ytext, "OIL")
        x = x + ox
	tpt.drawtext(x, ytext, "NITR")
        x = x + ox
	tpt.drawtext(x, ytext, "SOAP")
        x = x + ox
	tpt.drawtext(x, ytext, "DEUT")
	
	return false
end
-- Register the step function ClassicPowder. This will make the ClassicPowder function run every tick of Powder Toy.
tpt.register_step(ClassicPowder)
tpt.register_step(ClassicText)