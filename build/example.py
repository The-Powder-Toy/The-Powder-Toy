import utils
#                    example script

def noise(typ,amount=50):
    for xx in range(4,612):
        for yy in range(4,384):
            if(random.randrange(0,100)<amount):
                tpt.create(xx,yy,typ)

def increment(amount=1):
    for xx in range(4,612):
        for yy in range(4,384):
            i = tpt.get_pmap(xx,yy)>>8
            t = tpt.get_prop(i,"type")+amount
            if t>=148:
                t=t-148
            if t==128:
                t=129
            if t==55:
                t=56
            tpt.set_type(i=i,settoint=t)