from __future__ import unicode_literals
import time
import sys
import os
import numpy as np
#from matplotlib import font_manager as fm, rcParams
import matplotlib #
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
from itertools import chain
import socket
import struct
import ipaddress
from typing import NamedTuple

class Coords(NamedTuple):
    lon: float
    lat: float

class MyUbic(NamedTuple):
    coord: Coords
    cit: str
    ips: list


# Mapa marrazteko (hasieratzeko) metodoa
def draw_map(m, scale=0.1):
    # draw a shaded-relief image
#    m.shadedrelief(scale=scale)

    # lats and longs are returned as a dictionary
    lats = m.drawparallels(np.linspace(-90, 90, 13), labels=[1,0,0,0])
    lons = m.drawmeridians(np.linspace(-180, 180, 13), labels=[0,0,0,1])

    # keys contain the plt.Line2D instances
    lat_lines = chain(*(tup[1][0] for tup in lats.items()))
    lon_lines = chain(*(tup[1][0] for tup in lons.items()))
    all_lines = chain(lat_lines, lon_lines)

    # cycle through these lines and set the desired style
    for line in all_lines:
#        line.set(linestyle='-', alpha=0.2, color='w')
        line.set(linestyle=':', alpha=0.2, color='w')


# Puntua mapatik ezabatzeko metodoa
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

    iter = 0
    for scc in sc:
        rkoord = scc.get_offsets()[0]
        if(rkoord[0] == x and rkoord[1] == y):
            scc.remove()
            sc.pop(iter)
            break
        iter = iter + 1


# Puntua mapan gehitzeko metodoa
def draw_point(lonf, latf):
    if(lonf > 180.0 or lonf < -180.0 or latf > 90.0 or latf < -90.0):
        print("E4\tLongitudeak -180.0 eta 180.0 arteko balioak onartzen ditu.\n \tLatitudeak -90.0 eta 90.0 arteko balioak onartzen ditu.")
    else:
        #lox, lay = m(lonf, latf)
        points.extend( # gorde puntuak (Line2D objektuak) ondoren ezabatzeko
            m.plot(lonf, latf, markerfacecolor='red', markeredgecolor='black', marker='o', markersize=2.7, alpha=0.5)
        )

        fig.canvas.draw()
        fig.canvas.flush_events()

# Koordenatu horiek mapan gehitu badira, gorde diren zerrendako posizioa itzuliko du
# Zerrendan oraindik ez badago -1 itzuliko du
def aurkitu(koord):
    ind = 0
    for i in items:
        if koord == i.coord:
            return ind
        ind = ind + 1
    return -1


# ez da erabiltzen (ip - string) ... IP irakurri eta hover ekintza gertatzean adierazi (informazio gehigarri bezala)
def ip2int(addr):
    return struct.unpack("!I", socket.inet_aton(addr))[0]


def int2ip(addr):
    return socket.inet_ntoa(struct.pack("!I", addr))
#

# Mapan label bat (informazioarekin) gehitzeko metodoa (hover kasuan)
def update_annot(ind,a):
    pos = sc[a].get_offsets()[ind["ind"][0]]
    annot.xy = pos
    pp = Coords(pos[0],pos[1])
    ppos = aurkitu(pp)
    text = u""+items[ppos].cit
    text2 = ""
    if len(items[ppos].ips) > 20:
        its = items[ppos].ips[:20]
        text2 = "\n"+str(len(items[ppos].ips)-20)+" nodo gehiago..."
    else:
        its = items[ppos].ips
    ids = '\n'.join(its)
    text = text +"\n"+ids+text2
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
fig = plt.figure(figsize=(13, 7), edgecolor='w')
a = 0
ax = fig.add_subplot(111)
ax.set_title("Monero nodoak munduan zehar")
plt.rc('axes', unicode_minus=False)
#matplotlib.rc('font', family='Arial')
#matplotlib.rc('font', **{'sans-serif' : 'Arial',
#                         'family' : 'sans-serif'})

# Sortu mapa
m = Basemap(projection='cyl', resolution=None,
            llcrnrlat=-90, urcrnrlat=90,
            llcrnrlon=-180, urcrnrlon=180, ax=ax)
draw_map(m)

#m = Basemap(projection='mill', resolution=None,
#            lat_0=0, lon_0=0)
#m.drawstates(color='b')

m.bluemarble(scale=0.1)
#m.bluemarble(scale=0.5)
#m.bluemarble()

#m.drawmeridians(np.arange(m.lonmin,m.lonmax+15,30),size=7,labels=[0,0,0,1],color='gray',dashes=(1,4))
#parallels = np.arange(m.latmin,m.latmax+15,30)
#m.drawparallels(parallels,size=7,labels=[1,0,0,0],color='gray',dashes=(1,4))
#m.drawcountries()

#m = Basemap(projection='merc', resolution=None,
#            llcrnrlat=-90, urcrnrlat=90,
#            llcrnrlon=-180, urcrnrlon=180, ax=ax)

#m.drawcountries()
#m.drawmeridians(np.arange(m.lonmin,m.lonmax+30,60),labels=[0,0,0,1])
#m.bluemarble()


sc = []
points = [] # mapako puntuak gorde
items = []  # gorde nodoen informazioa
iter = 0    # zenbat gehitu diren kontatu
remo = 0
print("Koordenatuak irakurtzen...")

annot = ax.annotate("", xy=(0,0), xytext=(14,-25),textcoords="offset points",size=8,
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
annot.set_visible(False)
fig.canvas.mpl_connect("motion_notify_event", hover)
err = 0

print("<Eragiketa> <Longitude> <Latitude> <IP> <City>:") # Sarreraren formatua
# Koordenatuak sarrera estandarretik irakurri begizta infinitu batean (lerroz lerro irakurriz, 'a <long> <lat>' puntua gehitzeko, 'r <long> <lat>' puntua ezabatzeko eta 'q' exekuzioa bukatzeko)
while True:
    try:
        args = input("")
#        args = input("<Eragiketa> <Longitude> <Latitude> <IP> <City>:")
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
            berri = Coords(lonf,latf)
            index = aurkitu(berri)
            city = cit.replace('_', ' ')
            if lonf == 9.490900 and latf == 51.299301:
                city="Kassel"
            if lonf == 4.899500 and latf == 52.382401:
                city="Amsterdam"
            if lonf == 2.338700 and latf == 48.858200:
                city="Paris"
            if lonf == 21.225700 and latf == 45.753700:
                city="Timisoara"
            if lonf == 14.586400 and latf == 50.679600:
                city="Ceska Lipa"
            if lonf == 49.169300 and latf == 55.767502:
                city="Kazan"
            if lonf == 14.510600 and latf == 49.019299:
                city="Ceske Budejovice"
            if lonf == 19.141500 and latf == 50.211700:
                city="Myslowice"
            if lonf == -97.821999 and latf == 37.750999:
                city="(Cheney Reservoir)"

            if index == -1: # sortu puntu berria
                it = MyUbic(berri, city, [ip])
                items.append(it)
                s = plt.scatter(lonf,latf,c='r',alpha=0.02, s=17)
            else:           # txertatu ip bat puntuan
                items[index].ips.append(ip) # koordenatu horretan gehitu ip-a
                s = plt.scatter(lonf,latf,c='r',alpha=0.03, s=len(items[index].ips)/2+17)
            draw_point(lonf, latf)
            sc.append(s)
            iter = iter + 1 # kontatu

        elif(erag == 'r'): # puntua mapatik ezabatu
            remove_point(lonf, latf)
            ri = aurkitu(Coords(lonf,latf))
            rin = 0
            for ipak in items[ri].ips:
                if ipak == ip:
                    items[ri].ips.pop(rin)
                    if len(items[ri].ips) == 0:
                        items.pop(ri)
                    break
                rin = rin + 1
            remo = remo + 1 # kontatu

    except ValueError:
        if(args == 'q'): # exekuzioa bukatzeko 'q'
            break
        else:
            print("E2 Ez da formatu egokia erabili: \n >>> 'a <long> <lat> <IP> <city>' \t // puntua mapan gehitzeko \n >>> 'r <long> <lat> <IP> 0' \t\t // puntua mapatik ezabatzeko \n >>> 'q' \t\t\t\t // exekuzioa bukatzeko")
            err = err +1
        pass
    except KeyboardInterrupt: # (Ctrl + c) edo kill -2 PID... baina ez da beti harrapatzen
        break
    except Exception as e:
        print(e)
#        if(args == ' ' or args == '\n' or args == '' or not args):
#            time.sleep(1)
#            continue
#        print("E3 Ez da formatu egokia erabili: \n >>> 'a <long> <lat>' \t // puntua mapan gehitzeko \n >>> 'r <long> <lat>' \t // puntua mapatik ezabatzeko \n >>> 'q' \t\t // exekuzioa bukatzeko")

#        print("E3") # input bukatuta
        err = err + 1
        time.sleep(1)
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
print("Errorerik: ", err)
#plt.show()
plt.savefig("test.svg", format="svg")
print("Irudia gordeta 'test.svg' fitxategian")
