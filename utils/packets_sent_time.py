#!/usr/bin/env python

import argparse
import numpy
import re
import math
from collections import deque

def printMatrix(a):
   rows = a.shape[0]
   cols = a.shape[1]
   for i in range(0,rows):
      for j in range(0,cols):
         print("%2.0f" %a[i,j]),
      print
   print


parser = argparse.ArgumentParser(description="Extracts the number of allocated and deallocated slots per second.")
parser.add_argument("-l", "--log", type=str, required=True, help="the log file to parse")
parser.add_argument("-o", "--output", type=str, default="packets_sent.csv", help="the output file")
parser.add_argument("-s", "--step", type=str, default="1", help="unit of time")
args = parser.parse_args()

length = 500 / int(args.step)

sentVector = numpy.zeros((length,2))
totalSent = 0

for i in range(0,length):
    sentVector[i][0] = i * int(args.step)

for line in open(args.log):
    m = re.search("^\[\w*\]\s*([0-9.]*)\s*[0-9]*: Controller-Incoming", line)
    if m:
        #print m.group(0)
        time = float(m.group(1))
        pos = math.floor(math.floor(time) / int(args.step))
        totalSent += 1
        sentVector[pos][1] += 1


print "Data collection finished"

#printMatrix(allocationVector)
numpy.savetxt(args.output, sentVector, delimiter=";")

print "Sent %i\n"%(totalSent)

