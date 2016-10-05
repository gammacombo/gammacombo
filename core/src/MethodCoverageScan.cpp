#include "MethodCoverageScan.h"

MethodCoverageScan::MethodCoverageScan(Combiner *comb)
  : MethodAbsScan(comb)
{
  methodName = "Coverage";
}

MethodCoverageScan::MethodCoverageScan()
{
  exit(1);
}

MethodCoverageScan::~MethodCoverageScan()
{
}

int MethodCoverageScan::scan1d(int nRun)
{
  if ( !pCache ) {
    cout << "\nERROR : parameterCache has not been in set for the coverage scanner " << endl;
    exit(1);
  }
	nToys = arg->ncoveragetoys;
	TString forceVariables = "dD_k3pi,dD_kpi,d_dk,g,d_dpi,r_dpi,";

	// set up a graph of chi2 of best solution
	TCanvas* c2 = new TCanvas("c2runToys", "chi2");
	TH1F* hDeltaChi2 = new TH1F("hDeltaChi2", "Delta Chi2 (PLH) of toys", 50, 0, 25);
	hDeltaChi2->Draw();

	// set up a graph of p-values
	TCanvas* c3 = new TCanvas("c3runToys", "p-values");
	TH1F* hPvalues = new TH1F("hPvalues", "p-values", 60, -0.1, 1.1);
	hPvalues->Draw();

	// set up a root tree to save results
	float tSol = 0.0;
  float tSolScan = 0.0;
  float tSolProbErr1Low = 0.0;
  float tSolProbErr1Up  = 0.0;
  float tSolProbErr2Low = 0.0;
  float tSolProbErr2Up  = 0.0;
  float tSolProbErr3Low = 0.0;
  float tSolProbErr3Up  = 0.0;
  vector<float> paramVals;
  for ( int i=0; i<combiner->getParameterNames().size(); i++ ) {
    paramVals.push_back( 0.0 );
  }
	float tChi2free = 0.0;
	float tChi2scan = 0.0;
	float tSolGen;
	float tPvalue;
	float tId;
  int   tnRun;
	TTree *t = new TTree("coverage", "Coverage Toys Tree");
	t->Branch("sol",                  &tSol,          "sol/F");                // central value
	t->Branch("solScan",                  &tSolScan,          "solScan/F");                // central value
  t->Branch("solProbErr1Low",           &tSolProbErr1Low,   "solProbErr1Low/F");         // 1 sigma err
  t->Branch("solProbErr1Up",            &tSolProbErr1Up,    "solProbErr1Up/F");          // 1 sigma err
  t->Branch("solProbErr2Low",           &tSolProbErr2Low,   "solProbErr2Low/F");         // 2 sigma err
  t->Branch("solProbErr2Up",            &tSolProbErr2Up,    "solProbErr2Up/F");          // 2 sigma err
  t->Branch("solProbErr3Low",           &tSolProbErr3Low,   "solProbErr3Low/F");         // 3 sigma err
  t->Branch("solProbErr3Up",            &tSolProbErr3Up,    "solProbErr3Up/F");          // 3 sigma err
  // also want central values for nuisance params
  for ( int i=0; i<combiner->getParameterNames().size(); i++ ) {
    t->Branch( combiner->getParameterNames()[i].c_str(), &paramVals[i], (combiner->getParameterNames()[i]+"/F").c_str() );
  }
	t->Branch("chi2free",             &tChi2free,     "chi2free/F");           // chi2 value of free fit
	t->Branch("chi2scan",             &tChi2scan,     "chi2scan/F");           // chi2 value of scan fit
	t->Branch("solGen",               &tSolGen,       "solgen/F");             // central value of point where coverage is tested
	t->Branch("pvalue",               &tPvalue,       "pvalue/F");             // pvalues of the toys
	t->Branch("id",                   &tId,           "id/F");                 // toys id
  t->Branch("nrun",                 &tnRun,         "nrun/I");               // run no

  // set run number
  tnRun = nRun;

  // combine
  if ( !combiner->isCombined() ) combiner->combine();

	// set up a ToyTree to save the results from the
	// plugin toys
	ToyTree *myTree = new ToyTree(combiner);
	myTree->init();

	// for testing purposes we can fully scan the first 10 toys, set up a plot for this
	//OneMinusClPlot *plot = 0;
	//if ( arg->isAction("test") ) plot = new OneMinusClPlot(arg, "coveragetest_plugin_omcl");

	// toy loop
	for ( int i=0; i<nToys; i++ )
	{
		cout << "ITOY = " << i << endl;
		tId = i;
		RooWorkspace *w = combiner->getWorkspace();
		TString pdfName = combiner->getPdfName();
		TString parName = "par_"+combiner->getPdfName();
		TString varName = arg->var[0];

		// set point to test the coverage at
    int id = arg->id < 0 ? 0 : arg->id;
    pCache->setPoint(combiner, id);

    // tree gen sol
		tSolGen = w->var(varName)->getVal();

		// generate the toy point
		combiner->setObservablesToToyValues();

		// set limits (--pr)
		combiner->loadParameterLimits();

		if ( arg->debug ){
			cout << "PARAMETERS BEFORE TOY SCAN" << endl;
			combiner->getParameters()->Print("v");
		}

		// for testing, fully scan the first 10 toys
		//if ( arg->isAction("test") && i<10 ){
			//scanToy(combiner, plot, i);
		//}
		if ( arg->debug ){
			cout << "PARAMETERS AFTER TOY SCAN" << endl;
			combiner->getParameters()->Print("v");
		}

		// fix observables, float parameters
		fixParameters(combiner->getWorkspace(), "obs_"+combiner->getPdfName());
		floatParameters(combiner->getWorkspace(), "par_"+combiner->getPdfName());

		//
		// Get the best solution in the toy, and that with
		// the scan point fixed. This gets us the deltaChi2 on "data",
		// replacing a full 1-CL Prob scan.
		//
		FitResultCache frCache(arg);
		frCache.storeParsAtFunctionCall(w->set(parName));

		cout << "FREE" << endl;
		w->var(varName)->setConstant(false);
		RooFitResult* rToyFreeFull = fitToMinForce(w, pdfName, forceVariables);
		if ( !rToyFreeFull ) continue;
		RooSlimFitResult *rToyFree = new RooSlimFitResult(rToyFreeFull);
		tChi2free = rToyFree->minNll();  ///< save for tree
		tSol = w->var(varName)->getVal();
		rToyFree->Print();
		delete rToyFreeFull;

    // can fill values of fit parameters here
    for ( int i=0; i<combiner->getParameterNames().size(); i++ ) {
      paramVals[i] = w->var( combiner->getParameterNames()[i].c_str() )->getVal();
    }

		cout << "SCAN" << endl;
		setParameters(w, parName, frCache.getParsAtFunctionCall());
		w->var(varName)->setConstant(true);
		RooFitResult* rToyScanFull = fitToMinForce(w, pdfName, forceVariables);
		if ( !rToyScanFull ) continue;
		RooSlimFitResult *rToyScan = new RooSlimFitResult(rToyScanFull);
		tChi2scan = rToyScan->minNll();  ///< save for tree
		rToyScan->Print();
		w->var(varName)->setConstant(false);
		delete rToyScanFull;

		setParameters(w, parName, frCache.getParsAtFunctionCall());

		//
		// draw chi2
		//
		hDeltaChi2->Fill(tChi2scan-tChi2free);
		c2->cd();
		hDeltaChi2->Draw();
		c2->Update();

		//
		// compute p-value of the Plugin method
		//
		MethodPluginScan *scanner = new MethodPluginScan(combiner);
		tPvalue = scanner->getPvalue1d(rToyScan, tChi2free, myTree, i);
		cout << "P VALUE IS " << tPvalue << endl;
		hPvalues->Fill(tPvalue);
		c3->cd();
		hPvalues->Draw();
		c3->Update();

    // now do the uncertainty scan for the Prob method
    MethodProbScan *probScanner = new MethodProbScan( combiner );
    probScanner->initScan();
    probScanner->loadParameters( rToyFree ); // load parameters from forced fit
    probScanner->scan1d();
    //vector<RooSlimFitResult*> firstScanSolutions = probScanner->getSolutions();
    //for ( int i=0; i<firstScanSolutions.size(); i++ ){
      //probScanner->loadParameters( firstScanSolutions[i] );
      //probScanner->scan1d(true);
    //}
    probScanner->confirmSolutions();
    probScanner->printLocalMinima();
    CLInterval probSig1Int = probScanner->getCLintervalCentral(1);
    CLInterval probSig2Int = probScanner->getCLintervalCentral(2);
    CLInterval probSig3Int = probScanner->getCLintervalCentral(3);
    tSolScan        = probSig1Int.central;
    tSolProbErr1Low = probSig1Int.min;
    tSolProbErr1Up  = probSig1Int.max;
    tSolProbErr2Low = probSig2Int.min;
    tSolProbErr2Up  = probSig2Int.max;
    tSolProbErr3Low = probSig3Int.min;
    tSolProbErr3Up  = probSig3Int.max;


		// fill tree
		t->Fill();

		//
		// cleanup
		//
		delete rToyScan;
		delete rToyFree;
		if ( !arg->isAction("test") ) delete scanner;
	}

  // save trees
  TString idStr = arg->id<0 ? "0" : Form("%d",arg->id);
  TString dirname = "root/scan1dCoverage_"+name+"_"+scanVar1+"_id"+idStr;
  system("mkdir -p "+dirname);
  TFile *f = new TFile(Form(dirname+"/scan1dCoverage_"+name+"_"+scanVar1+"_id"+idStr+"_run%i.root", nRun), "recreate");
  t->Write();
  f->Close();
  return 0;

}

void MethodCoverageScan::readScan1dTrees(int runMin, int runMax) {

  TChain *c = new TChain("coverage");
	int nFilesMissing = 0;
	int nFilesRead = 0;

  TString idStr = arg->id<0 ? "0" : Form("%d",arg->id);
	TString dirname = "root/scan1dCoverage_"+name+"_"+scanVar1+"_id"+idStr;
	TString fileNameBase = dirname+"/scan1dCoverage_"+name+"_"+scanVar1+"_id"+idStr+"_run";
	if ( arg->debug ) cout << "MethodCoverageScan::readScan1dTrees() : ";
	cout << "reading files: " << fileNameBase+"*.root" << endl;
	for (int i=runMin; i<=runMax; i++){
		TString file = Form(fileNameBase+"%i.root", i);
		if ( !FileExists(file) ){
			if ( arg->verbose ) cout << "ERROR : File not found: " + file + " ..." << endl;
			nFilesMissing += 1;
			continue;
		}
		if ( arg->verbose ) cout << "reading " + file + " ..." << endl;
		c->Add(file);
		nFilesRead += 1;
	}

  if (arg->debug) c->Print();

	// initialize histograms
	h_sol           = new TH1F("h_sol", "best solution", 100, 0, 180);
	h_pvalue_plugin = new TH1F("h_pvalue_plugin", "p-value", 50, 0, 1);
	h_pvalue_prob   = new TH1F("h_pvalue_prob",   "p-value", 50, 0, 1);
	h_pvalue_plugin_notransf = new TH1F("h_pvalue_plugin_notransf", "p-value", 50, 0, 1);
	h_pvalue_prob_notransf = new TH1F("h_pvalue_prob_notransf", "p-value", 50, 0, 1);

	// transform function
	// options are p1, p1+exp, p1+1/x
	TString transFunc = "none";

	// set up root tree for reading
	float tSol = 0.0;
	float tChi2free = 0.0;
	float tChi2scan = 0.0;
	float tSolGen = 0.0;
	float tPvalue = 0.0;
  int   tnRun = 0;
	c->SetBranchAddress("sol",      &tSol);
	c->SetBranchAddress("chi2free", &tChi2free);
	c->SetBranchAddress("chi2scan", &tChi2scan);
	c->SetBranchAddress("solGen",   &tSolGen);
	c->SetBranchAddress("pvalue",   &tPvalue);
  c->SetBranchAddress("nrun",     &tnRun);

	// initialize loop variables
	nentries  = c->GetEntries();
	nfailed   = 0 ;
	n68plugin = 0.;
	n95plugin = 0.;
	n99plugin = 0.;
	n68prob   = 0.;
	n95prob   = 0.;
	n99prob   = 0.;

	// run through tree first to get p-value distributions which we can fit for a transform later
	for (Long64_t i = 0; i < nentries; i++)
	{
		c->GetEntry(i);
		// apply cuts (we'll count the failed ones later)
		if ( ! (tChi2free > -1e10 && tChi2scan > -1e10
					&& tChi2scan-tChi2free>0
					&& tSol != 0.0 ///< exclude some pathological jobs from when tSol wasn't set yet
					&& tChi2free < 500 && tChi2scan < 500
			   )){
			continue;
		}
		// fill plugin p-value distribution
		h_pvalue_plugin_notransf->Fill(tPvalue);
		// fill prob p-value distribution
		float pvalueProb = TMath::Prob(tChi2scan-tChi2free,1);
		h_pvalue_prob_notransf->Fill(pvalueProb);
	}

	// fit p-values (if transFunc=="none" don't bother)
	vector<double> fitParamsPlugin;
	vector<double> fitParamsProb;
	TCanvas *my_c;
	if ( transFunc != TString("none") ) {
		my_c = new TCanvas();
		my_c->cd();
		TH1F h("dummy","",1,0,1);
		h.GetYaxis()->SetRangeUser(0.,2.);
		h.Draw("AXIS");
		fitParamsProb = fitHist(h_pvalue_prob_notransf, transFunc);
		fitParamsPlugin = fitHist(h_pvalue_plugin_notransf, transFunc);
	}

	// Now fill the distributions (applying the transform if necessary)
	for (Long64_t i = 0; i < nentries; i++)
	{
		c->GetEntry(i);

		// apply cuts
		if ( ! (tChi2free > -1e10 && tChi2scan > -1e10
					&& tChi2scan-tChi2free>0
					&& tSol != 0.0 ///< exclude some pathological jobs from when tSol wasn't set yet
					&& tChi2free < 500 && tChi2scan < 500
			   )){
			nfailed++;
			continue;
		}

		// draw best fit points
		// assume the variable is an angle (=gamma) and bring it back
		// to the [0,pi] range
		h_sol->Fill(RadToDeg(fmod((double)tSol,3.14152)));

		// attempt a coverage correction
		// float a = -35.3458/76.1644;
		// tPvalue = (tPvalue + 1./2.*a*sq(tPvalue))/(1.+1./2.*a);

		// draw plugin p-value distribution
		if (transFunc!=TString("none")) {
			tPvalue = transform(fitParamsPlugin,transFunc,tPvalue);
		}
		h_pvalue_plugin->Fill(tPvalue);
		if(tPvalue>1.-0.6827) n68plugin++;
		if(tPvalue>1.-0.9545) n95plugin++;
		if(tPvalue>1.-0.9973) n99plugin++;

		// draw prob p-value distribution
		float pvalueProb = TMath::Prob(tChi2scan-tChi2free,1);

		// attempt a coverage correction
		// float a = -0.6;
		// pvalueProb = (pvalueProb + 1./2.*a*sq(pvalueProb))/(1.+1./2.*a);

		if (transFunc!=TString("none")) {
			pvalueProb = transform(fitParamsProb,transFunc,pvalueProb);
		}
		h_pvalue_prob->Fill(pvalueProb);
		if(pvalueProb>1.-0.6827) n68prob++;
		if(pvalueProb>1.-0.9545) n95prob++;
		if(pvalueProb>1.-0.9973) n99prob++;
	}
}

void MethodCoverageScan::saveScanner(TString fName)
{
	if ( fName=="" ){
		FileNameBuilder fb(arg);
		fName = fb.getFileNameScanner(this);
	}
	if ( arg->debug ) cout << "MethodCoverageScan::saveScanner() : saving scanner: " << fName << endl;
	TFile f(fName, "recreate");

  // save histograms
  h_sol->Write();
  h_pvalue_plugin->Write();
  h_pvalue_prob->Write();
  h_pvalue_plugin_notransf->Write();
  h_pvalue_prob_notransf->Write();

  // save result values
  TTree *outTree = new TTree("result_values","Coverage Result Values");
  outTree->Branch( "nentries",  &nentries,  "nentries/L" );
  outTree->Branch( "nfailed" ,  &nfailed ,  "nfailed/L"  );
  outTree->Branch( "n68plugin", &n68plugin, "n68plugin/F" );
  outTree->Branch( "n95plugin", &n95plugin, "n95plugin/F" );
  outTree->Branch( "n99plugin", &n99plugin, "n99plugin/F" );
  outTree->Branch( "n68prob"  , &n68prob  , "n68prob/F" );
  outTree->Branch( "n95prob"  , &n95prob  , "n95prob/F" );
  outTree->Branch( "n99prob",   &n99prob  , "n99prob/F" );
  outTree->Fill();
  outTree->Write();

  f.Close();
}

bool MethodCoverageScan::loadScanner(TString fName)
{
	if ( fName=="" ){
		FileNameBuilder fb(arg);
		fName = fb.getFileNameScanner(this);
	}
	if ( arg->debug ) cout << "MethodCoverageScan::loadScanner() : ";
	cout << "loading scanner: " << fName << endl;
	if ( !FileExists(fName) ){
		cout << "MethodCoverageScan::loadScanner() : ERROR : file not found: " << fName << endl;
		cout << "                                    Run first without the '-a plot' option to produce the missing file." << endl;
		exit(1);
	}
	TFile *f = new TFile(fName, "ro"); // don't delete this later else the objects die
  h_sol                     = (TH1F*)f->Get("h_sol");
  h_pvalue_plugin           = (TH1F*)f->Get("h_pvalue_plugin");
  h_pvalue_prob             = (TH1F*)f->Get("h_pvalue_prob");
  h_pvalue_plugin_notransf  = (TH1F*)f->Get("h_pvalue_plugin_notransf");
  h_pvalue_prob_notransf    = (TH1F*)f->Get("h_pvalue_prob_notransf");

  if ( ! h_sol ) {
    cout << "\nERROR : Histogram \'h_sol\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }
  if ( ! h_pvalue_plugin ) {
    cout << "\nERROR : Histogram \'h_pvalue_plugin\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }
  if ( ! h_pvalue_prob ) {
    cout << "\nERROR : Histogram \'h_pvalue_prob\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }
  if ( ! h_pvalue_plugin_notransf ) {
    cout << "\nERROR : Histogram \'h_pvalue_plugin_notransf\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }
  if ( ! h_pvalue_prob_notransf ) {
    cout << "\nERROR : Histogram \'h_pvalue_prob_notransf\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }

  TTree *loadTree = (TTree*)f->Get("result_values");
  if ( ! loadTree ) {
    cout << "\nERROR : Tree \'result_values\' not found in saved coverage scanner : " << fName << endl;
    exit(1);
  }
  loadTree->SetBranchAddress( "nentries",  &nentries  );
  loadTree->SetBranchAddress( "nfailed" ,  &nfailed   );
  loadTree->SetBranchAddress( "n68plugin", &n68plugin );
  loadTree->SetBranchAddress( "n95plugin", &n95plugin );
  loadTree->SetBranchAddress( "n99plugin", &n99plugin );
  loadTree->SetBranchAddress( "n68prob"  , &n68prob   );
  loadTree->SetBranchAddress( "n95prob"  , &n95prob   );
  loadTree->SetBranchAddress( "n99prob",   &n99prob   );
  loadTree->GetEntry(0);

  return true;
}

void MethodCoverageScan::plot()
{
	int font       = 133;
	int labelsize  = 35;  ///< axis labels, numeric solutions, CL guide lines
	int titlesize  = 40;  ///< axis titles, "LHCb", "Preliminary" is x0.75
	int legendsize = 40;  ///< legends in 1d and 2d plots
	gStyle->SetOptStat(0);
	gStyle->SetOptTitle(0);
	gStyle->SetPadTopMargin(0.05);
	gStyle->SetPadRightMargin(0.05);
	gStyle->SetPadBottomMargin(0.17);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetLabelOffset(0.005, "X");
	gStyle->SetLabelOffset(0.010, "Y");

	TCanvas *c1 = newNoWarnTCanvas(name+getUniqueRootName(),"Coverage Solution",800,600);
	h_sol->SetTitle(combiner->getTitle());
	h_sol->GetXaxis()->SetTitle("#gamma mod #pi [#circ]");
	h_sol->GetYaxis()->SetTitle("toys");
	h_sol->GetXaxis()->SetTitleSize(titlesize);
	h_sol->GetYaxis()->SetTitleSize(titlesize);
	h_sol->GetXaxis()->SetLabelSize(labelsize);
	h_sol->GetYaxis()->SetLabelSize(labelsize);
	h_sol->GetXaxis()->SetTitleFont(font);
	h_sol->GetYaxis()->SetTitleFont(font);
	h_sol->GetXaxis()->SetLabelFont(font);
	h_sol->GetYaxis()->SetLabelFont(font);
	h_sol->GetXaxis()->SetTitleOffset(0.8);
	h_sol->GetYaxis()->SetTitleOffset(0.85);
	h_sol->Draw();
	savePlot(c1, name + "_bestfit" + arg->plotext);

	// plot p-values
	TCanvas *c2 = newNoWarnTCanvas(name+getUniqueRootName(),"Coverage p-value (Prob)",800,600);
	h_pvalue_prob->SetTitle(combiner->getTitle());
	h_pvalue_prob->GetXaxis()->SetTitle("p-value");
	h_pvalue_prob->GetYaxis()->SetTitle("toys");
	h_pvalue_prob->GetXaxis()->SetTitleSize(titlesize);
	h_pvalue_prob->GetYaxis()->SetTitleSize(titlesize);
	h_pvalue_prob->GetXaxis()->SetLabelSize(labelsize);
	h_pvalue_prob->GetYaxis()->SetLabelSize(labelsize);
	h_pvalue_prob->GetXaxis()->SetLabelFont(font);
	h_pvalue_prob->GetYaxis()->SetLabelFont(font);
	h_pvalue_prob->GetXaxis()->SetTitleFont(font);
	h_pvalue_prob->GetYaxis()->SetTitleFont(font);
	h_pvalue_prob->GetXaxis()->SetTitleOffset(0.8);
	h_pvalue_prob->GetYaxis()->SetTitleOffset(0.85);
	h_pvalue_prob->SetLineColor(kRed+2);
	h_pvalue_prob->SetLineStyle(kDashed);
	h_pvalue_prob->SetLineWidth(2);
	h_pvalue_prob->GetYaxis()->SetRangeUser(0,h_pvalue_prob->GetMaximum()*1.2);
	h_pvalue_prob->Draw();

	int ci = 926;
	TColor *color = new TColor(ci, 0.35, 0.33, 0.85, " ", 0.488);
	h_pvalue_plugin->SetFillColor(ci);
	h_pvalue_plugin->SetFillStyle(3004);
	h_pvalue_plugin->Draw("same");
	TLegend *leg = new TLegend(0.6810345,0.7357294,0.9353448,0.8900634,NULL,"brNDC");
	leg->SetBorderSize(1);
	leg->SetLineColor(0);
	leg->SetLineStyle(1);
	leg->SetLineWidth(1);
	leg->SetFillColor(0);
	leg->SetFillStyle(3001);
	leg->SetTextSize(legendsize);
	leg->SetTextFont(font);
	leg->AddEntry(h_pvalue_prob, "Prob", "l");
	leg->AddEntry(h_pvalue_plugin, "Plugin", "l");
	leg->Draw();
	savePlot(c2, name + "_pvalue" + arg->plotext);

	// compute coverage
	float n = (float)(nentries-nfailed);
	cout<<"coverage test nToys="<<n<<endl;
	cout<<"Plugin:" << endl;
	cout<<" eta=68.27%: alpha="<<Form("%.4f",1.*n68plugin/n)<<" +/- "<<Form("%.4f",sqrt(n68plugin*(n-n68plugin)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n68plugin/n-0.6827) << endl
		<<" eta=95.45%: alpha="<<Form("%.4f",1.*n95plugin/n)<<" +/- "<<Form("%.4f",sqrt(n95plugin*(n-n95plugin)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n95plugin/n-0.9545) << endl
		<<" eta=99.73%: alpha="<<Form("%.4f",1.*n99plugin/n)<<" +/- "<<Form("%.4f",sqrt(n99plugin*(n-n99plugin)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n99plugin/n-0.9973) << endl;
	cout<<"Prob:" << endl;
	cout<<" eta=68.27%: alpha="<<Form("%.4f",1.*n68prob/n)<<" +/- "<<Form("%.4f",sqrt(n68prob*(n-n68prob)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n68prob/n-0.6827) << endl
		<<" eta=95.45%: alpha="<<Form("%.4f",1.*n95prob/n)<<" +/- "<<Form("%.4f",sqrt(n95prob*(n-n95prob)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n95prob/n-0.9545) << endl
		<<" eta=99.73%: alpha="<<Form("%.4f",1.*n99prob/n)<<" +/- "<<Form("%.4f",sqrt(n99prob*(n-n99prob)/n)/n)
		<<" alpha-eta="<<        Form("%.4f",1.*n99prob/n-0.9973) << endl;
	cout<<"The stat. errors are correlated as they come from the same toys!" << endl;
	cout<<"\nLatex:"<<endl;
	printLatexLine(0.6827, 1.*n68prob/n, sqrt(n68prob*(n-n68prob)/n)/n, 1.*n68plugin/n, sqrt(n68plugin*(n-n68plugin)/n)/n);
	printLatexLine(0.9545, 1.*n95prob/n, sqrt(n95prob*(n-n95prob)/n)/n, 1.*n95plugin/n, sqrt(n95plugin*(n-n95plugin)/n)/n);
	printLatexLine(0.9973, 1.*n99prob/n, sqrt(n99prob*(n-n99prob)/n)/n, 1.*n99plugin/n, sqrt(n99plugin*(n-n99plugin)/n)/n);
}

/// fit p-value histogram with some function

vector<double> MethodCoverageScan::fitHist( TH1* h, TString fitfunc, bool draw )
{
	if ( fitfunc!="p1" && fitfunc!="p1+exp" && fitfunc!="p1+1/x" ) {
		cout << "ERROR -- " << fitfunc << " is not a valid function" << endl;
		exit(1);
	}

	vector<double> fitParams;
	TString fitString;
	TF1 *fitFunc;

	if (fitfunc=="p1") fitString = "pol1";
	if (fitfunc=="p1+exp") fitString = "[0] + [1]*x + [2]*exp(-1.*[3]*x)";
	if (fitfunc=="p1+1/x") fitString = "[0] + [1]*x + [2]/(x+[3])";

	fitFunc = new TF1("fit",fitString,0.,1.);
	h->Fit("fit");
	for (int f=0; f<fitFunc->GetNumberFreeParameters(); f++){
		fitParams.push_back(fitFunc->GetParameter(f));
	}

	if (fitfunc=="p1") assert(fitParams.size()==2);
	if (fitfunc=="p1+exp") assert(fitParams.size()==4);
	if (fitfunc=="p1+1/x") assert(fitParams.size()==4);

	if (draw) {
		h->Draw("HISTsame");
		fitFunc->Draw("Fsame");
	}
	return fitParams;
}

///
/// apply the appropriate transformation
///

double MethodCoverageScan::transform( vector<double> fitParams, TString transFunc, double x )
{
	double y = -999.; // this should be getting returned

	if ( transFunc!="p1" && transFunc!="p1+exp" && transFunc!="p1+1/x" ) {
		cout << "ERROR -- " << transFunc << " is not a valid function" << endl;
		exit(1);
	}
	if (transFunc=="p1") assert(fitParams.size()==2);
	if (transFunc=="p1+exp") assert(fitParams.size()==4);
	if (transFunc=="p1+1/x") assert(fitParams.size()==4);


	if (transFunc=="p1") {
		double a = fitParams[1]/fitParams[0];
		y = ( x + 0.5*a*x*x ) / ( 1. + 0.5*a );
	}
	if (transFunc=="p1+exp") {
		double a = fitParams[1]/fitParams[0];
		double b = fitParams[2]/fitParams[0];
		double c = fitParams[3];
		y = ( x + 0.5*a*x*x + (b/c)*(1.-exp(-1.*c*x)) ) / ( 1. + 0.5*a + (b/c)*(1.-exp(-1.*c)) );
	}
	if (transFunc=="p1+1/x") {
		double a = fitParams[1]/fitParams[0];
		double b = fitParams[2]/fitParams[0];
		double c = fitParams[3];
		y = ( x + 0.5*a*x*x + b*log((x+c)/c) ) / ( 1. + 0.5*a + b*log((x+c)/c) );
	}

	assert( y >= 0. && y <= 1.);
	return y;
}

///
/// Print the results as Latex table line
///
void MethodCoverageScan::printLatexLine(float eta, float finProb, float finProbErr,
		float finPlug, float finPlugErr)
{
	printf("$\\eta=%.4f$ & $%.4f \\pm %.4f$ & $%.4f$ & $%.4f \\pm %.4f$ & $%.4f$ & $%.2f$ \\\\\n",
			eta, finProb, finProbErr, finProb-eta, finPlug, finPlugErr, finPlug-eta, eta/finPlug);
}

