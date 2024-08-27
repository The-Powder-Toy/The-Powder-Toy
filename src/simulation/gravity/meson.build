if host_platform == 'emscripten'
	# FFTW_MEASURE fails on emscripten for some reason, probably a timing issue
	# FFTW_ESTIMATE is 20% slower
	conf_data.set('FFTW_PLAN_MEASURE', 'false')
else
	conf_data.set('FFTW_PLAN_MEASURE', 'true')
endif
powder_files += files('Fft.cpp')
render_files += files('Null.cpp')
