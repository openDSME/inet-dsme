#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt
from pandas.tools.plotting import scatter_matrix
import numpy as np
from scipy.optimize import curve_fit
import random
import sys

from itertools import cycle, islice

df = pd.read_csv(sys.argv[1])
#df = df[df['from'] == 8]
#df = df[df['to'] == 2]
df = df[df['from'] == 2]
df = df[df['to'] == 1]
#df = df[df.index > 10000]

#df = df[df['slots'] > 0]
#df["expOut"] = df['musuDuration']/df['slots']
#df["cst2"] = df['queue']*df['expOut']

#df["cst"] = df['queue']*df['maOutTime']
#df['act'] = df['avgServiceTime'].ewm(span=30).mean()
#df['ma2'] = df['avgServiceTime'].ewm(span=100).mean()
#df['exp'] = df['cst'].ewm(span=30).mean()
#df['exp2'] = df['cst2'].ewm(span=30).mean()
#df = df[['maServiceTime','act','cst2','exp']]

#df = df[['avgServiceTime','maServiceTime','maExpectedServiceTime','expectedServiceTime']]
#df['exp'] = df['expectedServiceTime'].ewm(alpha=0.05).mean()
#df = df[['maExpectedServiceTime','expectedServiceTime','exp','maServiceTime','avgServiceTime']]
#df = df[['maExpectedServiceTime','maServiceTime','avgServiceTime']]
df['myStSlots'] = df['queue']*df['musuDuration']/df['avgServiceTime']
#df2 = df[['slots','optSlots','stSlots','myStSlots','finOptSlots']]
#df2 = df[['slots','optSlots','queue','predictedCapacity','maIn']]
#df2 = df[['slots','optSlots']]
df2 = df[['optSlots','requiredCapacity','predictedCapacity','Ipart']]
#df2 = df[['queue','queue']]

df2 = df2.set_index([range(0,len(df2))])

my_colors = list(islice(cycle(['b', 'r', 'y', 'k', 'g']), None, len(df)))

df2.plot(color=my_colors)
#df.slots.plot(label="slots")
#df.maStSlots.plot(label="stSlots")
#df.avgServiceTime.plot(secondary_y=True,style='y--')

plt.show()

