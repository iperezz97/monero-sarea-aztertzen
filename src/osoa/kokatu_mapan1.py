import time
import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
from itertools import chain
import socket
import struct
import ipaddress

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

def update_annot(ind,a):
    pos = sc[a].get_offsets()[ind["ind"][0]]
    annot.xy = pos
#    text = int2ip(info[a])
#    text = ipaddress.IPv4Address(info[a])
#    text = info[a]
    text = cits[a]
#    text = "{}".format(" ".join(list(map(str,ind["ind"]))))
    annot.set_text(text)
    annot.get_bbox_patch().set_facecolor('c')
    annot.get_bbox_patch().set_alpha(0.3)

def hover(event):
    a = 0
    aurk = 0
    for ssc in sc:
        if aurk == 1:
            break
        vis = annot.get_visible()
        if event.inaxes == ax:
            cont, ind = ssc.contains(event)
            if cont:
                aurk = 1
                update_annot(ind,a)
                annot.set_visible(True)
                fig.canvas.draw_idle()
            else:
                if vis:
#                    aurk = 1
                    annot.set_visible(False)
                    fig.canvas.draw_idle()
        a = a+1

### main exec ###

# ireki pipe-a
#path = "./fifoKanala"
#mode = 0o600
#os.mkfifo(path, mode) # sortuta C-n...
#fd = os.open(path, os.O_RDONLY) # ireki (deskriptorea)

plt.ion()
fig = plt.figure(figsize=(11, 6), edgecolor='w')
a = 0
ax = fig.add_subplot(111)
ax.set_title("Monero nodoak munduan zehar")

m = Basemap(projection='cyl', resolution=None,
            llcrnrlat=-90, urcrnrlat=90,
            llcrnrlon=-180, urcrnrlon=180, ax=ax)

draw_map(m)


sc = []
points = [] # mapako puntuak gorde
info = []
cits = []
iter = 0    # zenbat gehitu diren kontatu
remo = 0
print("Koordenatuak irakurtzen...")

annot = ax.annotate("", xy=(0,0), xytext=(12,12),textcoords="offset points",
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
annot.set_visible(False)
fig.canvas.mpl_connect("motion_notify_event", hover)

# Koordenatuak sarrera estandarretik irakurri begizta infinitu batean (lerroz lerro irakurriz, 'a <long> <lat>' puntua gehitzeko, 'r <long> <lat>' puntua ezabatzeko eta 'q' exekuzioa bukatzeko)
while True:
    try:
        err = 0
        #args = input("")
        args = input("<Eragiketa> <Longitude> <Latitude> <IP>  ")
        erag,valo,vala,ip,cit = args.split()
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
            s = plt.scatter(lonf,latf,c='b',alpha=0.04, s=20)
            sc.append(s)
#            info.append(int(ip))
            info.append(ip)
            cits.append(cit.replace('_', ' '))
            #print("SC", sc) # azkena gordetzen du soilik
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
#        time.sleep(1)
        pass


#fig = plt.figure(figsize=(8, 6), edgecolor='w')
#m = Basemap(projection='mill', resolution=None,
#            lat_0=0, lon_0=0)
#m.drawstates(color='b')
#m.bluemarble()
##draw_map(m)
#plt.show()
print("\nNodo kopurua: ", iter)
print("Ezabatuta: ", remo)
print("Errorea: ", err)
