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

dta = df['in'].astype(float)
#dta = pd.DataFrame(np.random.normal(0,1,600))
#dta = pd.DataFrame(stats.poisson.rvs(10,size=600).astype(float))

dta.index = pd.Index(sm.tsa.datetools.dates_from_range('1700', str(1700+len(dta)-1)))

fig = plt.figure(figsize=(12,8))
ax1 = fig.add_subplot(211)
fig = sm.graphics.tsa.plot_acf(dta.values.squeeze(), lags=40, ax=ax1)
ax2 = fig.add_subplot(212)
fig = sm.graphics.tsa.plot_pacf(dta, lags=40, ax=ax2)

plt.show()

arma_mod20 = sm.tsa.ARMA(dta, (2,0)).fit()
print arma_mod20.params

arma_mod30 = sm.tsa.ARMA(dta, (1,1)).fit()
print arma_mod20.aic, arma_mod20.bic, arma_mod20.hqic

print arma_mod30.params

print arma_mod30.aic, arma_mod30.bic, arma_mod30.hqic

###################

sm.stats.durbin_watson(arma_mod30.resid.values)

fig = plt.figure(figsize=(12,8))
ax = fig.add_subplot(111)
ax = arma_mod30.resid.plot(ax=ax);

resid = arma_mod30.resid

stats.normaltest(resid)

fig = plt.figure(figsize=(12,8))
ax = fig.add_subplot(111)
fig = qqplot(resid, line='q', ax=ax, fit=True)

plt.show()

###################

predict_sunspots = arma_mod30.predict('1980', '2100', dynamic=False)
print predict_sunspots

ax = dta.ix['1950':].plot(figsize=(12,8))
ax = predict_sunspots.plot(ax=ax, style='r--', label='Dynamic Prediction');
ax.legend();
#ax.axis((-20.0, 38.0, -4.0, 200.0));

plt.show()


