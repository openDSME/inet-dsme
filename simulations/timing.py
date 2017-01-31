#!/usr/bin/env python

import fileinput
import re
import subprocess

import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.font_manager as font_manager
import matplotlib.dates
from matplotlib.dates import MONTHLY, DateFormatter, rrulewrapper, RRuleLocator
from sets import Set
 
from pylab import *

data = {}
types = Set()
maxtime = 0

symbolDuration = (16/(1000.0*1000.0))

reftime = -1

INET = 0
CONTIKI = 1
log = INET

for line in fileinput.input():
    if log == INET:
        m = re.search("\[INFO\]\s*([0-9.]*)\s*([0-9]*):\s*[0-9]*\|[0-9]*\|[0-9]*\|(.*)\|([0-9]*)",line)
    else:
        m = re.search("Contiki [0-9]+\s*([0-9.]+)\s*Mote Contiki ([0-9]+).*br.[0-9]+\|[0-9]+\|[0-9]+\|(.*)<br.",line)
    if m:
        time = float(m.group(1))
        if log == CONTIKI:
            time /= 1000000.0

        if time > 40:
            continue

        print line
        sender = int(m.group(2))
        mtype = m.group(3)

        if reftime < 0 and mtype == "BEACON":
            reftime = time            
        duration = float(m.group(4))*symbolDuration
        #print time,sender,mtype

        if sender not in data:
            data[sender] = []
        data[sender].append((time,mtype,duration))

        types.add(mtype)
        maxtime = max(time,maxtime)

        #if time > 40:
        #    break

SO = 3
MO = 5
BO = 7
symbolsPerSlot = 60*(2**SO)
slotDuration = symbolsPerSlot*symbolDuration
aBaseSuperframeDuration = 16*slotDuration
#reftime +=  int(250 / aBaseSuperframeDuration)*aBaseSuperframeDuration

def fill_grid():
    musus = 2**(BO-MO)
    sus = 2**(MO-SO)
    grid = []
    for bi in range(0,30):
        for musu in range(0,musus):
            for su in range(0,sus):
                for slot in range(0,16):
                    slotStart = reftime+slotDuration*(slot+(16*(su+sus*(musu+(bi*musus)))))
                    
                    #((bi*(2**BO))+(musu*(2**MO))+(su*(2**SO)))*aBaseSuperframeDuration
                    #+slot*slotDuration
                    #mtype = "GTS"
                    #if slot == 0:
                    #    mtype = "BEACON"
                    #elif slot <= 8:
                    #    mtype = "CAP"

                    grid.append((slotStart,slot,slotDuration))

                    if slotStart > maxtime:
                        return grid
    return grid

grid = fill_grid()

types = sorted(list(types))
colors = {}
i = 0
colorstep = 255/len(types)
for mtype in types:
    red = i*colorstep
    green = i*colorstep
    blue = i*colorstep
    colors[mtype] = "#%02x%02x%02x"%(red,green,blue)
    i += 1

colors['BEACON'] = "#0000ff"
#colors['DATA'] = "#aa00ff"
colors['DATA'] = "#ff0000"

#print colors

maxsender = max(data.keys())
pos = arange(1.5,maxsender+1.5,0.5)
 
fig = plt.figure(figsize=((maxtime-reftime)*10,10))
ax = fig.add_subplot(111)

for area in grid:
        start = area[0]
        end = start+area[2]
        slot = area[1]
        if slot == 0:
            red = 0
            green = 0
            blue = 255
        elif slot <= 8:
            red = 0
            green = 255
            blue = 0
        else:
            red = 255
            green = 0
            blue = 0

        if slot % 2:
            red = min(red+80,255)
            green = min(green+80,255)
            blue = min(blue+80,255)
        color = "#%02x%02x%02x"%(red,green,blue)
        ax.barh(1.0, end - start, left=start, height=maxsender*0.5, color=color, alpha = 0.1, linewidth=0)
 
for sender in sorted(data.keys()):
    for pkt in data[sender]:
        start = pkt[0]
        end = start+pkt[2]
        if pkt[1] == "BEACON":
            lateness = int(round((start - reftime)/symbolDuration))%(symbolsPerSlot*16)
            if lateness > (symbolsPerSlot*16)/2:
                lateness -= (symbolsPerSlot*16)
            print "%f,%i,%i"%(start,sender,lateness)
        ax.barh((sender*0.5)+1.0, end - start, left=start, height=0.5, align='center', color=colors[pkt[1]], alpha = 1, linewidth=0)

 
# Format the y-axis
 
locsy, labelsy = yticks(pos,range(1,maxsender+1))
plt.setp(labelsy, fontsize = 14)
 
# Format the x-axis
 
ax.axis('tight')
#ax.set_ylim(ymin = -0.1, ymax = 4.5)
ax.set_xlim(xmin = reftime, xmax = maxtime)
#ax.grid(color = 'g', linestyle = ':')
 
#ax.xaxis_date() #Tell matplotlib that these are dates...
 
#rule = rrulewrapper(MONTHLY, interval=1)
#loc = RRuleLocator(rule)
#formatter = DateFormatter("%b '%y")
 
#ax.xaxis.set_major_locator(loc)
#ax.xaxis.set_major_formatter(formatter)
#labelsx = ax.get_xticklabels()
#plt.setp(labelsx, rotation=30, fontsize=12)
 
# Format the legend
 
#font = font_manager.FontProperties(size='small')
#ax.legend(loc=1,prop=font)
 
# Finish up
ax.invert_yaxis()
#fig.autofmt_xdate()
#plt.savefig('gantt.svg')
#plt.show()
plt.savefig('foo.pdf',bbox_inches='tight')

subprocess.call(['convert','-density','200','foo.pdf','foo.png'])
