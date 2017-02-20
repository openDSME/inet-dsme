#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt
from pandas.tools.plotting import scatter_matrix
import numpy as np
from scipy.optimize import curve_fit
import random

from itertools import cycle, islice

df = pd.read_csv('test.log')
#df = df[df['from'] == 8]
#df = df[df['to'] == 2]
df = df[df['from'] == 2]
df = df[df['to'] == 1]
#df = df[df.index > 500]


#df['myStSlots'] = df['queue']*df['musuDuration']/df['maServiceTime']
#df2 = df[['queue','stSlots','slots','myStSlots']]
df2 = df[['queue','queue']]

my_colors = list(islice(cycle(['b', 'r', 'y', 'k', 'g']), None, len(df)))

df2.plot(color=my_colors)
#df.avgServiceTime.plot(secondary_y=True,style='y--')
#df.maServiceTime.plot(secondary_y=True,style='y--')
plt.show()

