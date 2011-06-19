tpt.set_console(0)

function noSprk()
	tpt.reset_spark()
	return false
end
tpt.register_step(noSprk)