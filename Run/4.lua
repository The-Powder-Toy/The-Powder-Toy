tpt.set_console(0)

function noAirGrav()
	tpt.reset_velocity(0, 0, 612, 384)
	tpt.reset_gravity_field(0, 0, 612, 384)
	tpt.set_pressure(0, 0, 612, 384, 0)
	return false
end
tpt.register_step(noAirGrav)