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
        self.measure = {}
        self.hosts = {}
        self.host_patterns = [("trafficgen","sentPk:count"),
                              ("trafficgen","sinkRcvdPk:count"),
                              ("trafficgen","sinkRcvdPkDelay:max"),
                              ("trafficgen","sinkRcvdPkDelay:mean")]

    def load(self, file):
        with open(file) as f:
            # Read header
            attr_exp = re.compile("attr ([^ ]*) (.*)")
            for line in f:
                if len(line.strip()) == 0:
                    break
                m = attr_exp.match(line)
                if m:
                     self.param[m.group(1)] = floatifpossible(m.group(2))

            # Read body
            host_exps = [(p[1], re.compile("scalar .*\.host\[([0-9]*)\](.wrappedHost)?."+p[0]+".*"+p[1]+"(.*)")) for p in self.host_patterns]
            for line in f:
                for (name, exp) in host_exps:
                    m = exp.match(line)
                    if m:
                        host = int(m.group(1))
                        if host != 0:
                            self.hosts.setdefault(host,{})[name] = floatifpossible(m.group(3))

        for k in self.hosts.keys():
            self.hosts[k]["PRR"] = self.hosts[k]["sinkRcvdPk:count"]/self.hosts[k]["sentPk:count"]

        # minPRR
        try:
            self.measure['minPRR'] = min(list(h["PRR"] for h in self.hosts.values()))
        except ValueError:
            self.measure['minPRR'] = float('nan')

        # meanPRR
        with warnings.catch_warnings():
            warnings.filterwarnings('error')
            try:
                self.measure['meanPRR'] = np.mean(list(h["PRR"] for h in self.hosts.values()))
            except RuntimeWarning:
                self.measure['meanPRR'] = float('nan')

        # maxDelay
        try:
            self.measure['maxDelay'] = max(list(h["sinkRcvdPkDelay:max"] for h in self.hosts.values()))
        except ValueError:
            self.measure['maxDelay'] = float('nan')

        # meanDelay
        with warnings.catch_warnings():
            warnings.filterwarnings('error')
            try:
                self.measure['meanDelay'] = np.mean(list(h["sinkRcvdPkDelay:mean"] for h in self.hosts.values()))
            except RuntimeWarning:
                self.measure['meanDelay'] = float('nan')
