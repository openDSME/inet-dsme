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
#df.plot(color=my_colors)

#plt.show()

for a in range(0,100):
    alpha = a/100.0
    dta = pd.DataFrame()
    dta['in'] = df['in'].astype(float)
    dta['pin'] = dta['in']

    dta['pin'][0] = 0
    ma = 0
    #alpha = 0.9
    for i in range(1,len(dta)):
        current = dta['in'].iloc[i-1]
        ma = alpha*ma+(1-alpha)*current
        dta['pin'].iloc[i] = ma

    dta = dta[dta.index > 2000]

    dta['err'] = (dta['in']-dta['pin']).pow(2)
    meanerr = dta['err'].mean()
    print alpha,meanerr

    #dta.plot()

    #plt.show()


