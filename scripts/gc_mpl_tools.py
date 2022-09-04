import os
import ROOT as r
import numpy as np
import itertools
from scipy.stats import chi2
from scipy.interpolate import interp1d

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

  # convert to right format
  x, y = np.meshgrid(xcenters,ycenters)
  z = entries.reshape((h.GetNbinsX(),h.GetNbinsY()))
  z = z.T - minnll

  return x,y,z, bf

def eval1DPoint(res, x):
    f = interp1d(res[0],res[1],kind='quadratic')
    return f(x)

def read_gc_scan(scanfile, parfile, pars):

  if not os.path.exists(scanfile):
    raise RuntimeError('No such file', scanfile)
  if not os.path.exists(parfile):
    raise RuntimeError('No such file', parfile)
  if len(pars)>2:
    raise RuntimeError('Cannot pass more than two parameters', pars)

  # get best fit point
  bf = []
  minnll = None
  pf = open(parfile)
  ls = pf.readlines()
  for l in ls:
    if 'SOLUTION 1' in l: break ## only read the first solution
    if l.startswith('### FCN'):
      minnll = float(l.split()[2].strip(','))
    for var in pars:
      if l.startswith(var+' '):
        bf.append( float(l.split()[1]) )
  pf.close()

  if not len(bf)==len(pars):
    raise RuntimeError('Did not find a best fit value for all the parameters passed', pars)

  # get chi2 distribution
  tf = r.TFile(scanfile)
  #tf.ls()
  h = tf.Get("hChi2min").Clone()
  #h.Print()

  if h is None:
    raise RuntimeError('No \'hChi2min\' found in', scanfile)

  if h.InheritsFrom('TH2') and len(pars)==2:
    res = read2dscan(h, bf, minnll)
  elif h.InheritsFrom('TH1'):
    res = read1dscan(h, bf, minnll)
  else:
    raise RuntimeError('hchi2min does not appear to be a TH1 or TH2')

  tf.Close()

  return res

