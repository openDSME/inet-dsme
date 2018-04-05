#!/usr/bin/env python

import argparse
import re
import numpy
import subprocess
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description="Creates an animation from a log file to visualise the GTS allocation process.")
parser.add_argument("-l", "--log", type=str, required=True, help="the log file to parse")
args = parser.parse_args()

channels = 16
gtSlots = 7
superframes = 4
#superframes = 1
finalcapslot=8
totalalloc = 0
totaldealloc = 0

pernode = {}

msf = []
for j in range(0,channels):
    b = []
    for i in range(0,superframes):
        x = []
        for j in range(0,gtSlots):
            x.append([])
        b.append(x)
    msf.append(b)

tab = open("table.tex",'w')
tab.write("""
\documentclass{article}
\usepackage{tikz}
%\usepackage{tabularx}
\usepackage[active,tightpage]{preview}
\PreviewEnvironment{tikzpicture} 
\\begin{document}""")

times = []

def printtable(time):
    tab.write("""\\begin{tikzpicture}
    \\node{
    \\begin{tabular}{||""")
    for superframe in range(0,superframes):
        for gtSlot in range(0,gtSlots):
            tab.write("p{1cm} |")
        tab.write("|")
    tab.write("""} \hline\n""")
    tab.write(time + """\\\\\hline\n""")
    for channel in range(0,channels):
        tab.write('&'.join(map(lambda x: ', '.join(x), [item for sublist in msf[channel] for item in sublist])))
        #tab.write('&'.join([item for sublist in msf[channel] for item in sublist]))
        tab.write("\\\\[1cm]\hline\n")
    tab.write("""\end{tabular}
    };
    \end{tikzpicture}""")

for line in open(args.log):
    m = re.search("(.*?)((de)?)alloc ([0-9]+)(.)([0-9]+) ([0-9]+),([0-9]+),([0-9]+)", line)
    if m:
        print m.group(0)
        direction = ''
        if m.group(5) == '>':
            direction = "$\\rightarrow$"
        else:
            direction = "$\\leftarrow$"
        #+"$\\to$"
        text = m.group(4)+direction+m.group(6)
        array = msf[int(m.group(9))][int(m.group(8))][int(m.group(7))-finalcapslot-1]
        node = int(m.group(4))
        if not node in pernode:
            pernode[node] = 0
        if m.group(2) == 'de':
            totaldealloc += 1
            pernode[node] -= 1
            array.remove(text)
        else:
            totalalloc += 1
            pernode[node] += 1
            array.append(text)

        prefix = m.group(1)
        n = re.search("^\[\w*\]\s*([0-9.]*)\s*[0-9]*:", prefix)
        if n:
            time = n.group(1)
        else:
            n = re.search("^([0-9.]*);", prefix)
            if n:
                time = n.group(1)
        if n:
            printtable(time)
            times.append(float(time))

print msf

tab.write("""
\end{document}
""")

tab.close()

subprocess.call(['pdflatex', '-interaction=batchmode', 'table.tex'])

print pernode

print "Dealloc %i  Alloc %i\n"%(totaldealloc,totalalloc)

plt.hist(times)
plt.show()

