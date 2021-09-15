from argparse import ArgumentParser
parser = ArgumentParser(usage="python make_coverage_tables.py [options] [files]")
parser.add_argument("-i", dest='interactive', default=False, action="store_true", help='Interactive mode')
parser.add_argument("-o", dest='outfile'    , default='coverage', help='outfile basename')
parser.add_argument("-x","--xlabels", default=None, help="x labels")
parser.add_argument("-t","--xtitle", default=None, help="x title")
parser.add_argument("-f","--list"  , default=None, help="Read files as a list from another file")
parser.add_argument("-p","--par"   , default=None, help='Give parameter name')
parser.add_argument("-b","--bestfit", default=None, help='Add a best fit point val:file')
parser.add_argument("-l","--limit" , default=None, help='Limit for observed parameter values')
parser.add_argument("-d","--degrees", default=False, action="store_true")
parser.add_argument("files",nargs="*")
args = parser.parse_args()

from tabulate import tabulate
print_rows = []
print('')

import os
import ROOT as r
import matplotlib.pyplot as plt
plt.rcParams.update( {'font.size': 16} )
from matplotlib.colors import to_rgba
import numpy as np
from scipy.optimize import curve_fit
from scipy.stats import norm, chi2

if args.list:
  with open(args.list) as f:
    args.files = [l.strip('\n') for l in f.readlines()]

args.xfloat = False
xlabels = []
if args.xlabels:
  xlabels = args.xlabels.split(',')
  try:
    xlabels = [ float(x) for x in xlabels ]
    args.xfloat = True
  except:
    args.xfloat = False

args.bffil = None
if args.bestfit:
  arg = args.bestfit.split(':')
  args.bfval = float(arg[0])
  if len(arg)>1:
    args.bffil = ':'.join(arg[1:])

def htoxy(h):
  x = np.array( [ h.GetBinCenter(b) for b in range(1,h.GetNbinsX()+1) ] )
  y = np.array( [ h.GetBinContent(b) for b in range(1,h.GetNbinsX()+1) ] )
  return (x,y)

def step(x,y):
  x -= (x[1]-x[0])*0.5
  x = np.append(x,x[-1]+x[1]-x[0])
  y = np.append(y,y[-1])
  return (x,y)

def plots(tf):
  h_plug = tf.Get('h_pvalue_plugin')
  h_prob = tf.Get('h_pvalue_prob')
  #h_obs  = tf.Get('h_sol')
  t_res  = tf.Get('t_res')
  t_res.Draw('sol>>h_tsol','','goff')
  h_obs = r.gROOT.FindObject('h_tsol')

  x_plug, y_plug = htoxy(h_plug)
  x_prob, y_prob = htoxy(h_prob)

  f = lambda x,p0,p1: p0+x*p1

  plug_opt, _ = curve_fit(f, x_plug, y_plug, sigma=y_plug**0.5)
  prob_opt, _ = curve_fit(f, x_prob, y_prob, sigma=y_prob**0.5)

  fig,ax = plt.subplots(figsize=(8,6))
  ax.fill_between( *step(x_plug,y_plug), step="post", fc='none', ec='darkblue', hatch='//', label='Plugin')
  ax.fill_between( *step(x_prob,y_prob), step="post", fc=to_rgba('darkred',0.2), ec='darkred', label='Prob')

  x = np.linspace(0,1,100)
  ax.plot( x, f(x,*plug_opt), c='darkblue', lw=2 )
  ax.plot( x, f(x,*prob_opt), c='darkred' , ls='--', lw=2 )

  ax.set_xlim(0,1)
  ax.set_ylim(0,ax.get_ylim()[1])
  ax.legend(fontsize=16)
  ax.tick_params(axis='both',labelsize=16)
  ax.set_xlabel('p-value',fontsize=16)
  ax.set_ylabel('ntoys', fontsize=16)
  fig.tight_layout()
  fname = os.path.basename(tf.GetName()).replace('scanner_','').replace('.root','')
  fig.savefig(f'plots/pdf/{fname}_pvalue.pdf')

  fig,ax = plt.subplots(figsize=(8,6))
  x_obs, y_obs = htoxy(h_obs)

  if args.degrees: x_obs = np.degrees(x_obs)

  mean = np.average(x_obs, weights=y_obs)
  std  = (np.average( (x_obs-mean)**2, weights=y_obs))**0.5

  f = lambda x, m, s: norm.pdf(x,m,s)
  bw = (x_obs[1]-x_obs[0])
  N = np.sum(y_obs)

  ax.step( *step(x_obs,y_obs), where="post", c='darkblue' )

  if args.limit:
    xl = args.limit.split(':')
    ax.set_xlim( float(xl[0]), float(xl[1]) )
  x = np.linspace(*ax.get_xlim(),200)
  ax.plot(x, N*bw*f(x,mean,std),'r-')

  ax.text(0.7,0.7,'$\mu={:6.3f} \\pm {:5.3f}$'.format(mean,std/(N**0.5)),transform=ax.transAxes, fontsize=16)
  ax.text(0.7,0.6,'$\sigma={:5.3f} \\pm {:5.3f}$'.format(std,std/((N-1)**0.5)),transform=ax.transAxes, fontsize=16)

  ax.tick_params(axis='both',labelsize=16)
  xlab = '{0}'.format(args.par)
  if args.degrees: xlab += ' [$^\circ$]'
  ax.set_xlabel(xlab,fontsize=16)
  ax.set_ylabel('ntoys', fontsize=16)
  fig.tight_layout()
  fig.savefig(f'plots/pdf/{fname}_bestfit.pdf')

files = args.files
if args.bffil: files = [args.bffil] + args.files

for fil in files:

  tf = r.TFile(fil)

  plots(tf)

  tree = tf.Get("result_values")
  tree.GetEntry(0)

  den = tree.nentries - tree.nfailed
  for cl, exp in zip(['68','95','99'],['68.27','95.45','99.73']):
    num_prob = getattr(tree,f'n{cl}prob')
    num_plug = getattr(tree,f'n{cl}plugin')

    fr_prob = num_prob / den
    er_prob = ( fr_prob*(1-fr_prob)/den )**0.5
    rs_prob = fr_prob - float(exp)/100
    pl_prob = rs_prob / er_prob if er_prob>0 else 0

    fr_plug = num_plug / den
    er_plug = ( fr_plug*(1-fr_plug)/den )**0.5
    rs_plug = fr_plug - float(exp)/100
    pl_plug = rs_plug / er_plug if er_plug>0 else 0

    print_rows.append( [exp, fr_prob*100, er_prob*100, fr_plug*100, er_plug*100] )

    print( f'${exp:5s}$ & $({fr_prob*100:5.2f} \\pm {er_prob*100:4.2f})$ & ${rs_prob*100: 4.2f}$ & $({fr_plug*100:5.2f} \\pm {er_plug*100:4.2f})$ & ${rs_plug*100: 4.2f}$ \\\\')
  print('\\hline\\hline')
  tf.Close()

if args.xfloat:
  x = xlabels
else:
  x = np.arange(len(files)-1 if args.bffil else len(files))

print_rows = np.array(print_rows).astype(float)

fig, ax = plt.subplots(3,1,gridspec_kw={'hspace':0, 'wspace':0})


for nsig in range(1,4):
  cl = chi2.cdf(nsig**2,1)*100

  y_prob       = print_rows[nsig-1::3,1]
  y_prob_frerr = print_rows[nsig-1::3,2] / y_prob
  y_plug       = print_rows[nsig-1::3,3]
  y_plug_frerr = print_rows[nsig-1::3,4] / y_plug

  y_prob = nsig * ( y_prob / cl )
  y_prob_err = y_prob_frerr * y_prob
  y_plug = nsig * ( y_plug / cl )
  y_plug_err = y_plug_frerr * y_plug

  y_prob_pltv = y_prob
  y_prob_plte = y_prob_err
  y_plug_pltv = y_plug
  y_plug_plte = y_plug_err

  if args.bffil:
    y_prob_pltv = y_prob[1:]
    y_prob_plte = y_prob_err[1:]
    y_plug_pltv = y_plug[1:]
    y_plug_plte = y_plug_err[1:]

  # plot line
  ax[nsig-1].plot(x,np.full_like(x,nsig),'k--')

  label = 'Prob' if nsig==1 else None
  ax[nsig-1].errorbar( x, y_prob_pltv, yerr=y_prob_plte, fmt='bo', label=label)
  label = 'Plugin' if nsig==1 else None
  ax[nsig-1].errorbar( x, y_plug_pltv, yerr=y_plug_plte, fmt='rx', label=label)

  if args.bffil:
    label = 'Best Fit' if nsig==1 else None
    ax[nsig-1].errorbar( [args.bfval], [y_prob_pltv[0]], yerr=[y_prob_plte[0]], fmt='go' )
    ax[nsig-1].errorbar( [args.bfval], [y_plug_pltv[0]], yerr=[y_plug_plte[0]], fmt='gx', label=label)

  # text
  if len(x)>1:
    xtxt = x[0] + 0.2*(x[1]-x[0])
    ylim = ax[nsig-1].get_ylim()
    ytxt = nsig+0.02*np.diff(ylim)
    ax[nsig-1].text(xtxt, ytxt, '{:5.2%}'.format( chi2.cdf(nsig**2,1) ), ha='left', va='bottom', fontsize=14)

    ax[nsig-1].set_ylim( ylim[0], max(nsig+0.3*np.diff(ylim),ylim[1]) )

ax[0].tick_params(axis='x', labelsize=0)
ax[1].tick_params(axis='x', labelsize=0)
if not args.xfloat:
  ax[2].set_xticks( np.arange(len(xlabels)) )
  ax[2].set_xticklabels( xlabels )
if args.xtitle: ax[2].set_xlabel(args.xtitle)
lab = 'Coverage'
if args.par: lab += ' on {0}'.format(args.par)
lab += ' [$\sigma$]'
ax[0].set_ylabel(lab, fontsize=16 )
ax[0].legend(fontsize=14, ncol=3 if args.bffil else 2, bbox_to_anchor=(0.5,1.2),loc='center')
#fig.subplots_adjust(hspace=0,wspace=0)
fig.tight_layout()
fig.savefig('plots/pdf/{0}.pdf'.format(args.outfile))
print('Saved plot to: plots/pdf/{0}.pdf'.format(args.outfile))

print('')
print('Summary (latex format above)')
print(tabulate(print_rows,headers=['Expected','Prob','Err','Plugin','Err'],floatfmt='.1f'))
if args.interactive:
  plt.show()
