import ROOT as r

tf = r.TFile("root/scan1dDatasetsProb_PDF_Dataset_100p_branchingRatio.root")
tree = tf.Get("plugin")

chi2minZero   = -999.
chi2minGlobal = -999.
scanbest = -999.

insert_ev = -99
smallest_diff = 9999.
turning_point_clb = -999.

for ev in range(tree.GetEntries()):
	tree.GetEntry(ev)
	if tree.scanpoint == 0:
		chi2minZero = tree.chi2min
	chi2minGlobal = tree.chi2minGlobal
	scanbest = tree.scanbest
	if abs( tree.scanbest - tree.scanpoint ) < smallest_diff:
		smallest_diff = abs( tree.scanbest - tree.scanpoint )
		insert_ev = ev

print chi2minZero, chi2minGlobal, scanbest 

gr_sb = r.TGraph()
gr_b = r.TGraph()
gr_s = r.TGraph()

gr_clsb = r.TGraph()
gr_clb = r.TGraph()
gr_cls = r.TGraph()
gr_clsRat = r.TGraph()


p = 0
# using q to mean test statistic
for ev in range(tree.GetEntries()):
	tree.GetEntry(ev)
	
	q_b = chi2minZero - chi2minGlobal 
	q_sb = tree.chi2min - chi2minGlobal
	q_b_tilde = chi2minZero - tree.chi2min
	if q_b_tilde>0:
		q_b = q_sb
	cl_b  = r.TMath.Prob( q_b, 1 )
	cl_sb = r.TMath.Prob( q_sb, 1 )
	q_s = q_sb - q_b
	
	cl_s = r.TMath.Prob( q_s, 1 )
	
	if ev == insert_ev:
		# special case is alway met for this one event (the best fit)
		q_s = 0.
		cl_s = 1.
		gr_b.SetPoint(p, scanbest, q_b )
		gr_sb.SetPoint(p, scanbest, q_sb )
		gr_s.SetPoint(p, scanbest, q_s )
		gr_clb.SetPoint(p, scanbest, cl_b )
		gr_clsb.SetPoint(p, scanbest, cl_sb )
		gr_cls.SetPoint(p, scanbest, cl_s )
		gr_clsRat.SetPoint(p, scanbest, cl_sb/cl_b )
		p += 1

	gr_b.SetPoint(p, tree.scanpoint, q_b )
	gr_sb.SetPoint(p, tree.scanpoint, q_sb )
	gr_s.SetPoint(p, tree.scanpoint, q_s ) 
	
	gr_clb.SetPoint(p, tree.scanpoint, cl_b )
	gr_clsb.SetPoint(p, tree.scanpoint, cl_sb )
	gr_cls.SetPoint(p, tree.scanpoint, cl_s )
	gr_clsRat.SetPoint(p, tree.scanpoint, cl_sb/cl_b )
	p+=1

x = r.Double()
y = r.Double()
for p in range(gr_b.GetN()):
	gr_s.GetPoint(p,x,y)
	#print p, x, y,
	gr_cls.GetPoint(p,x,y)
	#print y

canv = r.TCanvas("c","c",1600,600)
canv.Divide(2,1)

gr_clsb.SetMarkerColor(r.kBlue)
gr_clsb.SetLineColor(r.kBlue)
gr_sb.SetMarkerColor(r.kBlue)
gr_sb.SetLineColor(r.kBlue)

gr_clb.SetMarkerColor(r.kRed)
gr_clb.SetLineColor(r.kRed)
gr_b.SetMarkerColor(r.kRed)
gr_b.SetLineColor(r.kRed)

gr_clsRat.SetMarkerColor(r.kGreen+3)
gr_clsRat.SetLineColor(r.kGreen+3)

canv.cd(1)
gr_s.GetYaxis().SetTitle( "#Delta#chi^{2}" )
leg = r.TLegend(0.6,0.6,0.89,0.89)
leg.AddEntry(gr_sb, "q_sb", "L")
leg.AddEntry(gr_b, "q_b", "L")
leg.AddEntry(gr_s, "q_s", "L" )
gr_s.Draw("ALP")
gr_sb.Draw("LPsame")
gr_b.Draw("LPsame")
leg.Draw()

canv.cd(2)
leg2 = r.TLegend(0.6,0.6,0.89,0.89)
leg2.AddEntry(gr_clsb, "CL_{S+B}", "L")
leg2.AddEntry(gr_clb, "CL_{B}", "L")
leg2.AddEntry(gr_cls, "CL_{S}", "L")
gr_cls.GetYaxis().SetTitle("p-value")
gr_cls.Draw("ALP")
gr_clsb.Draw("LPsame")
gr_clb.Draw("LPsame")
#gr_clsRat.Draw("LPsame")
leg2.Draw()

canv.Update()
canv.Modified()

canv.Print("cls.pdf")
tf.Close()

raw_input()
