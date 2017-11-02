#!/usr/bin/env python

import sys
import fileinput
import matplotlib.pyplot as plt
import csv

ad = {'0xc374': 1, '0x2353': 3, '0x3560': 7, '0x4061': 10, '0x0': 5, '0x2460': 4, '0x2760': 6, '0x2061': 8, '0x2453': 11, '0xffff': 2, '0xc471': 9}
ad = {v: k for k,v in ad.iteritems()}
print ad

data = {}
with open(sys.argv[1], 'rb') as csvfile:
    reader = csv.reader(csvfile, quotechar=',')
    for row in reader:
        a = ad[int(row[1])] 
        if a not in data.keys():
            data[a] = ([],[])

        data[a][0].append(row[0])
        data[a][1].append(row[2])

fig = plt.figure()
for k,v in data.iteritems():
    plt.plot(v[0],v[1],'.',label=k)
lgd = plt.legend(bbox_to_anchor=(1.05,1), loc=2, borderaxespad=0.)
plt.ylabel('beacon lateness [symbols]')
plt.xlabel('time [s]')
plt.savefig('lateness.pdf',bbox_extra_artists=(lgd,), bbox_inches='tight')
