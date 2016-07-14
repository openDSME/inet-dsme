import re
import numpy as np
import warnings

def floatifpossible(val):
    try:
        return float(val)
    except ValueError:
        return val.strip() # not a number
    

class Run():
    def __init__(self):
        self.param = {}
        self.param['sendInt'] = None
        self.param['configname'] = None
        self.param['seedset'] = None
        self.measure = {}
        self.hosts = {}
        self.host_patterns = [("trafficgen","sentPk:count"),
                              ("trafficgen","sinkRcvdPk:count"),
                              ("trafficgen","sinkRcvdPkDelay:max"),
                              ("trafficgen","sinkRcvdPkDelay:mean")]

    def load(self, file):
        exps = map(lambda p: (p, re.compile("attr "+p+"(.*)")), self.param.keys())
        host_exps = map(lambda p: (p[1], re.compile("scalar .*\.host\[([0-9]*)\]\.wrappedHost\."+p[0]+".*"+p[1]+"(.*)")), self.host_patterns)

        with open(file) as f:
            for line in f:
                for (p, exp) in exps:
                    m = exp.match(line)
                    if m:
                        self.param[p] = floatifpossible(m.group(1))

                for (name, exp) in host_exps:
                    m = exp.match(line)
                    if m:
                        host = int(m.group(1))
                        if host != 0:
                            self.hosts.setdefault(host,{})[name] = floatifpossible(m.group(2))

        for k in self.hosts.keys():
            self.hosts[k]["PRR"] = self.hosts[k]["sinkRcvdPk:count"]/self.hosts[k]["sentPk:count"]

        # minPRR
        try:
            self.measure['minPRR'] = min(map(lambda h: h["PRR"], self.hosts.values()))
        except ValueError:
            self.measure['minPRR'] = float('nan')

        # meanPRR
        with warnings.catch_warnings():
            warnings.filterwarnings('error')
            try:
                self.measure['meanPRR'] = np.mean(map(lambda h: h["PRR"], self.hosts.values()))
            except RuntimeWarning:
                self.measure['meanPRR'] = float('nan')

        # maxDelay
        try:
            self.measure['maxDelay'] = max(map(lambda h: h["sinkRcvdPkDelay:max"], self.hosts.values()))
        except ValueError:
            self.measure['maxDelay'] = float('nan')

        # meanDelay
        with warnings.catch_warnings():
            warnings.filterwarnings('error')
            try:
                self.measure['meanDelay'] = np.mean(map(lambda h: h["sinkRcvdPkDelay:mean"], self.hosts.values()))
            except RuntimeWarning:
                self.measure['meanDelay'] = float('nan')
