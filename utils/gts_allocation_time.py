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
parser.add_argument("-o", "--output", type=str, default="gts_allocation.csv", help="the output file")
args = parser.parse_args()

allocationVector = numpy.zeros((500,2))
totalAlloc = 0
totalDealloc = 0

for line in open(args.log):
    m = re.search("^\[\w*\]\s*([0-9.]*)\s*[0-9]*: ((de)?)alloc ([0-9]+)(.)([0-9]+) ([0-9]+),([0-9]+),([0-9]+)", line)
    if m:
        #print m.group(0)
        direction = ''
        if m.group(5) == '>':
            source = int(m.group(4)) - 1
            destination = int(m.group(6)) - 1
            time = float(m.group(1))
            pos = math.floor(time)
            if m.group(2) == 'de':
                totalDealloc += 1
                allocationVector[pos][0] += 1
            else:
                totalAlloc += 1
                allocationVector[pos][1] += 1


print "Data collection finished"

printMatrix(allocationVector)
numpy.savetxt(args.output, allocationVector, delimiter=";")

print "Dealloc %i  Alloc %i\n"%(totalDealloc,totalAlloc)

