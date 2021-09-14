#!/usr/bin/env python

#  A script which reads the PLUGIN toys
#  at a particular scan point and plots
#  the distribution of the free fit results

scanpoint = 0.005

import sys
files = sys.argv[1:]

import ROOT as r

chain = r.TChain("plugin")

for f in files:
  chain.Add( f )

print chain.GetEntries()

scan_val = -99999.
smallest_diff = 1.e10

# annoyingly need to find closet val first
for ev in range(chain.GetEntries()):
  chain.GetEntry(ev)
  diff = r.TMath.Abs( scanpoint - chain.scanpoint )
  if diff < smallest_diff:
    smallest_diff = diff
    scan_val = chain.scanpoint

print 'Scan val found: ', scan_val

full_dist = r.TH1F('full_dist','',500,0,0.1)

fit_vals = []
# now can look at the distribution for this generation point
for ev in range(chain.GetEntries()):
  chain.GetEntry(ev)
  full_dist.Fill( chain.scanbest )
  if r.TMath.Abs( scan_val - chain.scanpoint) < 1.e-6:
    fit_vals.append( chain.scanbest )

print fit_vals
print len(fit_vals)

dist = r.TH1F('dist','',100,0,0.06)

for val in fit_vals:
  dist.Fill(val)

outf = r.TFile("root/scanbest.root","RECREATE")
dist.Write()
full_dist.Write()
outf.Close()

canv = r.TCanvas()
dist.Draw('HIST')
canv.Update()
canv.Modified()
canv.Print('plots/pdf/scanbest.pdf')

canv2 = r.TCanvas()
full_dist.Draw('HIST')
canv2.Update()
canv2.Modified()
canv2.Print('plots/pdf/scanbestall.pdf')

raw_input()

