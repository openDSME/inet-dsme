#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt
from pandas.tools.plotting import scatter_matrix
import numpy as np
from scipy.optimize import curve_fit
import random
from itertools import cycle, islice
import statsmodels.api as sm
from statsmodels.graphics.api import qqplot
from scipy import stats
import scipy


df = pd.read_csv('test.log')
df = df[df['from'] == 8]
df = df[df['to'] == 2]
#df = df[df['from'] == 2]
#df = df[df['to'] == 1]

#df = df[['in','in']]

my_colors = list(islice(cycle(['b', 'r', 'y', 'k', 'g']), None, len(df)))

#dta = df['optSlots'].astype(float)

df['actCap'] = df['musuDuration']/df['maxServiceTimePerQueueLength']
#df['actCap'] = df['musuDuration']/df['maServiceTimePerQueueLength']
df['minCap'] = df[['actCap','slots']].min(axis=1)
df = df[['optSlots','slots','minCap']]
print df

#df = df[['maServiceTimePerQueueLength','maServiceTime','queue','actCap']]
#df['queue'] = 1000*df['queue']
#df['actCap'] = 1000*df['actCap']
#df = df[df.index > 5000]
#df = df[df.index < 10000]

df.plot(color=my_colors)

#plt.show()

df2 = df[['optSlots','minCap']]
df2.to_csv('foo.csv',index=False)

