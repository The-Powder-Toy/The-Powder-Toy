import tpt
import random
import time
#                    example script

def noise(typ,amount=50):
    for xx in range(4,612):
        for yy in range(4,384):
            if(random.randrange(0,100)<amount):
                tpt.create(xx,yy,typ)
