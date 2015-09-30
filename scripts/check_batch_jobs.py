#!/usr/bin/env python

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-d","--dir",default="sub",help="Directory to check in. Default=%default")
parser.add_option("-s","--synch",default=False,action="store_true",help="Synch the output root files to the root directory")
parser.add_option("-S","--synchdir",default=None,help="Synch to this directory (if different from directory with jobs scripts e.g. if files are stored on eos")
parser.add_option("-r","--regex",default=None,help="Filter folders by this regex")
parser.add_option("-R","--resubmit",default=None,help="Resubmit jobs (pass Queued, Failed, All). Running and Completed jobs don't get resubmitted.")
parser.add_option("-q","--queue",default=None,help='Queue to resubmit jobs to')
parser.add_option("-n","--skipBackUp",default=False,action="store_true", help="Dont backup old files")
(opts,args) = parser.parse_args()

import sys
allowed_resubmits = ['Queued','Failed','All']
if opts.resubmit:
  if opts.resubmit not in allowed_resubmits:
    sys.exit('--resubmit must be one of Queued, Failed, All')

import os
import fnmatch
import re
regex=None
if opts.regex:
  regex = re.compile(opts.regex)

job_dirs = []

for root, dirs, files in os.walk(opts.dir):
  if root==opts.dir: continue
  if root not in job_dirs: 
    if regex:
      if regex.match(root):
        job_dirs.append(root) 
    else:
      job_dirs.append(root) 

for job_dir in job_dirs:
  done = []
  fail = []
  run  = []
  waiting = []
  scripts = []
  for root, dirs, files in os.walk(job_dir):
    match_files = fnmatch.filter(files,"*.sh")
    for fil in match_files:
      scripts.append(fil)
      if os.path.exists(os.path.join(root,fil+'.done')):
        done.append(fil+'.done')
      elif os.path.exists(os.path.join(root,fil+'.fail')):
        fail.append(fil+'.fail')
      elif os.path.exists(os.path.join(root,fil+'.run')):
        run.append(fil+'.run')
      else:
        waiting.append(fil)

    print job_dir
    if len(waiting)>0: print '\tQueued:  ', len(waiting), '/', len(scripts)
    if len(fail)>0:    print '\tFailed:  ', len(fail), '/', len(scripts)
    if len(run)>0:     print '\tRunning: ', len(run), '/', len(scripts)
    if len(done)>0:    print '\tComplete:', len(done), '/', len(scripts)

    resubmits = []
    if opts.resubmit:
      if opts.resubmit=='Queued' or opts.resubmit=='All':
        resubmits += waiting
      if opts.resubmit=='Failed' or opts.resubmit=='All':
        resubmits += fail
    
    if opts.resubmit:
      print 'Will resubmit %d jobs:'%len(resubmits)
      for job in resubmits:
        full_path = '%s/%s/%s'%(os.getcwd(),job_dir,job)
        if opts.queue:
          line = 'bsub -q %s -o %s.log %s'%(opts.queue,full_path,full_path)
          os.system(line)
        else:
          print '\t', job

import time

if opts.synch:
  # check if back_up is required
  back_up_req = False
  for root,dirs, files in os.walk('root'):
    if root!='root': continue
    for dir in dirs:
      if dir in job_dirs:
        back_up_req = True
        break
  # do back up if needed
  if back_up_req and not opts.skipBackUp:
    os.system('mkdir -p root/back_up')
    timestamp = int(time.time())
    os.system('mkdir -p root/back_up/%d'%timestamp)
    os.system('mv root/scan* root/back_up/%d/'%timestamp)
  
  # now link the root files
  print 'Synching files'
  nSynch = 0
  for job_dir in job_dirs:
    synch_dir = job_dir
    if opts.synchdir:
      synch_dir = synch_dir.replace(opts.dir,opts.synchdir)
    print synch_dir
    for root, dirs, files in os.walk(synch_dir):
      match_files = fnmatch.filter(files,"*.root")
      target_dir = root.replace(opts.synchdir,'root/')
      os.system('mkdir -p %s'%target_dir)
      for fil in match_files:
        target_loc = os.path.join(os.getcwd(),target_dir,fil)
        original_loc = os.path.join(os.getcwd(),root,fil)
        exec_line = 'ln -s %s %s'%(original_loc,target_loc)
        #print exec_line
        os.system(exec_line)
        nSynch += 1

  print 'Synched %d files'%nSynch
