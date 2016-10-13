#!/usr/bin/env python

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-d","--dir",default="sub",help="Directory to check in. Default=%default")
parser.add_option("-s","--synch",default=False,action="store_true",help="Synch the output root files to the root directory. Default=%default")
parser.add_option("-S","--synchdir",default=None,help="Synch to this directory (if different from directory with jobs scripts e.g. if files are stored on eos. Default=%default")
parser.add_option("-r","--regex",default=None,help="Filter folders by this regex. Default=%default")
parser.add_option("-t","--date", default=None,help="Filter only folders modified after this date - format dd/mm/yyyy. Default=%default")
parser.add_option("-R","--resubmit",default=None,help="Resubmit jobs (pass Queued, Failed, Running, All). Completed jobs don't get resubmitted. Default=%default")
parser.add_option("-q","--queue",default=None,help='Queue to resubmit jobs to. Default=%default')
parser.add_option("-n","--skipBackUp",default=False,action="store_true", help="Dont backup old files. Default=%default")
parser.add_option("-c","--clearLinks",default=False,action="store_true", help="Clear old files / links out the way")
(opts,args) = parser.parse_args()

import sys
allowed_resubmits = ['Queued','Failed','All','Running']
if opts.resubmit:
  if opts.resubmit not in allowed_resubmits:
    sys.exit('--resubmit must be one of Queued, Failed, Running, All')

import os
import fnmatch
import re
import time
import datetime
regex=None
if opts.regex:
  regex = re.compile(opts.regex)

job_dirs = []

for root, dirs, files in os.walk(opts.dir):
  if root==opts.dir: continue
  if root not in job_dirs:
    include = True
    # check regex
    if regex:
      if not regex.match(root):
        include = False
    # check modified date
    if opts.date:
      d = datetime.datetime( int(opts.date.split('/')[2]), int(opts.date.split('/')[1]), int(opts.date.split('/')[0]) )
      dsecs = (d-datetime.datetime(1970,1,1)).total_seconds()
      if os.path.getmtime(root) < dsecs:
        include = False
    
    # if it passes then include it
    if include:
      job_dirs.append(root) 

# totals
total_done = []
total_fail = []
total_run  = []
total_waiting = []
total_scripts = []

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

    total_done.extend(done)
    total_fail.extend(fail)
    total_run.extend(run)
    total_waiting.extend(waiting)
    total_scripts.extend(scripts)

    resubmits = []
    if opts.resubmit:
      if opts.resubmit=='Queued' or opts.resubmit=='All':
        resubmits += waiting
      if opts.resubmit=='Failed' or opts.resubmit=='All':
        resubmits += fail
      if opts.resubmit=='Running' or opts.resubmit=='All':
        resubmits += run
    
    if opts.resubmit:
      print 'Will resubmit %d jobs:'%len(resubmits)
      for job in resubmits:
        full_path = '%s/%s/%s'%(os.getcwd(),job_dir,job.split('.sh')[0]+'.sh')
        if opts.queue:
          # should clean up old files if we resubmit
          os.system('rm -f %s.fail'%full_path)
          os.system('rm -f %s.done'%full_path)
          os.system('rm -f %s.run'%full_path)
          line = 'bsub -q %s -o %s.log %s'%(opts.queue,full_path,full_path)
          os.system(line)
        else:
          print '\t', os.path.basename( full_path ) 

# print total
print "TOTAL"
if len(total_waiting)>0: print '\tQueued:  ', len(total_waiting), '/', len(total_scripts)
if len(total_fail)>0:    print '\tFailed:  ', len(total_fail), '/', len(total_scripts)
if len(total_run)>0:     print '\tRunning: ', len(total_run), '/', len(total_scripts)
if len(total_done)>0:    print '\tComplete:', len(total_done), '/', len(total_scripts)


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
        # clear target location is requested
        if opts.clearLinks:
          os.system('rm -f %s'%target_loc)
        exec_line = 'ln -s %s %s'%(original_loc,target_loc)
        #print exec_line
        os.system(exec_line)
        nSynch += 1

  print 'Synched %d files'%nSynch
