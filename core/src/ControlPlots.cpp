#include "ControlPlots.h"

ControlPlots::ControlPlots(ToyTree *tt)
{
	this->tt = tt;
	arg = tt->getArg();
	t = tt->t;
	name = tt->getName();
	ctrlPadId    = 0;
	ctrlPlotCuts = "statusFree==0 && statusScan==0";
	// if ( arg->id!=-1 ) ctrlPlotCuts = ctrlPlotCuts && Form("BergerBoos_id==%i", arg->id);
	if ( arg->id!=-1 ) ctrlPlotCuts = ctrlPlotCuts && Form("id==%i", arg->id);
	if(t->GetEntries(ctrlPlotCuts)==0){
		std::cout << "\nError in ControlPlots::ControlPlots(): Prob free fit or Prob scan fit have inappropriate fit result (fit status or cov qual). Cannot do control plots." << std::endl;
		exit(1);
	}
}


ControlPlots::~ControlPlots()
{}


///
/// Make p-value control plots.
///
void ControlPlots::ctrlPlotPvalue()
{
	gStyle->SetOptStat(1111);
	TCanvas *c2 = newNoWarnTCanvas(getUniqueRootName(), name + " P-value Plots", 900, 600);
	c2->Divide(1,1);
	int ip = 1;
	TPad *pad;

	// plot 2: individual Better, Bg, All histograms
	pad = (TPad*)c2->cd(ip++);
	// better toys
	t->Draw(Form("chi2minToy-chi2minGlobalToy > (chi2min-chi2minGlobal):scanpoint>>htemp(%i,%f,%f,1,0.5,1.5)",
				tt->getScanpointN(), tt->getScanpointMin(), tt->getScanpointMax()), ctrlPlotCuts, "colz");
	TH1D* hBetter = ((TH2F*)(gPad->GetPrimitive("htemp")))->ProjectionX("hBetter",1,1);
	// background toys
	t->Draw(Form("chi2minToy-chi2minGlobalToy < -(chi2min-chi2minGlobal):scanpoint>>htemp(%i,%f,%f,1,0.5,1.5)",
				tt->getScanpointN(), tt->getScanpointMin(), tt->getScanpointMax()), ctrlPlotCuts, "colz");
	TH1D* hBg = ((TH2F*)(gPad->GetPrimitive("htemp")))->ProjectionX("hBg",1,1);
	// all toys
	t->Draw(Form("1:scanpoint>>htemp(%i,%f,%f,1,0.5,1.5)",
				tt->getScanpointN(), tt->getScanpointMin(), tt->getScanpointMax()), ctrlPlotCuts, "colz");
	TH1D* hAll = ((TH2F*)(gPad->GetPrimitive("htemp")))->ProjectionX("hAll",1,1);
	// failed toys
	TCut myCtrlPlotCuts = !ctrlPlotCuts;
	if ( arg->id!=-1 ) myCtrlPlotCuts = myCtrlPlotCuts && Form("id==%i", arg->id); // add the id cut back in as it got lost through the inversion
	//if ( arg->id!=-1 ) myCtrlPlotCuts = myCtrlPlotCuts && Form("BergerBoos_id==%i", arg->id);
	t->Draw(Form("1:scanpoint>>htemp(%i,%f,%f,1,0.5,1.5)",
				tt->getScanpointN(), tt->getScanpointMin(), tt->getScanpointMax()), myCtrlPlotCuts, "colz");
	TH1D* hFailed = ((TH2F*)(gPad->GetPrimitive("htemp")))->ProjectionX("hFailed",1,1);
	// construct nominal 1-CL histogram
	TH1D* hOmcl = (TH1D*)hBetter->Clone("hOmcl");
	hOmcl->Divide(hAll);
	// float hOmclScale = hAll->GetBinContent(hOmcl->GetMaximumBin())/hOmcl->GetMaximum(); // scale so the 1-CL curve is in units of toys
	float hOmclScale = hAll->GetMaximum()/hOmcl->GetMaximum(); // arb. units, else the plot looks bad when using --importance sampling
	hOmcl->Scale(hOmclScale);
	// construct background 1-CL histogram
	TH1D* hOmclBg = (TH1D*)hBg->Clone("hOmclBg");
	hOmclBg->Divide(hAll);
	hOmclBg->Scale(hOmclScale); //  use same scale as hOmcl
	// plot histos
	hAll->GetYaxis()->SetRangeUser(1.,hAll->GetMaximum()); // start from 1 so we can set the plot to log scale
	hAll->GetXaxis()->SetTitle("scanpoint");
	hAll->GetYaxis()->SetTitle("toys");
	hAll->SetStats(false);
	hAll->Draw();
	makePlotsNice("hAll");
	hOmcl->SetLineWidth(2);
	hOmcl->Draw("same");
	hOmclBg->SetLineColor(kRed);
	hOmclBg->Draw("same");
	hFailed->SetLineColor(kMagenta);
	hFailed->Draw("same");
	// rescale pad to have space for the legend
	pad->SetTopMargin(0.2182971);
	// add legend
	TLegend *leg = new TLegend(0.1599533,0.803442,0.9500348,0.9375);
	leg->AddEntry(hAll,    "all toys surviving cuts");
	leg->AddEntry(hFailed, "toys failing cuts");
	leg->AddEntry(hOmcl,   "1-CL of 'sig' toys (arb. units)");
	leg->AddEntry(hOmclBg, "1-CL of 'bkg' toys (same norm. as 'sig')");
	leg->SetFillStyle(0);
	leg->Draw();
	c2->Update();

	// // plot 6: log version of bg subtracted 1-CL plot
	// pad = (TPad*)c2->cd(ip++);
	// hBetter = (TH1D*)hBetter->Clone("hBetter2");
	// hAll = (TH1D*)hAll->Clone("hAll2");
	// hBg = (TH1D*)hBg->Clone("hBg2");
	// hBetter->Add(hBg, -1.);
	// hAll->Add(hBg, -1.);
	// hBetter->Divide(hAll);
	// TH1D* oneMinusCl = hBetter;
	// oneMinusCl->GetXaxis()->SetTitle("scanpoint");
	// oneMinusCl->GetYaxis()->SetTitle("p-value");
	// oneMinusCl->Draw();
	// pad->SetLogy();
	// c2->Update();
}

///
/// Make chi2 summary control plots.
///
void ControlPlots::ctrlPlotChi2()
{
	gStyle->SetOptStat(1111);
	TCanvas *c2 = newNoWarnTCanvas(getUniqueRootName(), name + " Chi2 Plots", 900, 600);
	c2->Divide(3,2);
	int ip = 1;
	TPad *pad;

	// get maximum chi2 to be plotted
	pad = (TPad*)c2->cd(ip);
	t->Draw("chi2minToy", ctrlPlotCuts && "abs(chi2minToy)<1000");
	float maxPlottedChi2 = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmax();
	maxPlottedChi2 = TMath::Min(maxPlottedChi2, (float)75.);
	// plot 1: 2D plot of chi scan vs. chi2 global
	pad = (TPad*)c2->cd(ip++);
	t->Draw(Form("chi2minToy:chi2minGlobalToy>>htemp1(75,0,%f,75,0,%f)",
				maxPlottedChi2,maxPlottedChi2), ctrlPlotCuts, "colz");
	((TH2F*)(gPad->GetPrimitive("htemp1")))->GetYaxis()->SetTitle("#chi^{2} scan");
	((TH2F*)(gPad->GetPrimitive("htemp1")))->GetXaxis()->SetTitle("#chi^{2} free");
	makePlotsNice("htemp1");
	pad->SetLogz();
	c2->Update();

	// plot 2:  chi2 distribution of the SCAN fit
	pad = (TPad*)c2->cd(ip++);
	t->Draw("chi2minToy", ctrlPlotCuts && Form("chi2minToy-chi2minGlobalToy>0 && chi2minToy<%f",maxPlottedChi2));
	((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->SetTitle("#chi^{2} scan");
	((TH1F*)(gPad->GetPrimitive("htemp")))->GetYaxis()->SetTitle("toys");
	makePlotsNice();
	c2->Update();

	// plot 3: empty
	pad = (TPad*)c2->cd(ip++);

	// plot 4: chi2 distribution of the FREE fit
	pad = (TPad*)c2->cd(ip++);
	t->Draw("chi2minGlobalToy>>hChi2free", ctrlPlotCuts
			&& Form("chi2minToy-chi2minGlobalToy>0 && chi2minGlobalToy<%f",maxPlottedChi2));
	TH1F* hChi2free = (TH1F*)(gPad->GetPrimitive("hChi2free"));
	// better toys
	t->Draw(Form("chi2minToy-chi2minGlobalToy > (chi2min-chi2minGlobal):scanpoint>>htemp(%i,%f,%f,1,0.5,1.5)",
				tt->getScanpointN(), tt->getScanpointMin(), tt->getScanpointMax()), ctrlPlotCuts, "colz");
	TH1D* hBetter = ((TH2F*)(gPad->GetPrimitive("htemp")))->ProjectionX("hBetter",1,1);
	// add the chi2 distribtion at the best fit value
	t->Draw("chi2minGlobalToy>>hChi2BestFit", ctrlPlotCuts
			&& Form("chi2minToy-chi2minGlobalToy>0 && chi2minGlobalToy<%f",maxPlottedChi2)
			&& Form("%f<scanpoint && scanpoint<%f",
				hBetter->GetBinCenter(hBetter->GetMaximumBin()-1),
				hBetter->GetBinCenter(hBetter->GetMaximumBin()+1)));
	TH1F* hChi2BestFit = (TH1F*)(gPad->GetPrimitive("hChi2BestFit"));
	// draw first distribution
	hChi2free->GetXaxis()->SetTitle("#chi^{2} free");
	hChi2free->GetYaxis()->SetTitle("toys");
	hChi2free->Draw();
	// move first stat box a little
	gPad->Update(); //  needed else FindObject() returns a null pointer
	TPaveStats *st = (TPaveStats*)hChi2free->FindObject("stats");
	st->SetName("hChi2freeStats");
	st->SetX1NDC(0.7778305); st->SetY1NDC(0.4562937);
	st->SetX2NDC(0.9772986); st->SetY2NDC(0.6056235);
	// draw second distribution
	hChi2BestFit->Scale(hChi2free->GetMaximum()/hChi2BestFit->GetMaximum()); // scale to same maximum
	hChi2BestFit->SetLineColor(kRed);
	hChi2BestFit->Draw("sames"); // s adds a second stat box
	// move second stat box a little
	gPad->Update();
	st = (TPaveStats*)hChi2BestFit->FindObject("stats");
	st->SetX1NDC(0.7778305); st->SetY1NDC(0.6274767);
	st->SetX2NDC(0.9772986); st->SetY2NDC(0.7877331);
	st->SetLineColor(kRed);
	makePlotsNice("hChi2free");
	// add legend
	TLegend *leg4 = new TLegend(0.5,0.8023019,0.9772986,0.9370629);
	leg4->AddEntry(hChi2free,    "#chi^{2} (all scan var values)");
	leg4->AddEntry(hChi2BestFit, "#chi^{2} (at best fit value)");
	leg4->SetFillStyle(1001);
	leg4->Draw();
	c2->Update();

	// plot 5: delta chi2
	pad = (TPad*)c2->cd(ip++);
	// good toys
	t->Draw("chi2minToy-chi2minGlobalToy",
			ctrlPlotCuts && Form("chi2minToy-chi2minGlobalToy>=0 && chi2minToy<%f && chi2minGlobalToy<%f", maxPlottedChi2, maxPlottedChi2));
	TH1F* h4sig = (TH1F*)(gPad->GetPrimitive("htemp"))->Clone("h4sig");
	// possibly background
	int nBkg = t->Draw("-(chi2minToy-chi2minGlobalToy)",
			ctrlPlotCuts && Form("chi2minToy-chi2minGlobalToy<0 && chi2minToy<%f && chi2minGlobalToy<%f", maxPlottedChi2, maxPlottedChi2));
	TH1F* h4bkg = (TH1F*) h4sig->Clone("h4bkg");
	if (nBkg !=0) h4bkg = (TH1F*)(gPad->GetPrimitive("htemp"))->Clone("h4bkg");
	if ( nBkg==0 ) h4bkg->Scale(0); // if no bkg events the htemp from the signal Draw gets cloned again!
	h4sig->Draw();
	h4sig->GetXaxis()->SetTitle("#Delta#chi^{2} scan-free");
	h4sig->GetYaxis()->SetTitle("toys");
	makePlotsNice("h4sig");
	h4bkg->SetLineColor(kRed);
	h4bkg->Draw("same");
	pad->SetLogy();
	// move first stat box a little
	gPad->Update(); //  needed else FindObject() returns a null pointer
	st = (TPaveStats*)h4sig->FindObject("stats");
	st->SetX1NDC(0.7778305); st->SetY1NDC(0.4562937);
	st->SetX2NDC(0.9772986); st->SetY2NDC(0.6056235);
	// add legend
	TLegend *leg5 = new TLegend(0.5,0.8023019,0.9772986,0.9370629);
	leg5->AddEntry(h4sig, "#Delta#chi^{2} of 'signal' toys");
	leg5->AddEntry(h4bkg, "#Delta#chi^{2} of 'bg' toys");
	leg5->SetFillStyle(1001);
	leg5->Draw();
	c2->Update();

	// plot 6: chi2 p-value distribution
	pad = (TPad*)c2->cd(ip++);
	// good toys
	int ndof = arg->var.size();
	t->Draw(Form("TMath::Prob(chi2minToy-chi2minGlobalToy,%i)", ndof),
			ctrlPlotCuts && Form("chi2minToy-chi2minGlobalToy>=0 && chi2minToy<%f && chi2minGlobalToy<%f", maxPlottedChi2, maxPlottedChi2));
	TH1F* h5sig = (TH1F*)(gPad->GetPrimitive("htemp"))->Clone("h5sig");
	h5sig->SetMaximum(h5sig->GetMaximum()*1.3);
	h5sig->Draw();
	h5sig->GetXaxis()->SetTitle("p(#Delta#chi^{2} scan-free)");
	h5sig->GetYaxis()->SetTitle("toys");
	makePlotsNice("h5sig");
	// move stat box a little
	gPad->Update(); //  needed else FindObject() returns a null pointer
	st = (TPaveStats*)h5sig->FindObject("stats");
	st->SetX1NDC(0.7778305); st->SetY1NDC(0.4562937);
	st->SetX2NDC(0.9772986); st->SetY2NDC(0.6056235);
	c2->Update();
	// add legend
	TLegend *leg6 = new TLegend(0.5,0.8023019,0.9772986,0.9370629);
	leg6->AddEntry(h5sig, Form("Prob(#Delta#chi^{2}, ndof=%i)",ndof));
	leg6->SetFillStyle(1001);
	leg6->Draw();

	ctrlPlotCanvases.push_back(c2);
}

///
/// Plot all fit results of the nuisances against
/// the scan variable.
/// Cuts are defined in the constructor (ctrlPlotCuts).
///
void ControlPlots::ctrlPlotNuisances()
{
	selectNewCanvas("Nuisances 1");
	vector<TString> usedVariableNames;

	int nBinsX = 50;
	int nBinsY = tt->getScanpointN()/2;

	for ( int j=0; j<t->GetListOfBranches()->GetEntries(); j++)
	{
		TString bName = ((TBranch*)t->GetListOfBranches()[0][j])->GetName();
		if ( ! (bName.EndsWith("_start")||bName.EndsWith("_scan")||bName.EndsWith("_free")) ) continue;

		TString bBaseName = bName;
		bBaseName.ReplaceAll("_start","");
		bBaseName.ReplaceAll("_scan","");
		bBaseName.ReplaceAll("_free","");

		bool nameWasUsed = false;
		for ( unsigned int i=0; i<usedVariableNames.size(); i++ ) if ( usedVariableNames[i]==bBaseName ) nameWasUsed = true;
		if ( nameWasUsed ) continue;
		usedVariableNames.push_back(bBaseName);

		TString varScan = bBaseName+"_scan";
		TString varFree = bBaseName+"_free";
		TString varStart = bBaseName+"_start";

		float customRangeLo = 0.0; //  Customize histogram range. Default will be the Draw() automatic
		float customRangeHi = 0.0; //  range. Anything outside this range will show in the overflow bins.

		if ( ( bName.BeginsWith("d_") || bName.BeginsWith("g") ) //  pi symmetry is only in the B strong phases!
				&& !( bName.BeginsWith("dD")) )
		{
			cout << "\nControlPlots::ctrlPlotNuisances() : WARNING : folding everything into the range [0,pi]. This is a remnant of the LHCb gamma combination.\n" << endl;
			varScan = "fmod("+varScan+",3.14152)";
			varFree = "fmod("+varFree+",3.14152)";
			varStart = "fmod("+varStart+",3.14152)";
			customRangeLo = 0.0;  customRangeHi = 3.14152;
		}

		gStyle->SetOptStat(10000); //  print overflow bins!
		float spmin = tt->getScanpointMin() - 0.01*(tt->getScanpointMax()-tt->getScanpointMin()); //  add some offset so that
		float spmax = tt->getScanpointMax() + 0.01*(tt->getScanpointMax()-tt->getScanpointMin()); //  the first/last scanpoint is also plotted

		{
			selectNewPad();
			if (arg->debug) cout << "ControlPlots::ctrlPlotNuisances() : plotting " << varScan << endl;
			t->Draw("scanpoint:"+varScan, ctrlPlotCuts, "colz");  // first test plot to get the automatic x axis range
			float xmin = customRangeLo==customRangeHi ? ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmin() : customRangeLo;
			float xmax = customRangeLo==customRangeHi ? ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmax() : customRangeHi;
			t->Draw("scanpoint:"+varScan+">>"
					+Form("hScan%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,spmin,spmax),
					ctrlPlotCuts, "colz");
			TH2F *hScan = ((TH2F*)(gPad->GetPrimitive(Form("hScan%i",j))));
			t->Draw("scanpoint:"+varStart+">>"
					+Form("hStart%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,spmin,spmax),
					ctrlPlotCuts, "box");
			TH2F *hStart = ((TH2F*)(gPad->GetPrimitive(Form("hStart%i",j))));
			gStyle->SetOptTitle(0);
			hScan->Draw("colz");
			hScan->GetXaxis()->SetTitle(varScan);
			hScan->GetYaxis()->SetTitle("scan point");
			hStart->Draw("boxsame");
			makePlotsNice(Form("hScan%i",j));
			updateCurrentCanvas();
		}
		{
			selectNewPad();
			if (arg->debug) cout << "ControlPlots::ctrlPlotNuisances() : plotting " << varFree << endl;
			t->Draw("scanpoint:"+varFree, ctrlPlotCuts, "colz");  // first test plot to get the automatic x axis range
			float xmin = customRangeLo==customRangeHi ? ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmin() : customRangeLo;
			float xmax = customRangeLo==customRangeHi ? ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmax() : customRangeHi;
			t->Draw("scanpoint:"+varFree+">>"
					+Form("hFree%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,spmin,spmax),
					ctrlPlotCuts, "colz");
			TH2F *hFree = ((TH2F*)(gPad->GetPrimitive(Form("hFree%i",j))));
			t->Draw("scanpoint:"+varStart+">>"
					+Form("hStart2%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,spmin,spmax),
					ctrlPlotCuts, "box");
			TH2F *hStart = ((TH2F*)(gPad->GetPrimitive(Form("hStart2%i",j))));
			gStyle->SetOptTitle(0);
			hFree->Draw("colz");
			hFree->GetXaxis()->SetTitle(varFree);
			hFree->GetYaxis()->SetTitle("scan point");
			hStart->Draw("boxsame");
			makePlotsNice(Form("hFree%i",j));
			updateCurrentCanvas();
		}
	}
}

///
/// Plot all observables against the scan variable.
/// Cuts are defined in the constructor (ctrlPlotCuts).
/// Overlay the theory parameters, which is where the toys
/// where generated.
///
void ControlPlots::ctrlPlotObservables()
{
	selectNewCanvas("Observables 1");
	for ( int j=0; j<t->GetListOfBranches()->GetEntries(); j++)
	{
		TString bName = ((TBranch*)t->GetListOfBranches()[0][j])->GetName();
		if ( ! bName.Contains("obs") ) continue;
		TString bBaseName = bName;
		bBaseName.ReplaceAll("_obs","");
		if (arg->debug) cout << "ControlPlots::ctrlPlotObservables() : plotting " << bBaseName << endl;
		int nBinsX = 50;
		int nBinsY = tt->getScanpointN()/2;
		selectNewPad();

		// observables
		t->Draw("scanpoint:"+bName,  ctrlPlotCuts, "colz");  // first test plot to get the automatic x axis range
		float xmin = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmin();
		float xmax = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmax();
		t->Draw("scanpoint:"+bName+">>"
				+Form("hObs%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,tt->getScanpointMin(),tt->getScanpointMax()),
				ctrlPlotCuts, "colz");
		TH2F *hObs = ((TH2F*)(gPad->GetPrimitive(Form("hObs%i",j))));

		// overlay theory (the branches have the same name but with th instead of obs)
		TString thName = bName;
		thName.ReplaceAll("_obs","_th");
		t->Draw("scanpoint:"+thName+">>"
				+Form("hTh%i(%i,%f,%f,%i,%f,%f)",j,nBinsX,xmin,xmax,nBinsY,tt->getScanpointMin(),tt->getScanpointMax()),
				ctrlPlotCuts, "box");
		TH2F *hTh = ((TH2F*)(gPad->GetPrimitive(Form("hTh%i",j))));

		gStyle->SetOptTitle(0);
		hObs->Draw("colz");
		hObs->GetXaxis()->SetTitle(bName);
		hObs->GetYaxis()->SetTitle("scan point");
		hTh->Draw("boxsame");
		makePlotsNice(Form("hObs%i",j));
		updateCurrentCanvas();
	}
}

///
/// Plot the deltaChi2 distribution of the toys in bins of the scan point, compare
/// to the Gaussian assumption (i.e. overlay a chi2 distribution with
/// 1 nodf).
///
void ControlPlots::ctrlPlotChi2Distribution()
{
	int nBins = 12; // this many chi2 plots we want
	float scanpointMin = tt->getScanpointMin();
	float scanpointMax = tt->getScanpointMax();
	if ( scanpointMin==scanpointMax ) nBins=1;  // else we get 12x the same bin
	selectNewCanvas("Chi2Distribution 1");
	for ( int i=0; i<nBins; i++ )
	{
		TVirtualPad *pad = selectNewPad();
		float binMin = scanpointMin+(float)i*(scanpointMax-scanpointMin)/(float)nBins;
		float binMax = binMin+(scanpointMax-scanpointMin)/(float)nBins;
		TCut bincut = Form("%f<scanpoint && scanpoint<%f", binMin*0.999, binMax*1.001); // factors to allow for the case of binMin=binMax
		float normEvents = t->Draw("chi2minToy-chi2minGlobalToy", ctrlPlotCuts && bincut && "chi2minToy-chi2minGlobalToy>0 && chi2minToy-chi2minGlobalToy<50");
		if ( !gPad->GetPrimitive("htemp") ) continue;
		TPaveText* txt = new TPaveText(0.3,0.8,0.9,0.9,"BRNDC");
		txt->AddText(Form("%.3f < %s < %.3f", binMin, arg->var[0].Data(), binMax));
		txt->SetBorderSize(0);
		txt->SetFillStyle(0);
		txt->SetTextAlign(12);
		txt->Draw();
		((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->SetTitle("#Delta#chi^{2}");
		makePlotsNice();
		pad->SetLogy();
		// draw a chi2 function
		TF1 *f = new TF1("f", "[0]*x^([1]/2-1)*exp(-x/2)", 0, 30);
		float binWidth = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetBinWidth(1);
		int ndof = arg->var.size();
		float norm = 1./(pow(2,ndof/2.)*TMath::Gamma(ndof/2.)) * normEvents*binWidth;
		f->SetParameter(0,norm);
		f->SetParameter(1,ndof);
		f->Draw("same");
		updateCurrentCanvas();
	}
}

///
/// Plot deltaChi2 of the toys versus the scan variable.
///
void ControlPlots::ctrlPlotChi2Parabola()
{
	if ( arg->debug ) cout << "ControlPlots::ctrlPlotChi2Parabola() : plotting ..." << endl;
	int nBins = 12;       //  this many chi2 plots we want
	selectNewCanvas("Chi2Parabola 1");
	float scanpointMin = tt->getScanpointMin();
	float scanpointMax = tt->getScanpointMax();
	if ( scanpointMin==scanpointMax ) nBins=1;  // else we get 12x the same bin
	gEnv->SetValue("Hist.Binning.2D.x",75);
	gEnv->SetValue("Hist.Binning.2D.y",75);

	TString plotExpression = "chi2minToy-chi2minGlobalToy:";
	if ( tt->isWsVarAngle(arg->var[0]) ){
		plotExpression += "fmod(scanbest-scanpoint,3.142)";
		// plotExpression += "fmod(scanbest-scanpoint,6.283)";
	}
	else {
		plotExpression += "scanbest-scanpoint";
		// plotExpression += "scanbest";
		// plotExpression += "a_gaus_obsUID0";
	}

	for ( int i=0; i<nBins; i++ ){
		selectNewPad();
		float binMin = scanpointMin+(float)i*(scanpointMax-scanpointMin)/(float)nBins;
		float binMax = binMin+(scanpointMax-scanpointMin)/(float)nBins;
		TCut bincut = Form("%f<scanpoint && scanpoint<=%f", binMin*0.999, binMax*1.001);  // factors to allow for the case of binMin=binMax
		t->Draw(plotExpression, ctrlPlotCuts && bincut && "chi2minToy-chi2minGlobalToy>0 && chi2minToy-chi2minGlobalToy<9", "colz");
		if ( !gPad->GetPrimitive("htemp") ) continue;
		TPaveText* txt = new TPaveText(0.3,0.8,0.9,0.9,"BRNDC");
		txt->AddText(Form("%.3f<var<%.3f", binMin, binMax));
		txt->SetBorderSize(0);
		txt->SetFillStyle(0);
		txt->SetTextAlign(12);
		txt->Draw();
		makePlotsNice();
		updateCurrentCanvas();
	}
}

///
/// Some more control plots.
///
void ControlPlots::ctrlPlotMore(MethodProbScan* profileLH)
{
	selectNewCanvas("MorePlots 1");

	// create a new TTree that contains the profile likelihood
	// chi2 so we can compare
	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : creating a new TTree that also contains the pll chi2 ..." << endl;
	TFile *fDummy = new TFile("/tmp/"+getUniqueRootName(),"recreate"); //  dummy file so the new tree is not memory resident
	TTree *tNew = new TTree("tNew", "tNew");
	float tNew_scanpoint = 0.;
	float tNew_chi2min = 0.;
	float tNew_chi2minPLH = 0.;
	tNew->Branch("scanpoint",        &tNew_scanpoint,        "scanpoint/F");
	tNew->Branch("chi2min",          &tNew_chi2min,          "chi2min/F");
	tNew->Branch("chi2minPLH",       &tNew_chi2minPLH,       "chi2minPLH/F");
	t->SetBranchStatus("*", 0); // speed up tree reading. Perhaps we need ".*" in certain root versions?
	t->SetBranchStatus("scanpoint", 1);
	t->SetBranchStatus("chi2min", 1);
	Long64_t nentries = t->GetEntries();
	float printFreq = nentries>101 ? 100 : nentries;  // for the status bar
	for (Long64_t j = 0; j < nentries; j++){
		// status bar
		if ( arg->debug && (((int)j % (int)(nentries/printFreq)) == 0) ) {
			cout << "ControlPlots::ctrlPlotMore() : reading toys " << Form("%.0f",(float)j/(float)nentries*100.) << "%   \r" << flush;
		}
		tt->GetEntry(j);
		int iBin = profileLH->getHchisq()->FindBin(tt->scanpoint);
		tNew_chi2minPLH = profileLH->getHchisq()->GetBinContent(iBin);
		tNew_scanpoint = tt->scanpoint;
		tNew_chi2min = tt->chi2min;
		tNew->Fill();
	}
	t->SetBranchStatus("*", 1); // perhaps we need ".*" in certain root versions?
	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : creating new TTree done.                      " << endl; // extra spaces for the above \r

	// make control plots
	// selectNewPad();
	// t->Draw("scanpoint:chi2minToy", "abs(chi2minToy)<25");
	// makePlotsNice("htemp", "");

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 1 ...\r" << flush;
	selectNewPad();
	t->Draw("scanpoint:chi2minToy", "abs(chi2minToy)<25", "colz");
	makePlotsNice();

	// selectNewPad();
	// t->Draw("scanbest:chi2minToy", "abs(chi2minToy)<25");
	// makePlotsNice("htemp", "");

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 2 ...\r" << flush;
	selectNewPad();
	RooRealVar *scanvar = profileLH->getScanVar1();
	float svmin = scanvar->getMin("scan");
	float svmax = scanvar->getMax("scan");
	t->Draw("scanbest:chi2minToy", Form("abs(chi2minToy)<25 && %f<scanbest && scanbest<%f",svmin,svmax), "colz");
	makePlotsNice();

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 3 ...\r" << flush;
	selectNewPad();
	tNew->Draw("scanpoint:chi2min", "", "colz");
	makePlotsNice();
	tNew->Draw("scanpoint:chi2minPLH", "", "boxsame");
	if (gPad->GetPrimitive("Graph")) ((TGraph*)(gPad->GetPrimitive("Graph")))->SetMarkerColor(kRed);

	// selectNewPad();
	// t->Draw("scanpoint:chi2min", "", "colz");
	// makePlotsNice();

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 4 ...\r" << flush;
	selectNewPad();
	t->Draw("chi2min:nrun", "", "colz");
	makePlotsNice();

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 5 ...\r" << flush;
	selectNewPad()->SetRightMargin(0.1);;
	tNew->Draw("scanpoint:chi2min-chi2minPLH", "", "colz");
	makePlotsNice();

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plot 6 ...\r" << flush;
	selectNewPad();
	t->Draw("chi2minGlobal:nrun", "", "colz");
	makePlotsNice();
	// draw a horizontal red line at the chi2minGlobal of the current PLH scan
	float xmin = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmin();
	float xmax = ((TH1F*)(gPad->GetPrimitive("htemp")))->GetXaxis()->GetXmax();
	TLine *l = new TLine(xmin,profileLH->getChi2minGlobal(),xmax,profileLH->getChi2minGlobal());
	l->SetLineColor(kRed);
	l->Draw();

	if ( arg->debug ) cout << "ControlPlots::ctrlPlotMore() : making plots done.        " << endl;
	delete tNew;
}


TCanvas* ControlPlots::selectNewCanvas(TString title)
{
	title.ReplaceAll(name+" ","");
	TCanvas* c1 = newNoWarnTCanvas(getUniqueRootName(), name+" "+title, 1200, 900);
	c1->Divide(4,3);
	ctrlPlotCanvases.push_back(c1);
	ctrlPadId = 0;
	return c1;
}


TVirtualPad* ControlPlots::selectNewPad()
{
	TCanvas* c1 = ctrlPlotCanvases[ctrlPlotCanvases.size()-1];

	if ( ctrlPadId>=12 )
	{
		// Create a new canvas that has the old title "foo 3" but with
		// incremented number: "foo 4". Only one-digit numbers are supported.
		TString title = c1->GetTitle();
		int oldNumber = TString(title[title.Sizeof()-2]).Atoi(); // get last character and turn into integer
		title.Replace(title.Sizeof()-2,1,Form("%i",++oldNumber)); // replace last character with incremented integer
		c1 = selectNewCanvas(title);
	}

	ctrlPadId+=1;
	// cout << "ControlPlots::selectNewPad() : selecting pad id " << ctrlPadId << endl;
	return c1->cd(ctrlPadId);
}

///
/// Save all control plots that were created so far.
///
void ControlPlots::saveCtrlPlots()
{
	for ( int i=0; i<ctrlPlotCanvases.size(); i++ ) {
		TString fName = ctrlPlotCanvases[i]->GetTitle();
		fName.ReplaceAll(name+" ", name+"_"+arg->var[0]+"_");
		fName.ReplaceAll(" ", "_");
		fName = "ctrlPlot_" + fName;
		savePlot(ctrlPlotCanvases[i], fName);
	}
}

///
/// Update the current control plot canvas.
///
void ControlPlots::updateCurrentCanvas()
{
	TCanvas* c1 = ctrlPlotCanvases[ctrlPlotCanvases.size()-1];
	c1->Update();
}

///
/// Apply some generic beautifications to histograms on the ROOT stack.
/// \param htemp Name of the histogram to find.
/// \param Graph Name of the graph to find.
///
void ControlPlots::makePlotsNice(TString htemp, TString Graph)
{
	if ( gPad->GetPrimitive(htemp) ) {
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetXaxis()->SetTitleFont(133);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetXaxis()->SetTitleSize(15);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetXaxis()->SetTitleOffset(2.5);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetXaxis()->SetLabelFont(133);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetXaxis()->SetLabelSize(12);

		((TH1F*)(gPad->GetPrimitive(htemp)))->GetYaxis()->SetTitleFont(133);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetYaxis()->SetTitleSize(15);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetYaxis()->SetTitleOffset(2.25);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetYaxis()->SetLabelFont(133);
		((TH1F*)(gPad->GetPrimitive(htemp)))->GetYaxis()->SetLabelSize(12);
	}

	if ( gPad->GetPrimitive(Graph) ) {
		((TH1F*)(gPad->GetPrimitive(Graph)))->SetMarkerStyle(7);
	}
}

