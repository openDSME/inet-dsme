#!/usr/bin/env python3

import argparse 
import subprocess
import re
from concurrent.futures import ThreadPoolExecutor

def runcmd(cmd):
    print(" ".join(cmd))
    subprocess.call(cmd)

def main(args):
    # Find out the configs and thier number of runs
    configs = []
    cmd = ["opp_run", "-a", args.config, "--repeat=1"]
    #                do not use repeat of ini file ^
    configs_output = subprocess.check_output(cmd)
    for line in configs_output.splitlines():
        m = re.search("Config (.*): ([0-9]*)",str(line))
        if m:
            config = m.group(1)
            runs = int(m.group(2))
            if config != "General":
                configs.append({'config':config, 'runs': runs})
    print(configs)

    # Build up commands
    cmds = []
    for repetition in range(0,int(args.repetitions)):
        for config in configs:
            for run in range(0,config['runs']):
                cmds.append(["opp_run",
                       "--repeat="+str(args.repetitions),
                       "--result-dir="+str(args.results),
                       "-u","Cmdenv",
                       "-r",str(run),
                       "--seed-set",str(repetition),
                       "-c",config['config'],
                       "-n", "simulations:src:"+args.inet+"/examples:"+args.inet+"/src",
                       "-l",args.inet+"/src/INET",
                       "-l","src/inet-dsme",
                       args.config])

    executor = ThreadPoolExecutor(max_workers=int(args.jobs))
    executor.map(runcmd,cmds)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('jobs', help='Number of parallel jobs')
    parser.add_argument('inet', help='Path to INET')
    parser.add_argument('results', help='Results directory')
    parser.add_argument('repetitions', help='Number of repetitions')
    parser.add_argument('config', help='.ini file')
    args = parser.parse_args()

    main(args)
