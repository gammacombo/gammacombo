from __future__ import print_function
import os

assert( os.path.exists( os.path.join( os.getcwd(), 'build' ) ) )
assert( os.path.exists( os.path.join( os.getcwd(), 'core' ) ) )
assert( os.path.exists( os.path.join( os.getcwd(), 'tutorial' ) ) )
os.chdir('tutorial')
if not os.path.exists('ci_logs'): os.mkdir('ci_logs')

global htmlf
htmlf = open('ci_logs/index.html','w')
htmlf.write('<font size="5"><u> GammaCombo Continous Integration Test </u></font><br>\n')
htmlf.write('<script language="Javascript"> document.write("CI checks performed on " + document.lastModified +" (UTC)"); </script> <br>\n')

### DEFINE IF CORRECT RESULT FOUND ###
def check_comb_stdout( logf ):
  f = open(logf)
  for line in f.readlines():
    if line.startswith('Fit quality'):
      assert( '1.54/(3-2), P = 21.5%' in line )
    if line.startswith('a_gaus = [') and 'Prob' in line:
      if '0.68CL' in line: assert( '1.20 -   0.46 +   0.46' in line )
      if '0.95CL' in line: assert( '1.20 -   0.93 +   0.93' in line )
  f.close()
  return True

def check_comb_dat( datf ):
  f = open(datf)
  for line in f.readlines():
    if line.startswith('a_gaus'): assert('1.200197    -0.462758     0.462758' in line)
    if line.startswith('b_gaus'): assert('2.160118    -0.846813     0.846813' in line)
  f.close()
  return True

def check_dsets_stdout( logf ):
  f = open(logf)
  for line in f.readlines():
    if line.startswith('branchingRatio = [') and 'Prob' in line and 'boundary' not in line:
      if '0.68CL' in line: assert( '0.00000010 -0.00000011 +0.00000015' in line )
      if '0.95CL' in line: assert( '0.00000010 -0.00000011 +0.00000030' in line )
  f.close()
  return True

def check_dsets_stdout_cls( logf ):
  f = open(logf)
  for line in f.readlines():
    if line.startswith('branchingRatio = [') and 'Prob' in line and 'boundary' not in line:
      if '0.68CL' in line: assert( '0.000000103 -0.000000116 +0.000000068' in line )
      if '0.95CL' in line: assert( '0.00000010 -0.00000011 +0.00000025' in line )
  f.close()
  return True

def check_dsets_dat( datf ):
  f = open(datf)
  for line in f.readlines():
    if line.startswith('Nbkg'):                assert('4990.20' in line and '-71.76' in line and '71.76' in line)
    if line.startswith('branchingRatio'):      assert('   0.000000    -0.000000     0.000000' in line)
    if line.startswith('exponent'):            assert('  -0.000977    -0.000027     0.000027' in line)
    if line.startswith('mean'):                assert('5370.000000     1.000000    -1.000000' in line)
    if line.startswith('norm_constant '):      assert('   0.000000    -0.000000     0.000000' in line)
    if line.startswith('norm_constant_sigma'): assert('   0.000000     1.000000    -1.000000' in line)
    if line.startswith('sigma'):               assert('  20.900000     1.000000    -1.000000' in line)
  f.close()
  return True

def copy_plot( inn, outn ):
  for ext in ['png','pdf']:
    inf  = 'plots/'+ext+'/'+inn+'.'+ext
    outf = 'ci_logs/'+outn+'.'+ext
    if os.path.exists(inf): os.system('cp %s %s'%(inf,outf))

def write_html_entry(cmd, plot):
  global htmlf
  htmlf.write('<div style="display:inline-block;border-style:solid;border-width:2px;color:black;width:600;word-break:break-all;word-wrap:break-al;">\n')
  htmlf.write('  <center>\n')
  htmlf.write('  </br>\n')
  htmlf.write(' <tt style="border-style:groove;border-color:black;border-width:1px"><mark style="background-color:#D0D3D4">&nbsp;%s&nbsp;</mark></tt> </br>\n'%cmd)
  htmlf.write('  <a href=\'%s.pdf\'><img width="600" src="%s.png"></a></br>\n'%(plot,plot))
  htmlf.write('  <a href=\'%s.png\'>png</a> &#160; <a href=\'%s.pdf\'>pdf</a>\n'%(plot,plot))
  htmlf.write('  </center>\n')
  htmlf.write('</div>\n')

### RUN TESTS ###
def check_comb_prob_run():
  cmd  = 'bin/tutorial -c 5 --var a_gaus --ps 1 --grouppos def:0.8'
  outn = 'comb_prob_run'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_comb_stdout('ci_logs/%s.log'%outn)
  check_comb_dat('plots/par/tutorial_tutorial5_a_gaus.dat')
  copy_plot('tutorial_tutorial5_a_gaus',outn)
  write_html_entry( cmd, outn )
  return True

def check_comb_prob_plot():
  cmd  = 'bin/tutorial -c 5 --var a_gaus --ps 1 -a plot --grouppos def:0.8'
  outn = 'comb_prob_plot'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_comb_stdout('ci_logs/%s.log'%outn)
  check_comb_dat('plots/par/tutorial_tutorial5_a_gaus.dat')
  copy_plot('tutorial_tutorial5_a_gaus',outn)
  write_html_entry( cmd, outn )
  return True

def check_comb_plugin_gen():
  cmd  = 'bin/tutorial -c 5 --var a_gaus -a pluginbatch --ntoys 5'
  outn = 'comb_plugin_gen'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_comb_stdout('ci_logs/%s.log'%outn)
  return True

def check_comb_plugin_run():
  cmd  = 'bin/tutorial -c 5 --var a_gaus -a plugin --ps 1 --grouppos def:0.8 --controlplots'
  outn = 'comb_plugin_run'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_comb_stdout('ci_logs/%s.log'%outn)
  copy_plot('tutorial_tutorial5_a_gaus_plugin',outn)
  write_html_entry( cmd, outn )
  return True

def check_comb_plugin_plot():
  cmd  = 'bin/tutorial -c 5 --var a_gaus -a plugin -a plot --ps 1 --grouppos def:0.8'
  outn = 'comb_plugin_plot'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_comb_stdout('ci_logs/%s.log'%outn)
  copy_plot('tutorial_tutorial5_a_gaus_plugin',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_build_workspace():
  os.system('bin/tutorial_dataset_build_workspace > ci_logs/dsets_build_workspace.log 2>&1')
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  return True

def check_dsets_prob_run():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 --ps 1 --npoints 20 --grouppos def:0.8'
  outn = 'dsets_prob_run'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout('ci_logs/%s.log'%outn)
  check_dsets_dat('plots/par/tutorial_dataset_PDF_Dataset_branchingRatio.dat')
  copy_plot('tutorial_dataset_branchingRatio',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_prob_plot():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a plot --ps 1 --npoints 20 --grouppos def:0.8'
  outn = 'dsets_prob_plot'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout('ci_logs/%s.log'%outn)
  check_dsets_dat('plots/par/tutorial_dataset_PDF_Dataset_branchingRatio.dat')
  copy_plot('tutorial_dataset_branchingRatio',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_prob_plot_cls():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a plot --ps 1 --npoints 20 --cls 1 --teststat 1 --grouppos def:0.8'
  outn = 'dsets_prob_plot_cls'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout_cls('ci_logs/%s.log'%outn)
  copy_plot('tutorial_dataset_branchingRatio_cls',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_plugin_gen():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a pluginbatch --ps 1 --npoints 20 --npointstoy 20 --ntoys 5'
  outn = 'dsets_plugin_gen'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout('ci_logs/%s.log'%outn)
  return True

def check_dsets_plugin_run():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a plugin --ps 1 --npoints 20 --npointstoy 20 --controlplots'
  outn = 'dsets_plugin_run'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout('ci_logs/%s.log'%outn)
  copy_plot('tutorial_dataset_branchingRatio_plugin',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_plugin_plot():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a plugin -a plot --ps 1 --npoints 20'
  outn = 'dsets_plugin_plot'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout('ci_logs/%s.log'%outn)
  copy_plot('tutorial_dataset_branchingRatio_plugin',outn)
  write_html_entry( cmd, outn )
  return True

def check_dsets_plugin_plot_cls():
  assert( os.path.exists(os.path.join(os.getcwd(),'workspace.root')) )
  cmd  = 'bin/tutorial_dataset --var branchingRatio --scanrange 0:5.e-7 -a plugin -a plot --ps 1 --npoints 20 --cls 1 --teststat 1'
  outn = 'dsets_plugin_plot_cls'
  os.system('%s > ci_logs/%s.log 2>&1'%(cmd,outn))
  check_dsets_stdout_cls('ci_logs/%s.log'%outn)
  copy_plot('tutorial_dataset_branchingRatio_plugin_cls',outn)
  write_html_entry( cmd, outn )
  return True

### __main__ ##
checks = [ check_comb_prob_run,
           check_comb_prob_plot,
           check_comb_plugin_gen,
           check_comb_plugin_run,
           check_comb_plugin_plot,
           check_dsets_build_workspace,
           check_dsets_prob_run,
           check_dsets_prob_plot,
           check_dsets_prob_plot_cls,
           check_dsets_plugin_gen,
           check_dsets_plugin_run,
           check_dsets_plugin_plot,
           check_dsets_plugin_plot_cls,
         ]

for i, ch in enumerate(checks):
  print('Running check', i+1,'/',len(checks),'"'+ch.__name__+'"')
  assert ch()
