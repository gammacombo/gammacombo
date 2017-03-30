import ROOT as r

def draw( canv, point, cl_b, cl_sb, obs ):
	canv.cd()
	cl_b[point].SetLineColor(r.kRed)
	cl_sb[point].SetLineColor(r.kBlue)
	obs[point].SetFillColor(r.kBlack)
	cl_b[point].Draw("HIST")
	cl_sb[point].Draw("HISTsame")
	obs[point].Draw("HISTsame")
	canv.Update()
	canv.Modified()


tree = r.TChain("plugin")

import os
for root, dirs, files in os.walk("root/scan1dDatasetsPlugin_PDF_Dataset_branchingRatio/"):
	for f in files:
		if 'run1' in f: continue
		tree.Add(os.path.join(root,f))

tree.Print()

scan_points = []
hists = {}
obs = {}

chi2minBkgGlobalToy = []

cl_b = {} 
cl_sb = {}

# fill background global mins
for ev in range(tree.GetEntries()):
	tree.GetEntry(ev)
	if tree.scanpoint==0:
		chi2minBkgGlobalToy.append( tree.chi2minGlobalToy )

# now run it
for ev in range(tree.GetEntries()):
	tree.GetEntry(ev)
	if tree.scanpoint not in scan_points: 
		scan_points.append(tree.scanpoint)
		hists[tree.scanpoint] =  r.TH1F( "p%d"%len(hists), "", 100, 0, 20 ) 
		obs[tree.scanpoint] = r.TH1F( "o%d"%len(hists),"",100,0,20)
		cl_b[tree.scanpoint] = r.TH1F( "cl_b%d"%len(hists),"",100,0,20)
		cl_sb[tree.scanpoint] = r.TH1F( "cl_sb%d"%len(hists),"",100,0,20)
	
	hists[tree.scanpoint].Fill ( tree.chi2minToy - tree.chi2minGlobalToy )
	obs[tree.scanpoint].Fill( tree.chi2min - tree.chi2minGlobal )

	cl_sb[tree.scanpoint].Fill( tree.chi2minToy - tree.chi2minGlobalToy )
	ntoy = tree.ntoy - 1
	if ntoy == -1: ntoy = 99
	cl_b[tree.scanpoint].Fill( tree.chi2minBkgToy - chi2minBkgGlobalToy[int(ntoy)] )


# draw for some point 20
p = 20
point = scan_points[p]

canv = r.TCanvas()
while p>=0:
	print 'Enter point to draw (anything less than 0 will exit)'
	p = int( raw_input() )
	draw( canv, scan_points[p], cl_b, cl_sb, obs )
	canv.Modified()
	canv.Update()


