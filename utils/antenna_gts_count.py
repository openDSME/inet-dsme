#!/usr/bin/env python

import argparse
import numpy
import re
from collections import deque

def printMatrix(a):
   rows = a.shape[0]
   cols = a.shape[1]
   for i in range(0,rows):
      for j in range(0,cols):
         print("%2.0f" %a[i,j]),
      print
   print


parser = argparse.ArgumentParser(description="Counts the number of slots allocated for each antenna.")
parser.add_argument("-l", "--log", type=str, required=True, help="the log file to parse")
parser.add_argument("-n", "--nodes", type=int, default="61", help="the number of non-antenna nodes")
parser.add_argument("-a", "--antennas", type=int, default="1", help="the number of antenna nodes")
parser.add_argument("-t", "--time", type=int, default="160", help="the warmup duration")
args = parser.parse_args()

numNodes = args.nodes + args.antennas

currentAllocationMatrix = numpy.zeros(numNodes)
totalAlloc = 0
totalDealloc = 0

allocationMatrixHistory = []

currentMatrixToAdd = currentAllocationMatrix.copy()
allocationMatrixHistory.append((0, currentMatrixToAdd))

pattern = re.compile("^\[\w*\]\s*([0-9.]*)\s*([0-9]*): ((de)?)alloc ([0-9]+)(.)([0-9]+) ([0-9]+),([0-9]+),([0-9]+)")
for line in open(args.log):
    m = pattern.match(line)
    if m:
        #print m.group(0)
        direction = ''
        node = int(m.group(2)) - 1
        time = float(m.group(1))
        if m.group(3) == 'de':
            totalDealloc += 1
            currentAllocationMatrix[node] -= 1
        else:
            totalAlloc += 1
            currentAllocationMatrix[node] += 1
        currentMatrixToAdd = currentAllocationMatrix.copy()
        allocationMatrixHistory.append((time, currentMatrixToAdd))

#print "Data collection finished"

events = len(allocationMatrixHistory)

best_slot_sum = 0
best_time = 0
for i in range(0, events):
    current_time, current_matrix = allocationMatrixHistory[i]
    current_slot_sum = 0
    for a in range(0, args.antennas):
        current_slot_sum += current_matrix[a]
    if(current_time > args.time and current_slot_sum > best_slot_sum):
        best_slot_sum = current_slot_sum
        best_time = current_time

print best_slot_sum

