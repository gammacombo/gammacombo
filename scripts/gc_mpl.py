from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument('-s','--scan', default=[], action="append", help="Scanner to pick up")
parser.add_argument('-v','--var' , default=[], action="append", help="Variables")
opts = parser.parse_args()

import numpy as np
from scipy.stats import chi2

import itertools

import os
import ROOT as r
r.gErrorIgnoreLevel = r.kFatal

angles = ['g','d_dk']

def read1dscan(h, bf, minnll):

  assert(len(bf)==1)

  x = np.array([ h.GetBinCenter(b) for b in range(1,h.GetNbinsX()+1) ])
  y = np.array([ h.GetBinContent(b) for b in range(1,h.GetNbinsX()+1) ])

  # get chi2 as DeltaChi2
  y -= minnll
  # convert to p-value
  y = chi2.sf(y,1)

  # insert the best fit
  assert(len(x)==len(y))
  index = np.argmax( x > bf )
  x = np.insert(x, index, bf )
  y = np.insert(y, index, 1.  )

  # convert to degrees
  if opts.var[0] in angles:
    x = np.degrees(x)

  return x, y, None

def read2dscan(h, bf, minnll):

  assert(len(bf)==2)

  xedges = np.array([ h.GetXaxis().GetBinLowEdge(b) for b in range(1,h.GetNbinsX()+2)])
  yedges = np.array([ h.GetYaxis().GetBinLowEdge(b) for b in range(1,h.GetNbinsX()+2)])
  xcenters = np.array([ h.GetXaxis().GetBinCenter(b) for b in range(1,h.GetNbinsX()+1)])
  ycenters = np.array([ h.GetYaxis().GetBinCenter(b) for b in range(1,h.GetNbinsX()+1)])

  # get global bins
  gbins = np.array( [ list([xbin,ybin]) for xbin, ybin in itertools.product( range(h.GetNbinsX()), range(h.GetNbinsY()) ) ] )

  # get content of each bin
  entries = np.array([ h.GetBinContent(xbin+1, ybin+1) for xbin, ybin in itertools.product( range(h.GetNbinsX()), range(h.GetNbinsY()) ) ])

  # convert to degrees
  if opts.var[0] in angles:
    xcenters = np.degrees(xcenters)
    xedges = np.degrees(xedges)
  if opts.var[1] in angles:
    ycenters = np.degrees(ycenters)
    yedges = np.degrees(yedges)

  # convert to right format
  x, y = np.meshgrid(xcenters,ycenters)
  z = entries.reshape((h.GetNbinsX(),h.GetNbinsY()))
  z = z.T - minnll

  return x,y,z

def read_gc_scan(fname):
  if not os.path.exists(fname):
    raise RuntimeError('No such file', fname)

  # get best fit point
  bf = []
  minnll = None
  pf = open(fname.replace('scanner/','par/').replace('_scanner','').replace('.root','.dat'))
  ls = pf.readlines()
  for l in ls:
    if 'SOLUTION 1' in l: break ## only read the first solution
    if l.startswith('### FCN'):
      minnll = float(l.split()[2].strip(','))
    for var in opts.var:
      if l.startswith(var+' '):
        bf.append( float(l.split()[1]) )
  pf.close()

  # get chi2 distribution
  tf = r.TFile(fname)
  h = tf.Get("hChi2min")

  if h is None:
    raise RuntimeError('No \'hChi2min\' found in', fname)

  res = None
  print(fname)
  print(h, type(h), h.InheritsFrom('TH2'))
  if h.InheritsFrom('TH2'):
    res = read2dscan(h, bf, minnll)
  elif h.InheritsFrom('TH1'):
    res = read1dscan(h, bf, minnll)
  else:
    raise RuntimeError('hchi2min does not appear to inherit from TH1')

  tf.Close()
  return res

class scan():
  def __init__(self, scanfile, lopts, fopts):
    self.x, self.y, self.z = read_gc_scan(scanfile)
    self.lopts = lopts
    self.fopts = fopts

import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, AutoMinorLocator
from matplotlib.patches import Patch
mpl.rcParams['axes.linewidth'] = 1

def setPlotStyle(fig,ax):
  # best to call this last

  ax.xaxis.set_minor_locator(AutoMinorLocator())
  ax.yaxis.set_minor_locator(AutoMinorLocator())
  ax.xaxis.set_ticks_position("both")
  ax.yaxis.set_ticks_position("both")
  ax.tick_params(axis="x", direction="in", which="minor", length=4, width=1)
  ax.tick_params(axis="y", direction="in", which="minor", length=4, width=1)
  ax.tick_params(axis="x", direction="in", which="major", length=8, width=1)
  ax.tick_params(axis="y", direction="in", which="major", length=8, width=1)

  plt.setp(ax.get_xticklabels(), fontsize=16)
  plt.setp(ax.get_yticklabels(), fontsize=16)
  xlab = ax.get_xlabel()
  ylab = ax.get_ylabel()
  ax.set_xlabel(xlab, usetex=True, fontsize=20, fontweight="bold", x=0.95, y=1)
  ax.set_ylabel(ylab, usetex=True, fontsize=20, fontweight="bold", x=1, y=0.95)
  ax.autoscale(enable=True, axis='x', tight=True)
  ax.autoscale(enable=True, axis='y', tight=True)

def addLHCbLogo(ax, x=0.8, y=0.88 ):
  ax.text(x,y,'LHCb', transform=ax.transAxes, fontsize=26, fontname="Times New Roman", fontweight="bold" )

def plot1d(fname, scans, xvar, fill=True):

  fig = plt.gcf()
  ax  = plt.gca()

  if fill:
    for scan in scans:
      ax.fill_between( scan.x, 0., scan.y, **scan.fopts )

  for scan in scans:
    ax.plot( scan.x, scan.y, **scan.lopts )

  ax.set_ylabel('1-CL')
  ax.set_xlabel(xvar)

  ax.legend(edgecolor="1", fontsize=12)
  addLHCbLogo(ax)

  setPlotStyle(fig,ax)
  fig.tight_layout()
  fig.savefig(fname+'.pdf')
  plt.show()

def plot2d(fname, scans, xvar, yvar, xlim=None, ylim=None, ncontours=2, cl2d=False):

  fig = plt.gcf()
  ax  = plt.gca()

  # just using the 1D chi2 (note this will only contain 39%, 85% etc. of distribution)
  levels = [ n**2  for n in range(ncontours+1) ]
  # if you want the 2D chi2:
  if cl2d:
    levels = [ chi2.ppf(1-chi2.sf(n**2,1),2) for n in range(ncontours+1) ]

  # line colors
  lcolors =  [  ((0.35,0.33,0.85),(0.09,0.66,0.91)),
                ((0.82,0.04,0.82),(0.84,0.00,0.99)),
                ((0.6,0.2,0.0),(0.8,0.6,0.2)),
                ((0.70,0.00,0.00),(0.90,0.20,0.20)),
                ((0.2,0.4,0.2),(0.0,0.4,0.0))
             ]

  fcolors =  [  ('#bee7fd','#d3e9ff'),
                ((0.96,0.48,1.00),(1.00,0.60,1.00)),
                ((0.90,0.78,0.60),(0.99,0.87,0.71)),
                ('#d95f02','#d95f02'),
                ((0.40,1.00,0.40),(0.40,0.80,0.40))

             ]

  leg_els = []
  for i, scan in enumerate(scans):
    # have to force the legend
    if 'colors' in scan.fopts.keys() and 'colors' in scan.lopts.keys() and ('label' in scan.fopts.keys() or 'label' in scan.lopts.keys() ):
      fc = scan.fopts['colors'][0]
      lc = scan.lopts['colors'][1]
      if 'label' in scan.fopts.keys():
        lab = scan.fopts['label']
        scan.fopts.pop('label')
      else:
        lab = scan.lopts['label']
        scan.lopts.pop('label')
      leg_els.append( Patch( facecolor=fc, edgecolor=lc, label=lab ) )

    # now make the actual plots
    ax.contourf(scan.x, scan.y, scan.z, levels, **scan.fopts)
    ax.contour (scan.x, scan.y, scan.z, levels, **scan.lopts)

  #for i, scan in enumerate(scans):
    #ax.contour(scan.x, scan.y, scan.z, levels, colors=lcolors[i], **scan.mplopts)

  ax.set_ylabel(yvar)
  ax.set_xlabel(xvar)

  ax.legend(handles=leg_els, edgecolor="1", fontsize=12, loc="upper left")
  addLHCbLogo(ax)

  setPlotStyle(fig,ax)

  if xlim: ax.set_xlim(xlim)
  if ylim: ax.set_ylim(ylim)

  fig.tight_layout()
  fig.savefig(fname+'.pdf')
  plt.show()


# 2d example
#cfg = { 'nbody'   : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+103+105+152+154+155+156_g_r_dk.root',
                      #'fopts'  : { 'label'  : r'$B^{+}\to D^{0}h^{+},\; D^{0}\to hh\pi^{0}/h3\pi$',
                                   #'colors'  : ('#bee7fd','#d3e9ff'),
                                   #'alpha'   : 0.9,
                                   #'zorder'  : 0,
                                 #},
                      #'lopts'  : { 'colors'  : ((0.35,0.33,0.85),(0.09,0.66,0.91)),
                                   #'zorder'  : 1,
                                 #}
                    #},
        #'ggsz'    : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+107_g_r_dk.root',
                      #'fopts'  : { 'label'  : r'$B^{+}\to D^{0}h^{+},\; D^{0}\to K_{S}^{0}h^{+}h^{-}$',
                                   #'colors'  : ((0.96,0.48,1.00),(1.00,0.60,1.00)),
                                   #'alpha'   : 0.9,
                                   #'zorder'  : 2,
                                 #},
                      #'lopts'  : { 'colors'  : ((0.82,0.04,0.82),(0.84,0.00,0.99)),
                                   #'zorder'  : 3,
                                 #}
                    #},
        #'2body'   : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+101+152+154+155+156+157+158_g_r_dk.root',
                      #'fopts'  : { 'label'  : r'$B^{+}\to D^{0}h^{+},\; D^{0}\to h^{+}h^{\prime-}$',
                                   #'colors'  : ((0.90,0.78,0.60),(0.99,0.87,0.71)),
                                   #'alpha'   : 0.9,
                                   #'zorder'  : 4,
                                 #},
                      #'lopts'  : { 'colors'  : ((0.6,0.2,0.0),(0.8,0.6,0.2)),
                                   #'zorder'  : 5,
                                 #}
                    #},
        #'alld'    : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+107+101+103+105+109+152+154+155+156+157+158_g_r_dk.root',
                      #'fopts'  : { 'label'  : r'All $B^{+}\to D^{0}h^{+}$ modes',
                                   #'colors'  : ('#d95f02','#d95f02'),
                                   #'alpha'   : 0.9,
                                   #'zorder'  : 6,
                                 #},
                      #'lopts'  : { 'colors'  : ((0.70,0.00,0.00),(0.90,0.20,0.20)),
                                   #'zorder'  : 7,
                                 #}
                    #},
        #'dhch'    : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_lhcb_2020_beauty_and_charm_g_r_dk.root',
                      #'fopts'  : { 'label'  : r'Combination',
                                   #'colors'  : ((0.40,1.00,0.40),(0.40,0.80,0.40)),
                                   #'alpha'   : 0.9,
                                   #'zorder'  : 8,
                                 #},
                      #'lopts'  : { 'colors'  : ((0.2,0.4,0.2),(0.0,0.4,0.0)),
                                   #'zorder'  : 9,
                                 #}
                    #}
      #}

#scans = []
#for key, options in cfg.items():
  #scanfile = options['scanner']
  #pl = scan(scanfile,options['lopts'],options['fopts'])
  #scans.append(pl)

#x = r'$\gamma\;[^\circ]$'
#y = r'$r_{B^{+}}^{D^{0}K^{+}}$'
#plot2d( 'plot2d', scans, x, y, xlim=(0,180), ylim=(0.05,0.2), ncontours=2, cl2d=True )

#s1 = scan('plots/scanner/gammacharm_lhcb_scanner_empty+103+105+152+154+155+156_g_r_dk.root')
#s2 = scan('plots/scanner/gammacharm_lhcb_scanner_empty+107_g_r_dk.root')
#s3 = scan('plots/scanner/gammacharm_lhcb_scanner_empty+101+152+154+155+156+157+158_g_r_dk.root')
#s4 = scan('plots/scanner/gammacharm_lhcb_scanner_empty+107+101+103+105+109+152+154+155+156+157+158_g_r_dk.root')
#s5 = scan('plots/scanner/gammacharm_lhcb_scanner_lhcb_2020_beauty_and_charm_g_r_dk.root')
#x = r'$\gamma\;[^\circ]$'
#y = r'$r_{B^{+}}^{D^{0}K^{+}}$'
#plot2d('plot2d', [s1,s2,s3,s4,s5], xvar=x, yvar=y, ncontours=2, cl2d=True)

#import sys
#sys.exit()

# 1D example
#cfg = { 'gamma_bs': { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+118+119+161_g.root',
                      #'fopts'  : { 'label'  : r'$B_{s}^{0}$',
                                   #'color'  : '#d95f02',
                                 #},
                      #'lopts'  : { 'color'  : '#d95f02',
                                   #'linewidth': 1
                                 #}
                    #},
        #'gamma_bd': { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+114+115+152+154+155+160_g.root',
                      #'fopts'  : { 'label'  : r'$B^{0}$',
                                   #'color'  : '#e6ab02',
                                 #},
                      #'lopts'  : { 'color'  : '#e6ab02',
                                   #'linewidth': 1
                                 #}
                    #},
        #'gamma_bu': { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_empty+108+102+104+106+110+111+113+116+152+154+155+156+157+158+159_g.root',
                      #'fopts'  : { 'label'  : r'$B^{+}$',
                                   #'color'  : '#7570b3',
                                 #},
                      #'lopts'  : { 'color'  : '#7570b3',
                                   #'linewidth': 1
                                 #}
                    #},
        #'dhch'    : { 'scanner': 'plots/scanner/gammacharm_lhcb_scanner_lhcb_2020_beauty_and_charm_g.root',
                      #'fopts'  : { 'label'  : r'Combination',
                                   #'color'  : '#1b9e77',
                                 #},
                      #'lopts'  : { 'color'  : '#1b9e77',
                                   #'linewidth': 1
                                 #}
                    #}
      #}

#scans = []
#for key, options in cfg.items():
  #scanfile = options['scanner']
  #pl = scan(scanfile,options['lopts'],options['fopts'])
  #scans.append(pl)

#plot1d( 'plot', scans, r'$\gamma\;[^\circ]$' )

#plt.show()

scans = []
for scf in opts.scan:
  pl = scan(scf, {'color':'slateblue'}, {'color':'slateblue'})
  scans.append(pl)

#plot1d( 'plot', scans, r'$\gamma\;[^\circ]$' )

#if opts
plot2d( 'plot', scans, r'$|q/p|$', r'$\phi$')

