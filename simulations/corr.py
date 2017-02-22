from scipy import optimize
from math import *
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def find_rho_Pb(K,PRR):
    PER = 1-PRR

    def Pb(rho):
        pk = (rho-1)/(rho-(1/(rho**K)))
        return (pk-PER)**2

    #cons = ({'type':'ineq',
    #         'fun': lambda x: 0.9999 - x})
    #res = optimize.minimize(Pb, 0.9, constraints = cons)

    res = optimize.minimize(Pb, 0.9)
    return res['x'][0]

def find_rho_Pbs(K,PRR,s):
    PER = 1-PRR
    sq = s*s
    sqesq = sqrt(exp(-sq))

    def Pbs(rho):
        d = sqesq*sqrt(rho)*(sq-1)
        pk = ((rho**((d+2*K)/(2+d))*(rho-1)) / (rho**(2*((d+K+1)/(2+d)))-1)) 
        return (pk-PER)**2

    res = optimize.minimize(Pbs, 0.9)
    return res['x'][0]

K = 20
PRR = 0.99
print find_rho_Pb(K,PRR)

sd = []
rd = []
for i in range(1,200):
    s = i/100.0
    sd.append(s)
    rd.append(find_rho_Pbs(K,PRR,s))

z = np.polyfit(sd,rd,3)
p = np.poly1d(z)

df = pd.DataFrame({'r':rd},index=sd)
df['p'] = p(df.index)
#print df
df.plot()
plt.show()
