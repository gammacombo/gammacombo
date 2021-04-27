#!/usr/bin/env python

import sys
import ROOT as r
tf = r.TFile( sys.argv[1] )
tree = tf.Get('plugin')

nb = r.TH1F('nb','nb',40,0.4,1.7)
na = r.TH1F('na','na',40,0.4,1.7)

nb_r = r.TH1F('nb_r','nb_r',40,0.0,0.06)
na_r = r.TH1F('na_r','na_r',40,0.0,0.06)

nb2d = r.TH2F('nb2d','nb',40,0.4,1.7,40,0.,0.06)
na2d = r.TH2F('na2d','na',40,0.4,1.7,40,0.,0.06)

all_cuts = "(chi2minToy>-1e10) && (chi2minGlobalToy>-1e10) && (chi2minToy-chi2minGlobalToy > 0) && (chi2minToy < 1000) && (TMath::Prob(chi2minToy-chi2minGlobalToy,2)<1)"
better_cuts = all_cuts + " && ( (chi2minToy-chi2minGlobalToy)>(chi2min-chi2minGlobal) )"
chi2_cut = " && (chi2minToy-chi2minGlobalToy < 30 )"
all_cuts += chi2_cut
better_cuts += chi2_cut

tree.Draw("g_scan>>nb", better_cuts, "goff")
tree.Draw("g_scan>>na", all_cuts, "goff")

tree.Draw("r_dpi_scan>>nb_r", better_cuts,"goff")
tree.Draw("r_dpi_scan>>na_r", all_cuts,"goff")

tree.Draw("r_dpi_scan:g_scan>>nb2d", better_cuts,"goff")
tree.Draw("r_dpi_scan:g_scan>>na2d", all_cuts,"goff")

# na in 1D should be scaled down by the number of bins in the other dimension?

#na.Scale(1./40.)
#na_r.Scale(1./40.)

p = nb.Clone('p')
p.Sumw2()
p.Divide(na)

p_r = nb_r.Clone('p_r')
p_r.Sumw2()
p_r.Divide(na_r)

p2d = nb2d.Clone('p2d')
p2d.Sumw2()
p2d.Divide(na2d)

print tree.GetEntries()
print na.GetMaximum()
print na_r.GetMaximum()

print p.GetMaximum()
print p_r.GetMaximum()
print p2d.GetMaximum()

canv = r.TCanvas('c','c',1200,900)
canv.Divide(3,3)
canv.cd(1)
nb.Draw()
canv.cd(2)
na.Draw()
canv.cd(3)
p.Draw()
canv.cd(4)
nb_r.Draw()
canv.cd(5)
na_r.Draw()
canv.cd(6)
p_r.Draw()
canv.cd(7)
nb2d.Draw("colz")
canv.cd(8)
na2d.Draw("colz")
canv.cd(9)
p2d.Draw("colz")
#canv.cd(10)
#p2d_x = p2d.ProjectionX()
#p2d_x.Draw()
#canv.cd(11)
#p2d_y = p2d.ProjectionY()
#p2d_y.Draw()
canv.Update()
canv.Modified()

canv.Print("plots/2dtest.pdf")

raw_input()
outf = r.TFile("root/2dtestOut.root","recreate")
outf.cd()
nb.Write()
na.Write()
p.Write()
nb_r.Write()
na_r.Write()
p_r.Write()
nb2d.Write()
na2d.Write()
p2d.Write()


outf.Close()
tf.Close()

