#!/usr/bin/env python

from subprocess import *
import argparse
import datetime
import tempfile
import tarfile
import os

def main(args):
    if not args.force and check_output(['git','status','-s']) != "":
        print "ERROR: Working tree not clean. Please commit, stash or force with -f"
        return

    # Determine commit properties
    dtime = datetime.datetime.fromtimestamp(int(check_output(['git','log','-1','--pretty=%ct']).strip())).strftime("%Y-%m-%d-%H-%M-%S")
    sanitized_commit = check_output(['git','log','-1','--pretty=%f-%h']).strip()
    subject = check_output(['git','log','-1','--pretty=%s']).strip()
    branch = check_output(['git','rev-parse','--abbrev-ref','HEAD']).strip()
    post_file = check_output(['git','log','-1','--date=short','--pretty=%cd-%f-%h']).strip()
    output_dir = 'results/'+dtime+'_'+sanitized_commit

    # Pack up the results temporarily
    results_tar = tempfile.TemporaryFile()
    tfile = tarfile.open( fileobj=results_tar, mode='w:' )
    tfile.add( args.results, '.' )
    tfile.close()

    try:
        # Stash and checkout the jekyll branch
        check_output(['git','stash'])
        check_output(['git','checkout',args.jekyll])

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
        to_file = os.path.join('_posts',post_file+os.path.splitext(from_file)[1])
        try:
            os.rename(from_file,to_file)
        except OSError as e:
            print "ERROR: Index file %s could not be moved!"%(from_file)
            raise e

        # Add and commit
        call(['git','add',output_dir])
        call(['git','add',to_file])
        call(['git','commit','-m','Results of \''+subject+'\''])

        # Push if requested
        if not args.no_push:
            call(['git','push',args.remote,args.jekyll])

    finally:
        # Revert old working tree
        check_output(['git','checkout',branch])
        check_output(['git','stash','apply'])

if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-f','--force', action='store_true', help="Process even if working tree is not clean")
    parser.add_argument('results', help='Directory with result files')
    parser.add_argument('-i','--index', help='Markdown or Textile file inside of the results directory')
    parser.add_argument('-j','--jekyll', help='Name of the jekyll branch')
    parser.add_argument('-np','--no-push', dest='no_push', action='store_true', help="Do not push to remote")
    parser.add_argument('-r','--remote', help='Remote to push the jekyll branch to')
    parser.set_defaults(force=False,index='index.md',jekyll='gh-pages',no_push=False,remote='origin')
    args = parser.parse_args()

    original_dir = os.getcwd()
    args.results = os.path.join(original_dir,args.results)

    try:
        # Switch to root of repository
        os.chdir(check_output(['git','rev-parse','--show-toplevel']).strip())

        main(args)
    finally:
        # Switch back to original directory
        os.chdir(original_dir)
        
