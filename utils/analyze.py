#!/usr/bin/env python3

from Run import Run
#import ConfigParser

import argparse
import sys
import os
import csv
import glob
from subprocess import *
import numpy as np
import scipy as sp
import scipy.stats

#from sets import Set

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    m = np.mean(a)
    se = 0
    if len(a) > 1:
        se = scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1+confidence)/2., n-1)
    return m, h

def main(args):
    measures = ['minPRR','meanPRR','maxDelay','meanDelay']
    args.parameter = ['configname']+args.parameter

    allhosts = set()
    aggregated = {}
    runs = []
    repetitions = set()
    for f in args.input:
        r = Run()
        r.load(f)

        allhosts.update(r.hosts.keys())

        runs.append(r)
        
        repetitions.add(r.param["seedset"])
        for m in measures:
            sub = aggregated
            for param in args.parameter:
                sub = sub.setdefault(r.param[param],{})
            sub.setdefault(m,{})[r.param["seedset"]] = r.measure[m]

    print(aggregated)

    with open(args.output+'/aggregated.csv','w') as resultfile:
        cols = args.parameter[:]
        for m in measures:
            cols += [m, m+"_error"]

        writer = csv.DictWriter(resultfile, fieldnames=cols)
        headers = dict( (n,n) for n in cols )
        writer.writerow(headers)

        #for config,v in aggregated.iteritems():
        stack = list(([k],v) for k,v in aggregated.items())
        #for param in sorted(v.keys()):
        while stack:
            keys, v = stack.pop()
            print(v.keys())
            if not list(v)[0] in measures: # not yet deep enough
                stack.extend(list((keys+[k],v) for k,v in v.items()))
            else:
                result = dict(zip(args.parameter, keys))
                for k in measures: 
                    (m,h) = mean_confidence_interval(list(v[k].values()))

                    if np.isnan(m):# TODO remove
                        m = 0
                    if np.isnan(h):# TODO remove
                        h = 0
                    #h = 0 # TODO remove
                    result[k] = m
                    result[k+"_error"] = h
                writer.writerow(result)
                print(result)

    with open(args.output+'/per_host.csv','w') as resultfile:
        sets = set()
        
        perhost = {}
        for r in runs:
            for host in allhosts:
                setname = "-".join(str(r.param[p]) for p in args.parameter+['seedset'])
                sets.add(setname+"-received")
                sets.add(setname+"-lost")
                if host in r.hosts:
                    received = r.hosts[host]['sinkRcvdPk:count']
                    lost = r.hosts[host]['sentPk:count']-r.hosts[host]['sinkRcvdPk:count']
                else:
                    received = 0
                    lost = 100

                perhost.setdefault(host,{})[setname+"-received"] = received
                perhost[host][setname+"-lost"] = lost

        cols = ['address']
        cols += sorted(sets)
        #print cols

        writer = csv.DictWriter(resultfile, fieldnames=cols)
        headers = dict( (n,n) for n in cols )
        writer.writerow(headers)

        for host, v in perhost.items():
            data = {'address': host+1}
            data.update(v)
            writer.writerow(data)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-p','--parameter', help="Parameter of x-Axis", default=['sendInt'],nargs='*')
    parser.add_argument('output', help='Output directory')
    parser.add_argument('input', help='Input scalar files', nargs='+')
    args = parser.parse_args()

    main(args)
        
