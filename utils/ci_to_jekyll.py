#!/usr/bin/env python

import subprocess
import argparse
import datetime
import tempfile
import tarfile
import os
import sys

def printcmd(args,shell):
    print "$",
    if shell:
        print args
    else:
        print " ".join(args)

def call(args,shell=False):
    printcmd(args,shell)
    return subprocess.call(args,shell=shell)

def check_output(args,shell=False,cwd=None):
    printcmd(args,shell)
    return subprocess.check_output(args,shell=shell,cwd=cwd)

def main(args):
    # Determine commit properties
    paths = ['.']
    if args.submodule:
        paths.append(args.submodule)

    vals = {}
    for d in paths:
        vals[d] = {}
        vals[d]['datetime'] = datetime.datetime.fromtimestamp(int(check_output(['git','log','-1','--pretty=%ct'],cwd=d).strip()))
        vals[d]['date'] = vals[d]['datetime'].strftime("%Y-%m-%d")
        vals[d]['sanitized_commit'] = check_output(['git','log','-1','--pretty=%f-%h'],cwd=d).strip()
        vals[d]['subject'] = check_output(['git','log','-1','--pretty=%s'],cwd=d).strip().replace(":","&#58;")
        vals[d]['post_file'] = check_output(['git','log','-1','--date=short','--pretty=%cd-%f-%h'],cwd=d).strip()

    maxtimepath = max(vals, key = lambda v: vals[v]['datetime'])
    vals = vals[maxtimepath]

    output_dir = 'results/'+vals['date']+'-'+vals['sanitized_commit']
    branch = check_output(['git','rev-parse','--abbrev-ref','HEAD']).strip() # for checkout, so no cwd

    # Pack up the results temporarily
    results_tar = tempfile.TemporaryFile()
    tfile = tarfile.open( fileobj=results_tar, mode='w:' )
    tfile.add( args.results, '.' )
    tfile.close()

    if call(['git','config','--get','user.email']) != 0:
        call(['git','config','user.email',args.email])

    if call(['git','config','--get','user.name']) != 0:
        call(['git','config','user.name',args.name])

    if call(['git','stash','--include-untracked','--all']) != 0:
        exit(1)

    call(['git','clean','-df']) # save, since we just stashed

    try:
        # Stash and checkout the jekyll branch
        call(['git','config','remote.'+args.remote+'.fetch','refs/heads/*:refs/remotes/'+args.remote+'/*']) # required since travis clones only the current branch
        call(['git','fetch'])
        call(['git','checkout',args.jekyll])

        # Create necessary directories
        for d in ['results','_posts']:
            if not os.path.exists(d):
                os.makedirs(d)

        # Extract tar
        results_tar.flush()
        results_tar.seek(0)
        tfile = tarfile.open( fileobj=results_tar, mode='r:' )
        tfile.extractall(output_dir)
        tfile.close()

        # Move index file
        from_file = os.path.join(output_dir,args.index)
        to_file = os.path.join('_posts',vals['post_file']+os.path.splitext(from_file)[1])
        try:
            os.rename(from_file,to_file)
        except OSError as e:
            print "ERROR: Index file %s could not be moved!"%(from_file)
            raise e

        # Add and commit
        call(['git','add',output_dir])
        call(['git','add',to_file])
        print "Executing %s"%args.execute
        call(args.execute,shell=True)
        call(['git','commit','-m','Results of \''+vals['subject']+'\''])

        # Push if requested
        if not args.no_push:
            if args.ssh: # https -> ssh
                url = check_output(['git','config','--get','remote.'+args.remote+'.url']).strip()
                url = url.replace("https://","")
                url = url.replace("/",":",1)
                url = "git@"+url
                call(['git','remote','set-url',args.remote,url])

            call(['git','push','-v',args.remote,args.jekyll])
    except:
        print "Unexpected error:", sys.exc_info()[0]
        raise

    finally:
        # Revert old working tree
        call(['git','clean','-dfx'])
        call(['git','checkout','-f',branch])
        call(['git','stash','apply'])

if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('results', help='Directory with result files')
    parser.add_argument('-i','--index', help='Markdown or Textile file inside of the results directory')
    parser.add_argument('-j','--jekyll', help='Name of the jekyll branch')
    parser.add_argument('-np','--no-push', dest='no_push', action='store_true', help="Do not push to remote")
    parser.add_argument('-r','--remote', help='Remote to pull from and push to')
    parser.add_argument('-u','--name', help='Name of the commiter if not set by git config')
    parser.add_argument('-e','--email', help='E-mail of the commiter if not set by git config')
    parser.add_argument('-s','--ssh', help='Rewrite the remote url from https to ssh')
    parser.add_argument('-m','--submodule', help='Also consider this submodules when determining the commit string')
    parser.add_argument('-x','--execute', help='Additional command to execute before a commit to the jekyll branch. Can be used e.g. for git lfs.')
    parser.set_defaults(force=False,index='index.md',jekyll='gh-pages',no_push=False,remote='origin',name='ci',email='ci@localhost',ssh=True,execute='')
    args = parser.parse_args()

    original_dir = os.getcwd()
    args.results = os.path.join(original_dir,args.results)

    try:
        # Switch to root of repository
        os.chdir(check_output(['git','rev-parse','--show-toplevel']).strip())

        main(args)
    finally:
        # Switch back to original directory
        print original_dir
        os.chdir(original_dir)
        
