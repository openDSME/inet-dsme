#!/usr/bin/env python

import argparse
import numpy
import re
from collections import deque

from matplotlib import pyplot as plt
from matplotlib import animation
from matplotlib.patches import Circle
from matplotlib.collections import LineCollection
from matplotlib.colors import ListedColormap, BoundaryNorm

def printMatrix(a):
   rows = a.shape[0]
   cols = a.shape[1]
   for i in range(0,rows):
      for j in range(0,cols):
         print("%2.0f" %a[i,j]),
      print
   print


parser = argparse.ArgumentParser(description="Creates an animation from a log file to visualise packet loss.")
parser.add_argument("-l", "--log", type=str, required=True, help="the log file to parse")
parser.add_argument("-o", "--output", type=str, default="gts_allocation.mp4", help="the output file")
parser.add_argument("-n", "--nodes", type=int, default="62", help="the number of nodes")
args = parser.parse_args()

numNodes = args.nodes

currentLossMatrix = numpy.zeros((numNodes, numNodes))
positionVector = numpy.zeros((numNodes,2))
totalLoss = 0

lossMatrixHistory = []

f = open(args.log)
for i in range(1000):
    line = f.next()
#for line in open(args.log):
    m = re.search("^\[\w*\]\s*0\s*([0-9]*): POSITION: x=([0-9.]+), y=([0-9.]+)", line)
    if m:
        #print m.group(0)
        index = int(m.group(1))-1
        positionVector[index][0] = float(m.group(2))
        positionVector[index][1] = float(m.group(3))

currentMatrixToAdd = currentLossMatrix.copy()
lossMatrixHistory.append((0, currentMatrixToAdd))

for line in open(args.log):
    m = re.search("^\[\w*\]\s*([0-9.]*)\s*([0-9]+): DROPPED->([0-9]+):.*", line)
    if m:
        #print m.group(0)
        source = int(m.group(2)) - 1
        destination = int(m.group(3)) - 1
        time = float(m.group(1))
        totalLoss += 1
        currentLossMatrix[source][destination] += 1
        currentMatrixToAdd = currentLossMatrix.copy()
        lossMatrixHistory.append((time, currentMatrixToAdd))


fig = plt.figure(figsize=(10,10))
ax = plt.axes(xlim=(-100, 900), ylim=(-100, 900))
time_text = ax.text(0.02, 0.95, '', transform=ax.transAxes)
last_time_text = ax.text(0.02, 0.90, '', transform=ax.transAxes)
ax.set_ylim(ax.get_ylim()[::-1])

for i in range(0, numNodes):
    x = positionVector[i][0]
    y = positionVector[i][1]
    circle = Circle((x, y), 10)
    ax.add_artist(circle)

cmap = ListedColormap(['b', 'r', 'b'])
norm = BoundaryNorm([0, 1, 1.5, 2], cmap.N)

line = LineCollection([], cmap=cmap, norm=norm,lw=2)
ax.add_collection(line)

keys = len(lossMatrixHistory)

def init():
    line.set_segments([])
    time_text.set_text('')
    last_time_text.set_text('')
    return line, time_text, last_time_text

previous_key = 0
def animate(i):
    global previous_key
    lastKey = 0
    next_time, unused = lossMatrixHistory[lastKey + 1]
    while lastKey + 2 < keys and i > int(next_time):
        lastKey += 1
        next_time, unused = lossMatrixHistory[lastKey + 1]

    time, lossMatrix = lossMatrixHistory[lastKey]
 
    time_text.set_text("time=" + "%4d" % i + "s")
    last_time_text.set_text(time)
    segments = []
    widths = []
    colors = []
    for s in range(0,numNodes):
        for d in range(0,numNodes):
            if lossMatrix[s][d] > 0:
                xs = positionVector[s][0];
                ys = positionVector[s][1];
                xd = positionVector[d][0];
                yd = positionVector[d][1];
                #print str(s) + "->" + str(d) 
                segments.append([(xs, ys), (xd, yd)])
                if lastKey > 0:
                    unused_time, lastMatrix = lossMatrixHistory[previous_key]
                    if int(lossMatrix[s][d]) == int(lastMatrix[s][d]):
                        colors.append("#FFFFFF")
                        widths.append(0)
                    elif int(lossMatrix[s][d]) < int(lastMatrix[s][d]):
                        colors.append("#00FF00")
                        widths.append(2)
                    else:
                        colors.append("#FF0000")
                        widths.append(2)
                else:
                    widths.append(0)
                    colors.append("#0000FF")
    if colors:    
        line.set_edgecolors(colors)
        line.set_facecolors(colors)
    if widths:
        line.set_linewidths(widths)
    line.set_segments(segments)
    previous_key = lastKey
    return line, time_text, last_time_text

final_time, unused =  lossMatrixHistory[keys - 1]
anim = animation.FuncAnimation(fig, animate, init_func=init, frames=int(final_time) + 10, interval=1, blit=True)

anim.save(args.output, writer='avconv', fps=2, extra_args=['-vcodec', 'libx264'])

plt.show()

print "Loss %i\n"%(totalLoss)

