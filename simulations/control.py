#!/usr/bin/env python

import pandas as pd
import matplotlib.pyplot as plt
from pandas.tools.plotting import scatter_matrix
import numpy as np
from scipy.optimize import curve_fit
import random

#df = pd.read_csv('foo.csv')
df = pd.read_csv('test.log')

print df

#df = df[df['address']==2]

#df = df[["maIn","slots","maxServiceTime","maOutTime"]]
#df = df[["maInTime","maOutTime","avgServiceTime","maxServiceTime","maIn","maServiceTime"]]
#df = df[df['maInTime'] > 0]
#df = df[df['maOutTime'] > 0]
#df['la'] = 1/df['maInTime']
#df['mu'] = 1/df['maOutTime']
#df = df[abs((df['mu']-df['la'])) > 0.000001]
#df['W'] = 1/(df['mu']-df['la'])
#df['W'] = df[df['W'] < 0.1]
#df = df[["la","mu","W","maServiceTime","maxServiceTime","slots"]]
#df = df.dropna()
#df = df[df['avgServiceTime'] > 0]
#df['invST'] = 1/df['avgServiceTime']
#df = df[df['avgServiceTime'] < 100000]
#df['invST'] = df['maxServiceTime']
#df['TS'] = TS(1/df['maInTime'],1/df['maOutTime'],df['queue'])
#df['TS'] = TS(1/df['maInTime'],1/df['maOutTime'],10)
#df["cst"] = df['queue']*df['maOutTime']
#df = df[df['maOutTime'] < 100000]
df = df[df['slots'] > 0]
df["expOut"] = df['musuDuration']/df['slots']
#df["cst2"] = df['queue']*df['expOut']
#df = df[["maServiceTime","cst",'maInTime','queue','slots']]
#df = df[["maServiceTime","cst","cst2"]]
#df = df[["maOutTime","expOut"]]
df['ma'] = df["maServiceTimePerQueueLength"]
df['max'] = df["maxServiceTimePerQueueLength"]
df['avg'] = df["avgServiceTimePerQueueLength"]
df = df[df['ma'] < 100000]
df = df[df['max'] < 100000]
df = df[df['avg'] < 100000]
df = df[df['maOutTime'] < 100000]
df['xt'] = df[['ma','expOut']].max(axis=1)
#df['zz'] = df[['ma','maInTime']].max(axis=1)
#df = df[["maOutTime","maServiceTimePerQueueLength","maxServiceTimePerQueueLength","avgServiceTimePerQueueLength"]]
df = df[["maOutTime","ma","max","avg","xt"]]

#plt.figure()
#df.plot()

scatter_matrix(df, alpha=0.05, diagonal='kde')
#show()
plt.show()
exit(1)

df['maInInt'] = np.int64(df['maIn']*100.0)/100.0
df = df[df['maInInt']>0.266]
print df
#da = df.groupby('maInInt')['maxServiceTime'].mean()
da = df.groupby('maInInt')['slots'].mean()
#da = da.loc[:,('maxServiceTime')]
#print da.index.values
#print da.values
#da.plot()
#plt.show()
x = da.index.values
yn = da.values

def func(x, a, b, c):
    return a * np.exp(-b * x) + c

def func2(x, a, b, c):
    x[(x+b)==0] = 0.001
    return (a/(x+b)+c)

def func3(x, a, b, c):
    return a*x*x+b*x+c

def func4(x, a, b, c):
    return np.floor(a*x*x+b*x+c)

def func5(x, a, b, c):
    def myfunc(a):
        return random.random()-0.5
    myfunc = np.vectorize(myfunc)
    #print myfunc(x)
    return np.round(a*x*x+b*x+c+myfunc(x))
    

#x = np.linspace(0,4,50)
#y = func(x, 2.5, 1.3, 0.5)
#yn = y + 0.2*np.random.normal(size=len(x))
#print yn

#popt, pcov = curve_fit(func, x, yn)
#popt, pcov = curve_fit(func2, x, yn)
popt, pcov = curve_fit(func3, x, yn)
print popt, pcov

plt.figure()
plt.plot(x, yn, 'ko', label="Original Noised Data")
#plt.plot(x, func(x, *popt), 'r-', label="Fitted Curve")
#plt.plot(x, func2(x, *popt), 'r-', label="Fitted Curve")
plt.plot(x, func3(x, *popt), 'r-', label="Fitted Curve")
plt.plot(x, func5(x, *popt), 'y-', label="Y")
plt.plot(x, func4(x, 0, 1, 1), 'b-', label="B")
plt.legend()
plt.show()
