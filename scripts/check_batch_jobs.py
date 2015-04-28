#!/usr/bin/env python

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-d","--dir",default="sub",help="Directory to check in. Default=%default")
parser.add_option("-s","--synch",default=False,action="store_true",help="Synch the output root files to the root directory")
(opts,args) = parser.parse_args()

import os
import fnmatch

job_dirs = []

for root, dirs, files in os.walk(opts.dir):
  if root==opts.dir: continue
  if root not in job_dirs: job_dirs.append(root) 

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

import time

if opts.synch:
  # check if back_up is required
  back_up_req = False
  for root,dirs, files in os.walk('root'):
    if root!='root': continue
    for dir in dirs:
      if 'scan' in dir:
        back_up_req = True
        break
  # do back up if needed
  if back_up_req:
    os.system('mkdir -p root/back_up')
    timestamp = int(time.time())
    os.system('mkdir -p root/back_up/%d'%timestamp)
    os.system('mv root/scan* root/back_up/%d/'%timestamp)
  
  # now link the root files
  print 'Synching files'
  nSynch = 0
  for job_dir in job_dirs:
    for root, dirs, files in os.walk(job_dir):
      match_files = fnmatch.filter(files,"*.root")
      target_dir = root.replace('sub/','root/')
      os.system('mkdir -p %s'%target_dir)
      for fil in match_files:
        target_loc = os.path.join(os.getcwd(),target_dir,fil)
        original_loc = os.path.join(os.getcwd(),root,fil)
        exec_line = 'ln -s %s %s'%(original_loc,target_loc)
        os.system(exec_line)
        nSynch += 1

  print 'Synched %d files'%nSynch
