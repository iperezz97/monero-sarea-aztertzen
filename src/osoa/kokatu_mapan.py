import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
from itertools import chain
import socket
import struct


def draw_map(m, scale=0.1):
    # draw a shaded-relief image
    m.shadedrelief(scale=scale)

    # lats and longs are returned as a dictionary
    lats = m.drawparallels(np.linspace(-90, 90, 13))
    lons = m.drawmeridians(np.linspace(-180, 180, 13))

    # keys contain the plt.Line2D instances
    lat_lines = chain(*(tup[1][0] for tup in lats.items()))
    lon_lines = chain(*(tup[1][0] for tup in lons.items()))
    all_lines = chain(lat_lines, lon_lines)

    # cycle through these lines and set the desired style
    for line in all_lines:
        line.set(linestyle='-', alpha=0.2, color='w')
#        line.set(linestyle=':', alpha=0.2, color='w')


def remove_point(x, y):
    for rp in points:
#        print(rp.get_data()[0], rp.get_data()[1])
        if(rp.get_data()[0] == x and rp.get_data()[1] == y):
            try:
                rp.set_data([0],[0])
                rp.remove()
            except:
                print("E5\tErrorea puntua ezabatzerakoan.")
                pass
            break


def draw_point(lonf, latf):
    if(lonf > 180.0 or lonf < -180.0 or latf > 90.0 or latf < -90.0):
        print("E4\tLongitudeak -180.0 eta 180.0 arteko balioak onartzen ditu.\n \tLatitudeak -90.0 eta 90.0 arteko balioak onartzen ditu.")
    else:
        #lox, lay = m(lonf, latf)
        points.extend( # gorde puntuak (Line2D objektuak)
            m.plot(lonf, latf, markerfacecolor='red', markeredgecolor='black', marker='o', markersize=2.7, alpha=0.5)
        )
        fig.canvas.draw()
        fig.canvas.flush_events()

# IP irakurri eta hover ekintza gertatzean adierazi (informazio gehigarri bezala)
def ip2int(addr):
    return struct.unpack("!I", socket.inet_aton(addr))[0]


def int2ip(addr):
    return socket.inet_ntoa(struct.pack("!I", addr))
#


### main exec ###


# ireki pipe-a
#path = "./fifoKanala"
#mode = 0o600
#os.mkfifo(path, mode) # sortuta C-n...
#fd = os.open(path, os.O_RDONLY) # ireki (deskriptorea)

plt.ion()
fig = plt.figure(figsize=(11, 6), edgecolor='w')

ax = fig.add_subplot(111)
ax.set_title("Monero nodoak munduan zehar")

m = Basemap(projection='cyl', resolution=None,
            llcrnrlat=-90, urcrnrlat=90,
            llcrnrlon=-180, urcrnrlon=180, ax=ax)

draw_map(m)

points = [] # mapako puntuak gorde
iter = 0    # zenbat gehitu diren kontatu
remo = 0
print("Koordenatuak irakurtzen...")

# Koordenatuak sarrera estandarretik irakurri begizta infinitu batean (lerroz lerro irakurriz bi zenbakiak, 'r <long> <lat>' puntua ezabatzeko eta 'q' exekuzioa bukatzeko)
while True:
    try:
        err = 0
        #args = input("")
        args = input("<Eragiketa> <Longitude> <Latitude>")
        erag,valo,vala = args.split()
        if(erag == 'q' ): # exekuzioa bukatu
            break
        lonf = float(valo)
        latf = float(vala)
        if(lonf > 180.0 or lonf < -180.0 or latf > 90.0 or latf < -90.0):
            err = 1
            print("E1\tLongitudeak -180.0 eta 180.0 arteko balioak onartzen ditu.\n \tLatitudeak -90.0 eta 90.0 arteko balioak onartzen ditu. ")
            continue

        elif(erag == 'a'): # puntua mapan gehitu
            draw_point(lonf, latf)
            iter = iter + 1 # kontatu

        elif(erag == 'r'): # puntua mapatik ezabatu
            remove_point(lonf, latf)
            remo = remo + 1 # kontatu

    except ValueError:
        if(args == 'q'): # exekuzioa bukatzeko 'q'
            break
        else:
            print("E2 Ez da formatu egokia erabili: \n >>> 'a <long> <lat>' \t // puntua mapan gehitzeko \n >>> 'r <long> <lat>' \t // puntua mapatik ezabatzeko \n >>> 'q' \t\t // exekuzioa bukatzeko")
            err = 2
        pass
    except:
        print("E3 Ez da formatu egokia erabili: \n >>> 'a <long> <lat>' \t // puntua mapan gehitzeko \n >>> 'r <long> <lat>' \t // puntua mapatik ezabatzeko \n >>> 'q' \t\t // exekuzioa bukatzeko")
        err = 3
        pass



#fig = plt.figure(figsize=(8, 6), edgecolor='w')
#m = Basemap(projection='mill', resolution=None,
#            lat_0=0, lon_0=0)
#m.drawstates(color='b')
#m.bluemarble()
##draw_map(m)
#plt.show()
print("Nodo kopurua: ", iter)
print("Ezabatuta: ", remo)
print("Errorea: ", err)
