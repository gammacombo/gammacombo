import os
import sys
import importlib
import ROOT as r
import numpy as np
import itertools
from scipy.stats import chi2
from scipy.interpolate import interp1d
import matplotlib.pyplot as plt
import matplotlib.patches as patches
plt.style.use(os.path.dirname( os.getcwd() ) + '/scripts/gc.mplstyle')
from tabulate import tabulate

def load_interval(fname):
  if not os.path.exists(fname):
    raise RuntimeError('No such file', fname)
  #print('Loading configuration from file', fname)
  spec = importlib.util.spec_from_file_location('module',fname)
  module = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(module)
  if hasattr(module,'intervals'):
    return module.intervals
  return None

def print_interval(fname, nsigma=1):
    intervals = load_interval(fname)
    print_rows = []
    for cl, cfgs in intervals.items():
        ns = 0
        conf = 0
        if cl=='0.68':
            ns = 1
        elif cl=='0.95':
            ns = 2
        elif cl=='1.00':
            ns = 3
        else:
            continue
        
        if ns>nsigma:
            continue 

        conf = f'{chi2.cdf(ns**2, 1):4.1%}'
        
        for sol in cfgs:
            print_rows.append( [ ns, conf, sol['central'], sol['neg'], sol['pos'], sol['min'], sol['max'] ] )

    print( tabulate( print_rows, headers=['nSig','cl','val','-err','+err','min','max'], floatfmt=' .1f' ) )   

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

  return x, y, None, bf


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

  return x, y, z, bf

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
  # iterate pars first to keep the right order
  for var in pars:
    for l in ls:
      if 'SOLUTION 1' in l: break ## only read the first solution
      if l.startswith('### FCN'):
        minnll = float(l.split()[2].strip(','))
      if l.startswith(var+' '):
        bf.append( float(l.split()[1]) )
  pf.close()

  if not len(bf)==len(pars):
    raise RuntimeError('Did not find a best fit value for all the parameters passed', pars, 'in', parfile)

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

def_lopts_1d = [ dict( c='steelblue' ),
                 dict( c='forestgreen' ),
                 dict( c='peru' ),
                 dict( c='hotpink' )
               ]

def_fopts_1d = [ dict( fc='skyblue' ),
                 dict( fc='darkseagreen' ),
                 dict( fc='peru' ),
                 dict( fc='hotpink' )
               ]

def_mopts_1d = [ dict( fmt='ko', ms=3 ),
                 dict( fmt='b+', ms=3 ),
                 dict( fmt='gx', ms=3 ),
                 dict( fmt='r^', ms=3 )
               ]

def_lopts_2d = [
        { 'colors' : ((0.09,0.66,0.91),(0.35,0.33,0.85))   , 'zorder': 1 },
        { 'colors' : ((0.8,0.6,0.2)   ,(0.6,0.2,0.0)   )   , 'zorder': 3 },
        { 'colors' : ((0.84,0.00,0.99),(0.82,0.04,0.82))   , 'zorder': 5 },
        { 'colors' : ((0.55,0.28,0.10),(0.60,0.30,0.10))   , 'zorder': 7 },
        { 'colors' : ((0.0,0.4,0.0)   ,(0.2,0.4,0.2)   )   , 'zorder': 9 } 
        # { 'colors' : ((0.90,0.20,0.20),(0.70,0.00,0.00))   , 'zorder': 7 },
        # { 'colors' : ((0.0,0.4,0.0)   ,(0.2,0.4,0.2)   )   , 'zorder': 9 }
               ]

def_fopts_2d = [
        { 'colors' : ('#bee7fd'       ,'#d3e9ff'       ) , 'alpha': 0.9, 'zorder': 0 },
        { 'colors' : ((0.90,0.78,0.60),(0.99,0.87,0.71)) , 'alpha': 0.9, 'zorder': 2 },
        { 'colors' : ((0.96,0.48,1.00),(1.00,0.60,1.00)) , 'alpha': 0.9, 'zorder': 4 },
        { 'colors' : ('#d95f02'       ,'#df823b'       ) , 'alpha': 0.9, 'zorder': 6 },
        { 'colors' : ((0.40,1.00,0.40),(0.40,0.80,0.40)) , 'alpha': 0.9, 'zorder': 8 }
               ]

def_mopts_2d = [
        { 'marker': 'o', 'color' : (0.09,0.66,0.91), 'zorder': 10 },
        { 'marker': 'o', 'color' : (0.8,0.6,0.2)   , 'zorder': 11 },
        { 'marker': 'o', 'color' : (0.84,0.00,0.99), 'zorder': 12 },
        { 'marker': 'o', 'color' : (0.90,0.20,0.20), 'zorder': 13 },
        { 'marker': 'o', 'color' : (0.0,0.4,0.0)   , 'zorder': 14 }

               ]

hflav_cols = {
    'b': (116,112,174),
    'r': (209,53,54),
    'g': (74,155,122),
    'p': (142,81,159),
    'y': (221,173,58),
    'lg': (117,165,57),
    'k': 'k'
}

keys = hflav_cols.keys()

for key in keys:
    item = hflav_cols[key]
    if type(item)!=str:
        newval = tuple( [ float(v)/255. for v in item] )
        hflav_cols[key] = newval

def get_lopts( nscans, lopts, dim=1 ):

    if dim==1:
        defs = def_lopts_1d
    else:
        defs = def_lopts_2d

    ret = [ None for i in range(nscans) ]
    for i in range(nscans):
        if i<len(lopts):
            ret[i] = lopts[i]
        elif i<len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default lopt for iscanner = {i}")
    return ret

def get_fopts( nscans, fopts, dim=1 ):

    if dim==1:
        defs = def_fopts_1d
    else:
        defs = def_fopts_2d

    ret = [ None for i in range(nscans) ]
    for i in range(nscans):
        if i<len(fopts):
            ret[i] = fopts[i]
        elif i<len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default fopt for iscanner = {i}")
    return ret

def get_mopts( nscans, mopts, dim=1 ):

    if dim==1:
        defs = def_mopts_1d
    else:
        defs = def_mopts_2d

    ret = [ None for i in range(nscans) ]
    for i in range(nscans):
        if i<len(mopts):
            ret[i] = mopts[i]
        elif i<len(defs):
            ret[i] = defs[i]
        else:
            raise RuntimeError(f"Can't find a default fopt for iscanner = {i}")
    return ret

def addLHCbLogo(ax, x=0.02, y=0.95, prelim=False):
    ax.text(x, y, 'LHCb', transform=ax.transAxes, fontname='Times New Roman', fontsize=24, ha="left", va="top")
    if prelim:
        ax.text(x, y-0.08, 'Preliminary', transform=ax.transAxes, fontname='Times New Roman', fontsize=14, ha="left", va="top")

def addContoursLine(ax, x=0.01, y=0.01, nlevs=2, etc=False, cl2d=False):
    text = "contours hold"
    for lev in range(nlevs):
        if cl2d:
            fc = chi2.cdf( (lev+1)**2, 1 )
        else:
            fc = chi2.cdf( (lev+1)**2, 2 )
        text += rf" {fc*100:4.1f}\%"
        if lev<nlevs-1:
            text += ","
    if etc:
        text += " etc."
    ax.text(x, y, text, transform=ax.transAxes, ha="left", va="bottom", fontsize=13, fontname='Times New Roman', color='0.5')


def plot1d( scanpoints, lopts=[], fopts=[], xtitle=None, legtitles=None, angle=False, ax=None, save=None, logo=False, prelim=False, legopts={} ):
    """ parameters
    scanpoints : 2D array-like
        - expect an array of (x,y) points for each scanner [ (x1,y1), (x2,y2), ... , (xn,yn) ]
    lopts : list of dict (default: None)
        - expect a list of dictionaries giving the line options for each scanner
    fopts : list of dict (default: None)
        - expect a list of dictionaries giving the fill options for each scanner
    """
    ax = ax or plt.gca()

    nscanners = len(scanpoints)
    fopts = get_fopts( nscanners, fopts, dim=1 )
    lopts = get_lopts( nscanners, lopts, dim=1 )

    # convert if angle
    if angle:
        scanpoints = [ [np.degrees(x), y] for x, y in scanpoints ]

    # do the fills first
    for i, (x, y) in enumerate(scanpoints):

        # plot the fill
        ax.fill_between( x, 0, y, **fopts[i] )
    
    # overlay the lines
    for i, (x, y) in enumerate(scanpoints):
        
        # plot the line
        ax.plot( x, y, **lopts[i] )
    
    # add legend 
    leg_els = []
    if legtitles:
        for i, title in enumerate(legtitles):
            lopt = dict(**lopts[i])
            fopt = dict(**fopts[i])
            if 'c' in lopt.keys():
                lopt['ec'] = lopt.pop('c')
            if 'c' in fopt.keys():
                fopt['fc'] = fopt.pop('c')
            if 'color' in lopt.keys():
                lopt['ec'] = lopt.pop('color')
            if 'color' in fopt.keys():
                fopt['fc'] = fopt.pop('color')
            leg_opts = { **lopt, **fopt, **dict(label=title) }

            leg_els.append( patches.Patch( **leg_opts ) )

        ax.legend( handles=leg_els, **legopts )
    
    # style
    ax.set_ylim(0,1)
    ax.autoscale( enable=True, axis='x', tight=True )

    # xaxis label
    if xtitle:
        ax.set_xlabel(xtitle)

    # yaxis label
    ax.set_ylabel('$p=1-CL$')
    
    # add logos
    if logo:
        addLHCbLogo(ax, prelim=prelim)

    if save:
        fig = plt.gcf()
        fig.savefig(save)
        
def plot2d( scanpoints, lopts=[], fopts=[], mopts=[], title=[None,None], levels=1, legtitles=None, angle=[False,False], ax=None, save=None, bf=None, cl2d=False, logo=False, prelim=False, contourline=False, legopts={} ):
    """ parameters
    scanpoints : 2D array-like
        - expect an array of (x,y,z) points for each scanner [ (x1,y1,z1), (x2,y2,z2), ... , (xn,yn,zn) ]
    lopts : list of dict (default: None)
        - expect a list of dictionaries giving the line options for each scanner
    fopts : list of dict (default: None)
        - expect a list of dictionaries giving the fill options for each scanner
    title : list of str
        - must be two item list or tuple of [xtitle, ytitle]
    angle : list of bool
        - must be two item list or tuple of [xangle, yangle]
    legtitles : list of str
        - list of legend titles
    levels : int or list of int or list of float
        - if int then will plot this number of sigma contours at -2DLL=1,4,9,..,N**2
        - if list of int will plot this number of sigma contours as above
        - if list of float will plot this number containing the fraction
    """
    ax = plt.gca()

    # figure out the levels
    if type(levels)==int:
        levels = [ (lev+1)**2 for lev in range(levels) ]
        if cl2d:
            levels = [ chi2.ppf( chi2.cdf(lev,1), 2 ) for lev in levels ]
    else:
        if type(levels[0])==int:
            levels = [ lev**2 for lev in levels ]
        else:
            levels = [ chi2.ppf( lev, 2 ) for lev in levels ]

    # figure out frac contained
    fc = [ chi2.cdf(lev, 2) for lev in levels ]

    nscanners = len(scanpoints)
    fopts = get_fopts( nscanners, fopts, dim=2 )
    lopts = get_lopts( nscanners, lopts, dim=2 )
    mopts = get_mopts( nscanners, mopts, dim=2 )

    # convert if angle
    if angle[0]:
        scanpoints = [ [np.degrees(x), y, z] for x, y, z in scanpoints ]
        if bf is not None:
            for i, pt in enumerate(bf):
                if pt is not None:
                    bf[i] = [np.degrees(pt[0]), pt[1] ]
    if angle[1]:
        scanpoints = [ [x, np.degrees(y), z] for x, y, z in scanpoints ]
        if bf is not None:
            for i, pt in enumerate(bf):
                if pt is not None:
                    bf[i] = [pt[0], np.degrees(pt[1])]

    # do the fills first
    for i, (x, y, z) in enumerate(scanpoints):

        # plot the fill
        ax.contourf( x, y, z, levels=[0]+levels, **fopts[i] )
    
    # overlay the lines
    for i, (x, y, z) in enumerate(scanpoints):
        
        # plot the line
        ax.contour( x, y, z, levels=levels, **lopts[i] )

    # add the best fit
    if bf is not None:
        for i, pt in enumerate(bf):
            if pt is not None:
                ax.plot( pt[0], pt[1], **mopts[i] )
    
    # add legend 
    leg_els = []
    if legtitles:
        for i, ltitle in enumerate(legtitles):
            lopt = dict(**lopts[i])
            fopt = dict(**fopts[i])
            mopt = dict(**mopts[i])
            if 'c' in lopt.keys():
                lopt['ec'] = lopt.pop('c')
            if 'color' in lopt.keys():
                lopt['ec'] = lopt.pop('color')
            if 'colors' in lopt.keys():
                c = lopt.pop('colors')
                lopt['ec'] = c[0]
            if 'c' in fopt.keys():
                fopt['fc'] = fopt.pop('c')
            if 'color' in fopt.keys():
                fopt['fc'] = fopt.pop('color')
            if 'colors' in fopt.keys():
                c = fopt.pop('colors')
                fopt['fc'] = c[0]

            leg_opts = { **lopt, **fopt, **dict(label=ltitle) }
            
            if ltitle is not None:
                leg_els.append( patches.Patch( **leg_opts ) )
        
        ax.legend( handles=leg_els, **legopts )
        
    # style
    if title[0]:
        ax.set_xlabel( title[0] )
    if title[1]:
        ax.set_ylabel( title[1] )
    
    # add logos
    if logo:
        addLHCbLogo(ax, prelim=prelim)
    if contourline:
        addContoursLine(ax, nlevs=len(levels), cl2d=cl2d)

    if save:
        fig = plt.gcf()
        fig.savefig(save)

def hflav_logo(subtitle, pos=[0.02,0.98], ax=None, scale=1):
    
    ax = ax or plt.gca()
    
    xwidth = 0.22 # width of logo in units of axis
    yratio = 0.7  # relative height of y to xwidth (in axis units)
    ysub = 0.8    # relative size of subtitle to HFLAV title
    fontsize = 16 # font size of HFLAV bit
    fontsub = 0.7 # relative size of subtitle font
    font = {'family': 'sans-serif', 'style': 'italic', 'color': 'white', 'weight': 500, 'size': scale * fontsize, 'stretch': 'condensed'}
    
    fraction = 1 - ysub / (1 + ysub)

    # black part with white HFLAV
    x = (pos[0], pos[0]+xwidth)
    y = (pos[1]-yratio*fraction*xwidth, pos[1])
    xc = (x[0]+x[1])/2
    yc = (y[0]+y[1])/2
    ax.fill( [x[0],x[1],x[1],x[0]], [y[0],y[0],y[1],y[1]], 'k', ec='k', lw=0.5, transform=ax.transAxes, clip_on=False, zorder=100 )
    ax.text( xc, yc-0.01, 'HFLAV', fontdict=font, ha='center', va='center', transform=ax.transAxes, usetex=False, clip_on=False, zorder=110 ) 

    # white part with black subtitle
    y = (pos[1]-yratio*xwidth, pos[1]-yratio*fraction*xwidth)
    yc = (y[0]+y[1])/2
    font['color'] = 'black'
    font['size'] *= fontsub
    ax.fill( [x[0],x[1],x[1],x[0]], [y[0],y[0],y[1],y[1]], 'w', ec='k', lw=0.5, transform=ax.transAxes, clip_on=False, zorder=120 )
    ax.text( xc, yc-0.005, subtitle, fontdict=font, ha='center', va='center', transform=ax.transAxes, usetex=False, clip_on=False, zorder=130 ) 

