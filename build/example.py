import tpt
import random
#                    example script

def noise(typ,amount=50):
    for xx in range(284,316):
        for yy in range(184,216):
            if(random.randrange(0,100)<amount):
                tpt.create(xx,yy,typ)
            

