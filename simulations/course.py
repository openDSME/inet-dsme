#!/usr/bin/env python

import subprocess
import re
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-c','--cached')
args = parser.parse_args()

if not args.cached:
    subprocess.call("grep -o  \"control,.*\" mac.log > control.csv",shell=True)

df = pd.read_csv('control.csv')
del df['control']
df = df[df['from'] == 10]
df = df[df['to'] == 4]

print df

