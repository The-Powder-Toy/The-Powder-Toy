### CONSOLE ###
the console in this version is a python console and will execute any command you enter.
the api for interacting with the powder toy is contained in the tpt module and is already imported at startup.
currently implemented api functions:
    create(x,y,type)         create a particle of type <type> at <x>,<y>
    reset_velocity()         resets all velocity
    reset_pressure()         resets all pressure
    reset_sparks()           resets all sparks

### USING SET COMMANDS ###
the console uses keywords to identify variables of what to set, you can specify x and y coords with x=100 y=100, ect.
i is the particle number
name will change all of name's type to the setto variable. It needs to be a string "dust" ect, "all" for all.
the setto is what the particle(s) will be set to, it is a string for the type/ctype commands, and an int for the rest.
you need to set the to* variable and one location/name/coords for it to work.
    set_type(x=,y=,i=,name="",setto="")    
    set_life()    
    set_temp()      
    set_tmp()        
    set_x()            
    set_y()            
    set_vx()          
    set_vy()          
    set_ctype()    

    pause()                  pause the game(note that closing the console
                             pauses the game)
    unpause()                unpause the game(note that closing the console
                             pauses the game)
    toggle_pause()           toggle pause(note that closing the console
                             pauses the game)
    close_console()          closes the console and pauses the game
    open_console()           opens the console
    toggle_console()         toggles the console

NOTE: these functions don't do bounds checking, which means that they CAN AND
      WILL CRASH the powder toy. be carefull.

### TIPS&TRICKS ###
all functions that need a particle type expect a number. but instead of doing
tpt.set_life(32,life) you can do tpt.set_life(element['uran'],life)

### COMPILING ###
before you compile you should have python 2.x installed.
you must run the "getheader.py" script to get the correct linking options, add these options to the makefile
