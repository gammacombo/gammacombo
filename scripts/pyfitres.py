### A script which will read the minimum and correlation matrix from the GC
### output and keep it in a python uncertainties package format

import uncertainties as u
import numpy as np
from operator import itemgetter
from scipy.stats import multivariate_normal
from scipy.stats.distributions import chi2

def strip(meas, indices=[]):
  if len(indices)>0:
    submeas = np.array(meas)[indices]
    subcorr = u.correlation_matrix(meas)[indices,:][:,indices]
    meas = u.correlated_values_norm( [ tuple( (val.n, val.s) ) for val in submeas ], subcorr )
  return meas

def read_minimum( fname, indices=[], verbose=True, with_names=False ):

  # assumed the input is a plain text file
  f = open(fname)
  lines = f.readlines()

  pars = []
  values = []

  # get the solution (use only the first one)
  for i, line in enumerate(lines):
    if line.startswith("SOLUTION"):
      # get sol numb
      soln = int(line.split()[1].replace(":",""))
      if soln>0: break

    if line.strip().startswith('Parameter'):
      p = 0
      while True:
        relline = lines[i+2+p]
        els = relline.split()
        if len(els)==0: break
        assert( int(els[0])==p )
        name = els[1]
        val = float( els[2] )
        err = float( els[4] )
        if len(els)==6:
          if els[5]=='(Deg)':
            val = np.radians( val )
            err = np.radians( err )
        pars.append( name )
        values.append( tuple( ( val, err ) ) )
        p += 1

  # get the correlation matrix
  corr_mat = np.full( (len(pars),len(pars) ), -999. )
  corr_mentioned = False
  for i, line in enumerate(lines):
    if line.strip().startswith("Correlation"):
      assert( lines[i+3].startswith('%dx%d'%(len(pars),len(pars))) )
      corr_mentioned = True
      i = i+5
      while True:
        hline = lines[i]
        ystart = int(hline.replace('|','').split()[0])
        yend   = int(hline.replace('|','').split()[-1])
        for p in range(len(pars)):
          cor_line = lines[i+2+p]
          els = cor_line.split()
          els = els[2:]
          for y, el in enumerate(els):
            corr_mat[p][ystart+y] = float(el)

        if yend == len(pars)-1:
          break
        else:
          i = i+4+len(pars)

  # if correlation was never mentioned
  if not corr_mentioned:
    corr_mat = np.identity( (len(pars), len(pars)) )

  # make sure we found the values
  for i in range(len(pars)):
    for j in range(len(pars)):
      assert( abs(corr_mat[i][j]+999.)>1e-6 )

  f.close()

  corr_values = u.correlated_values_norm( values, corr_mat )

  if len(indices)>0:
    pars = np.array(pars)[indices]
    corr_values = strip(corr_values, indices)
    corr_mat = u.correlation_matrix(corr_values)

  if verbose:
    # print values
    print('Read minimum from file',fname)
    print('  Values:')
    for p, name in enumerate(pars):
      print('    {:3d} | {:15s} {: 6.4f} +/- {:6.4f}'.format(p,name,corr_values[p].n,corr_values[p].s))

    # print correlation
    print('  Correlation:')
    prstr = '    {:3s} |'.format('')
    for i in range(len(pars)):
      prstr += ' {:^7d}|'.format(i)
    print(prstr)
    print('   ',''.join( ['-' for i in range(len(prstr)-4) ] ) )
    for i in range(len(pars)):
      prstr = '    {:3d} |'.format(i)
      for j in range(len(pars)):
        prstr += ' {: 6.4f} '.format(corr_mat[i][j])
      print(prstr)

  if with_names:
    return corr_values, pars
  else:
    return corr_values

def compare(meas1, meas2, indices=[], inter_corr=0.):

  assert( len(meas1) == len(meas2) )

  assert(len(indices)<len(meas1))

  meas1 = strip(meas1, indices )
  meas2 = strip(meas2, indices )

  assert( len(meas1) == len(meas2) )
  dim = len(meas1)

  # now we build a 2N x 2N matrix to hold the correlation between the two measurements
  off_diag = inter_corr * np.identity(dim)
  tot_corr = np.identity( 2*dim )
  tot_corr[0:dim,0:dim]         = u.correlation_matrix(meas1)
  tot_corr[dim:2*dim,dim:2*dim] = u.correlation_matrix(meas2)
  tot_corr[0:dim,dim:2*dim]     = off_diag
  tot_corr[dim:2*dim,0:dim]     = off_diag
  tot_vals = [ tuple( (val.n, val.s) ) for val in meas1 ] + [ tuple( (val.n, val.s) ) for val in meas2 ]

  all_values = u.correlated_values_norm( tot_vals, tot_corr )

  # get the differences with the correlation included

  ## METHOD 1
  duvals1 = []
  for p in range(dim):
    cm = np.array( [ [1., inter_corr], [inter_corr, 1.] ] )
    (m1, m2) = u.correlated_values_norm( [ (meas1[p].n, meas1[p].s), (meas2[p].n, meas2[p].s) ], cm )
    #print( m1 - m2 )
    duvals1.append( m1 - m2 )

  dcov1 = u.covariance_matrix( duvals1 )
  dcor1 = u.correlation_matrix( duvals1 )

  ## METHOD 2
  duvals2 = [ all_values[v] - all_values[v+dim] for v in range(dim) ]
  dcov2 = u.covariance_matrix( duvals2 )
  dcor2 = u.correlation_matrix( duvals2 )

  duvals = duvals1
  dcov   = dcov1

  # print differences
  print('Comparison differences (%dD):'%dim)
  for i, duval in enumerate(duvals):
    prstr = ''
    if len(indices)>0:
      prstr += '{:3d} | '.format(indices[i])
    else:
      prstr += '{:3d} | '.format(i)

    prstr += '{:6.4f}'.format(duval)

    print(prstr)

  # now we have the correlated delta for each obs
  # calculate the likelihood at the observation (i.e. what the minimum will be)
  dpurevals = np.array([ x.n for x in duvals ])
  nll2_min = -2.*multivariate_normal.logpdf( dpurevals, dpurevals, dcov )

  # now get the likelihood at 0
  dexpvals = np.zeros(dim)
  nll2 = -2.*multivariate_normal.logpdf( dexpvals, dpurevals, dcov )

  # likelihood delta
  dnll2 = nll2 - nll2_min

  # p-value from likelihood value (given n dim)
  prob = chi2.sf(dnll2,dim)

  # convert p-value back into significance for 1 d.o.f
  sig = np.sqrt(chi2.isf(prob,1))
  print('\t -2DLL:', dnll2)
  print('\t p-val:', prob)
  print('\t sig:  ', sig)

def point_compare(meas, point):

  assert(len(meas)==len(point))

  dim = len(meas)

  vals = [ val.n for val in meas ]
  cov = u.covariance_matrix( meas )
  cor = u.correlation_matrix( meas )

  # get nll at min
  nll2_min = -2.*multivariate_normal.logpdf( vals, vals, cov )
  # get nll at point
  nll2     = -2.*multivariate_normal.logpdf( point, vals, cov )

  #print( nll2_min, nll2 )

  # likelihood delta
  dnll2 = nll2 - nll2_min

  # p-value from likelihood value
  prob = chi2.sf(dnll2,dim)

  # convert p-value back into significance for 1 d.o.f
  sig = np.sqrt(chi2.isf(prob,1))
  print('Comparison of values to point:', point)
  print('\t -2DLL:', dnll2)
  print('\t p-val:', prob)
  print('\t sig:  ', sig)


# example test
#a = read_minimum("RunScripts/Legacy2020/glwads_2016_dh_sol.log" )
#b = read_minimum("RunScripts/Legacy2020/glwads_2020_dh_sol.log" )
#compare(a,b,[1,3,5],0.57)
#point_compare(a,[0,0])

