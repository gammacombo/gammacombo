#!/usr/bin/env python

# trawl relevant combiner directories and run git commands

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-g","--git_extension",default="status",help="Extension to git command to execute")
parser.add_option("-c","--useCombinersFile",default=False,action="store_true",help="Use cmake/combiners.cmake file to get paths")
(opts,args) = parser.parse_args()

import os, sys
if not os.path.exists('cmake/combiners.cmake'):
  sys.exit('No file found at cmake/combiners.cmake so nothing to do')

def printLine(text=''):
  n_dashes = 50-len(text)
  front_dashes = n_dashes/2
  back_dashes = n_dashes-front_dashes
  to_print = ''
  for i in range(front_dashes): to_print += '-'
  to_print += text
  for i in range(back_dashes): to_print += '-'
  print to_print

def executeGitCommand(path):
  printLine()
  printLine(' In directory: %s '%path)
  printLine(' Executing: \'git %s\' '%opts.git_extension)
  printLine()
  cwd = os.getcwd()
  os.chdir(path)
  os.system('git %s'%opts.git_extension)
  os.chdir(cwd)

def findGitPaths(start_path,maxdepth=2):
  git_paths = set()
  import subprocess
  for root, dirs, files in os.walk(start_path):
    sub_path = root.replace(start_path+'/','')
    if sub_path.startswith('.'): continue
    if sub_path.count('/')>maxdepth-1: continue
    if sub_path=='': continue
    cwd = os.getcwd()
    os.chdir(root)
    path = subprocess.check_output(['git', 'rev-parse', '--show-toplevel']).strip('\n')
    if path=='': continue
    git_paths.add( path )
    os.chdir(cwd)
  return list(git_paths)

def getGitPathsFromCombiners():
  switch=False
  combiners=[]
  f = open('cmake/combiners.cmake')
  for line in f.readlines():
    if line.startswith('#'): continue
    if 'COMBINER_MODULES' in line:
      switch=True
      continue
    if switch==True and ')' in line:
      switch=False
      continue
    if switch:
      combiners.append(line.split()[0])
  f.close()
  return [os.getcwd() + combiners]


# __main__

git_paths = []
# get list of all git paths
if opts.useCombinersFile:
  git_paths = getGitPathsFromCombiners()
else:
  git_paths = findGitPaths(os.getcwd())

# now go into directories and do the git command
for combiner in git_paths:
  executeGitCommand(combiner)
