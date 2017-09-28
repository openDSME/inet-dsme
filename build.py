#!/usr/bin/env python3

import subprocess
import argparse
import os
import shutil
import filecmp

def build(inet_path,jobs):
    # Setup required features
    myfeatures = os.path.join(os.path.dirname(__file__),'inet-oppfeaturestate')
    inetfeatures = os.path.join(inet_path,'.oppfeaturestate')
    if not os.path.isfile(inetfeatures) or not filecmp.cmp(myfeatures,inetfeatures):
        shutil.copyfile(myfeatures,inetfeatures)
    subprocess.check_call(["./inet_featuretool","repair"], cwd=inet_path)

    # Build INET
    subprocess.check_call(["make","makefiles"], cwd=inet_path)
    subprocess.check_call(["make","-j",str(jobs)], cwd=inet_path)

    # Build inet-dsme
    subprocess.check_call(["opp_makemake","-f","--deep","--make-so",
        "-I",os.path.join(inet_path,'src'),
        "-I",os.path.join(inet_path,'src/inet/common'),
        "-I",os.path.dirname(__file__),
        "-KINET_PROJ="+inet_path],
        cwd=os.path.join(os.path.dirname(__file__),'src'))
    subprocess.check_call(["make","-j",str(jobs)],
        cwd=os.path.join(os.path.dirname(__file__),'src'))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    import multiprocessing
    parser.add_argument('--inet',default=os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))),'inet'))
    parser.add_argument('--jobs',default=multiprocessing.cpu_count())
    args = parser.parse_args()
    build(args.inet,args.jobs)
