#include "ToyTree.h"

ToyTree::ToyTree(Combiner *c, TChain* t, bool _quiet):
	quiet(_quiet)
{
	setCombiner(c); // load properties from the combiner
	this->initMembers(t);
	this->storeObs  = true;
	this->storeTh   = true;
	this->storeGlob = false;
}

ToyTree::ToyTree(PDF_Datasets *p, OptParser* opt, TChain* t, bool _quiet):
	quiet(_quiet)
{
	assert(p);
	this->comb = NULL;
	this->w = p->getWorkspace();
	this->name = p->getName();
	this->arg = opt;
	this->pdfName  = "pdf_"+p->getPdfName();
	this->obsName  = p->getObsName();
	this->parsName = p->getParName();
  this->globName = p->getGlobalObsName();
	this->thName   = "";
	this->initMembers(t);
	this->storeObs  = false;
	this->storeTh   = false;
	this->storeGlob = true;
};


ToyTree::~ToyTree()
{}

///
/// Initialize the data members.
///
void ToyTree::initMembers(TChain* t){
	this->t = t;
	scanpointMin        = 0.;
	scanpointMax        = 0.;
	scanpointN          = -1;
	scanpoint           = 0.;
	scanpointyMin       = 0.;
	scanpointyMax       = 0.;
	scanpointyN         = -1;
	scanpointy          = 0.;
	chi2min             = 0.;
	chi2minGlobal       = 0.;
	chi2minBkg    		  = 0.;
	chi2minToy          = 0.;
	chi2minGlobalToy    = 0.;
	chi2minBkgToy       = 0.;
	chi2minGlobalBkgToy = 0.;
	chi2minBkgBkgToy 	= 0.;
	scanbest            = 0.;
	scanbesty           = 0.;
	scanbestBkg         = 0.;
	scanbestBkgfitBkg   = 0.;
	nrun                = 0.;
	ntoy                = 0.;
	npoint              = 0.;
	id                  = 0.;
	statusFree          = -5.;
	statusScan          = -5.;
	statusFreeBkg       = -5.;
	statusScanBkg       = -5.;
	statusBkgBkg       = -5.;
	statusScanData      = -5.;
	nBergerBoos         = 0.;
	BergerBoos_id       = 0.;
	genericProbPValue   = 0.;
	covQualFree         = -2.;
	covQualScan         = -2.;
	covQualFreeBkg         = -2.;
	covQualScanBkg         = -2.;
	covQualBkgBkg         = -2.;
	covQualScanData     = -2.;
	statusFreePDF       = -5.;
	statusScanPDF       = -5.;
	chi2minToyPDF       = 0.;
	chi2minGlobalToyPDF = 0.;
	chi2minBkgToyPDF    = 0.;
    bestIndexScanData   = 0;
};

///
/// Set the combiner to work with. When setting a new combiner, different
/// from the one used in the constructor, the new one needs to have the
/// same structure, but can have different unique PDF IDs. This way we can,
/// e.g. in the coverage test, save results from many different toys.
///
void ToyTree::setCombiner(Combiner* c)
{
	assert(c);
	if ( !c->isCombined() ){
		cout << "ToyTree::setCombiner() : ERROR : combiner is not combined yet! Exit." << endl;
		exit(1);
	}
	this->comb = c;
	this->w = c->getWorkspace();
	this->name = c->getName();
	this->arg = c->getArg();
	pdfName  = "pdf_"+c->getPdfName();
	obsName  = "obs_"+c->getPdfName();
	parsName = "par_"+c->getPdfName();
	thName   = "th_"+c->getPdfName();
}

///
/// Save current values of the proxy variables as an event
/// into the TTree.
///
void ToyTree::fill()
{
	if ( t ) t->Fill();
}

///
/// Save the TTree into a root file.
///
void ToyTree::writeToFile(TString fName)
{
	assert(t);
	if ( arg->debug ) cout << "ToyTree::writeToFile() : ";
	cout << "saving toys to: " << fName << endl;
	TFile *f = new TFile(fName, "recreate");
	t->Write();
	f->Close();
}

void ToyTree::writeToFile()
{
	assert(t);
	if ( arg->debug ){
		cout << "ToyTree::writeToFile() : ";
		cout << "saving toys to ... " << endl;
	}
	t->GetCurrentFile()->cd();
	t->Write();
}

///
/// Initialize a new TTree, set up all its leaves, connect
/// them to the proxy variables.
///
void ToyTree::init()
{
	t = new TTree("plugin", "plugin");
	t->Branch("BergerBoos_id",       &BergerBoos_id,       "BergerBoos_id/F");
	t->Branch("chi2min",             &chi2min,             "chi2min/F");
	t->Branch("chi2minToy",          &chi2minToy,          "chi2minToy/F");
	t->Branch("chi2minGlobal",       &chi2minGlobal,       "chi2minGlobal/F");
	t->Branch("chi2minGlobalToy",    &chi2minGlobalToy,    "chi2minGlobalToy/F");
	t->Branch("chi2minGlobalBkgToy", &chi2minGlobalBkgToy, "chi2minGlobalBkgToy/F");
	t->Branch("chi2minBkgBkgToy", 	 &chi2minBkgBkgToy,    "chi2minBkgBkgToy/F");
	t->Branch("chi2minBkg",    	     &chi2minBkg,		   "chi2minBkg/F");
	t->Branch("chi2minBkgToy", 	     &chi2minBkgToy,       "chi2minBkgToy/F");
	t->Branch("covQualFree",         &covQualFree,         "covQualFree/F");
	t->Branch("covQualScan",         &covQualScan,         "covQualScan/F");
	t->Branch("covQualFreeBkg",      &covQualFreeBkg,      "covQualFreeBkg/F");
	t->Branch("covQualScanBkg",      &covQualScanBkg,      "covQualScanBkg/F");
	t->Branch("covQualBkgBkg",       &covQualBkgBkg,       "covQualBkgBkg/F");
	t->Branch("covQualScanData",     &covQualScanData,     "covQualScanData/F");
	t->Branch("genericProbPValue",   &genericProbPValue,   "genericProbPValue/F");
	t->Branch("id",                  &id,                  "id/F");
	t->Branch("nBergerBoos",         &nBergerBoos,         "nBergerBoos/F");
	t->Branch("nrun",                &nrun,                "nrun/F");
	t->Branch("ntoy",                &ntoy,                "ntoy/F");
	t->Branch("npoint",              &npoint,              "npoint/F");
	t->Branch("scanbest",            &scanbest,            "scanbest/F");
	t->Branch("scanbesty",           &scanbesty,           "scanbesty/F");
	t->Branch("scanbestBkg",         &scanbestBkg,         "scanbestBkg/F");
	t->Branch("scanbestBkgfitBkg",   &scanbestBkgfitBkg,   "scanbestBkgfitBkg/F");
	t->Branch("scanpoint",           &scanpoint,           "scanpoint/F");
	t->Branch("scanpointy",          &scanpointy,          "scanpointy/F");
	t->Branch("statusFree",          &statusFree,          "statusFree/F");
	t->Branch("statusScan",          &statusScan,          "statusScan/F");
	t->Branch("statusScanData",      &statusScanData,      "statusScanData/F");
	t->Branch("statusFreeBkg",       &statusFreeBkg,   		"statusFreeBkg/F");
	t->Branch("statusScanBkg",       &statusScanBkg,    	"statusScanBkg/F");
	t->Branch("statusBkgBkg",        &statusBkgBkg,        	"statusBkgBkg/F");
    t->Branch("bestIndexScanData",   &bestIndexScanData,    "bestIndexScanData/I");
	if ( !arg->lightfiles )
	{
		TIterator* it = w->set(parsName)->createIterator();
		while ( RooRealVar* p = (RooRealVar*)it->Next() )
		{
			parametersScan.insert(pair<string,float>(p->GetName(),p->getVal()));
			t->Branch(TString(p->GetName())+"_scan", &parametersScan[p->GetName()], TString(p->GetName())+"_scan/F");
			parametersFree.insert(pair<string,float>(p->GetName(),p->getVal()));
			t->Branch(TString(p->GetName())+"_free", &parametersFree[p->GetName()], TString(p->GetName())+"_free/F");
			parametersPll.insert(pair<string,float>(p->GetName(),p->getVal()));
			t->Branch(TString(p->GetName())+"_start", &parametersPll[p->GetName()], TString(p->GetName())+"_start/F");
		}
		// observables
		if(this->storeObs){
			delete it; it = w->set(obsName)->createIterator();
			while ( RooRealVar* p = (RooRealVar*)it->Next() )
			{
				observables.insert(pair<string,float>(p->GetName(),p->getVal()));
				t->Branch(TString(p->GetName()), &observables[p->GetName()], TString(p->GetName())+"/F");
			}
		}
		// theory
		if(this->storeTh){
			delete it; it = w->set(thName)->createIterator();
			while ( RooRealVar* p = (RooRealVar*)it->Next() )
			{
				theory.insert(pair<string,float>(p->GetName(),p->getVal()));
				t->Branch(TString(p->GetName()), &theory[p->GetName()], TString(p->GetName())+"/F");
			}
		}
		// global observables
	    if(this->storeGlob){
	      delete it;
	      if(w->set(globName)==NULL){
	      	cerr<<"Unable to store parameters of global constraints because no set called "+globName
	      		<<" is defined in the workspace. "<<endl;
	      		//\todo Implement init function in PDF_Datasets to enabe the user to set the name of this set in the workspace.
	      		exit(EXIT_FAILURE);
	      }
	      it = w->set(globName)->createIterator();
	      while ( RooRealVar* p = (RooRealVar*)it->Next() )
	      {
	        constraintMeans.insert(pair<TString,float>(p->GetName(),p->getVal()));
	        t->Branch(TString(p->GetName()), &constraintMeans[p->GetName()], TString(p->GetName())+"/F");
	      }
	    }
	    delete it;
	}
}

///
/// Provide the interface to read an external TChain.
///
void ToyTree::open()
{
	TObjArray* branches = t->GetListOfBranches();
	if(branches->FindObject("BergerBoos_id"      )) t->SetBranchAddress("BergerBoos_id",      &BergerBoos_id);
	if(branches->FindObject("chi2min"            )) t->SetBranchAddress("chi2min",            &chi2min);
	if(branches->FindObject("chi2minToy"         )) t->SetBranchAddress("chi2minToy",         &chi2minToy);
	if(branches->FindObject("chi2minGlobal"      )) t->SetBranchAddress("chi2minGlobal",      &chi2minGlobal);
	if(branches->FindObject("chi2minGlobalToy"   )) t->SetBranchAddress("chi2minGlobalToy",   &chi2minGlobalToy);
	if(branches->FindObject("chi2minGlobalBkgToy")) t->SetBranchAddress("chi2minGlobalBkgToy",&chi2minGlobalBkgToy);
	if(branches->FindObject("chi2minBkgBkgToy"	 )) t->SetBranchAddress("chi2minBkgBkgToy",	  &chi2minBkgBkgToy);
	if(branches->FindObject("chi2minBkg"      	 )) t->SetBranchAddress("chi2minBkg",      	  &chi2minBkg);
	if(branches->FindObject("chi2minBkgToy"   	 )) t->SetBranchAddress("chi2minBkgToy",   	  &chi2minBkgToy);
	if(branches->FindObject("covQualFree"        )) t->SetBranchAddress("covQualFree",        &covQualFree);
	if(branches->FindObject("covQualScan"        )) t->SetBranchAddress("covQualScan",        &covQualScan);
	if(branches->FindObject("covQualScanData"    )) t->SetBranchAddress("covQualScanData",    &covQualScanData);
	if(branches->FindObject("covQualFreeBkg"     )) t->SetBranchAddress("covQualFreeBkg",     &covQualFreeBkg);
	if(branches->FindObject("covQualScanBkg"     )) t->SetBranchAddress("covQualScanBkg",     &covQualScanBkg);
	if(branches->FindObject("covQualBkgBkg"      )) t->SetBranchAddress("covQualBkgBkg",      &covQualBkgBkg);
	if(branches->FindObject("genericProbPValue"  )) t->SetBranchAddress("genericProbPValue",  &genericProbPValue);
	if(branches->FindObject("nBergerBoos"        )) t->SetBranchAddress("nBergerBoos",        &nBergerBoos);
	if(branches->FindObject("scanbest"           )) t->SetBranchAddress("scanbest",           &scanbest);
	if(branches->FindObject("scanbesty"          )) t->SetBranchAddress("scanbesty",          &scanbesty);
	if(branches->FindObject("scanbestBkg"        )) t->SetBranchAddress("scanbestBkg",        &scanbestBkg);
	if(branches->FindObject("scanbestBkgfitBkg"  )) t->SetBranchAddress("scanbestBkgfitBkg",  &scanbestBkgfitBkg);
	if(branches->FindObject("scanpoint"          )) t->SetBranchAddress("scanpoint",          &scanpoint);
	if(branches->FindObject("scanpointy"         )) t->SetBranchAddress("scanpointy",         &scanpointy);
	if(branches->FindObject("statusFree"         )) t->SetBranchAddress("statusFree",         &statusFree);
	if(branches->FindObject("statusScan"         )) t->SetBranchAddress("statusScan",         &statusScan);
	if(branches->FindObject("statusScanData"     )) t->SetBranchAddress("statusScanData",     &statusScanData);
	if(branches->FindObject("statusFreeBkg"      )) t->SetBranchAddress("statusFreeBkg",      &statusFreeBkg);
	if(branches->FindObject("statusScanBkg"      )) t->SetBranchAddress("statusScanBkg",      &statusScanBkg);
	if(branches->FindObject("statusBkgBkg"       )) t->SetBranchAddress("statusBkgBkg",       &statusBkgBkg);
	if(branches->FindObject("chi2minGlobalToyPDF")) t->SetBranchAddress("chi2minGlobalToyPDF",&chi2minGlobalToyPDF);
	if(branches->FindObject("chi2minBkgToyPDF"	 )) t->SetBranchAddress("chi2minBkgToyPDF",	  &chi2minBkgToyPDF);
	if(branches->FindObject("chi2minToyPDF"      )) t->SetBranchAddress("chi2minToyPDF",      &chi2minToyPDF);
	// if(branches->FindObject("covQualFree"        )) t->SetBranchAddress("covQualFree",        &covQualFree);
	// if(branches->FindObject("covQualScan"        )) t->SetBranchAddress("covQualScan",        &covQualScan);
	// if(branches->FindObject("covQualScanData"    )) t->SetBranchAddress("covQualScanData",    &covQualScanData);
	if(branches->FindObject("statusFreePDF"      )) t->SetBranchAddress("statusFreePDF",      &statusFreePDF);
	// if(branches->FindObject("statusScanData"     )) t->SetBranchAddress("statusScanData",     &statusScanData);
	if(branches->FindObject("statusScanPDF"      )) t->SetBranchAddress("statusScanPDF",      &statusScanPDF);
    if(branches->FindObject("bestIndexScanData"  )) t->SetBranchAddress("bestIndexScanData",  &bestIndexScanData);
}

///
/// Activate only core branches to speed up reading them.
///
void ToyTree::activateCoreBranchesOnly()
{
	TObjArray* branches = t->GetListOfBranches();
	t->SetBranchStatus("*", 0); // perhaps we need ".*" in certain root versions?
	if(branches->FindObject("BergerBoos_id"))         t->SetBranchStatus("BergerBoos_id",      1);
	if(branches->FindObject("chi2min"))               t->SetBranchStatus("chi2min",            1);
	if(branches->FindObject("chi2minToy"))            t->SetBranchStatus("chi2minToy",         1);
	if(branches->FindObject("chi2minGlobal"))         t->SetBranchStatus("chi2minGlobal",      1);
	if(branches->FindObject("chi2minGlobalToy"))      t->SetBranchStatus("chi2minGlobalToy",   1);
	if(branches->FindObject("chi2minGlobalBkgToy"))   t->SetBranchStatus("chi2minGlobalBkgToy",1);
	if(branches->FindObject("chi2minBkgBkgToy"))   	  t->SetBranchStatus("chi2minBkgBkgToy",   1);
	if(branches->FindObject("chi2minBkg"))         	  t->SetBranchStatus("chi2minBkg",         1);
	if(branches->FindObject("chi2minBkgToy"))      	  t->SetBranchStatus("chi2minBkgToy",      1);
	if(branches->FindObject("genericProbPValue"))     t->SetBranchStatus("genericProbPValue",  1);
	if(branches->FindObject("id"))                    t->SetBranchStatus("id",                 1);
	if(branches->FindObject("nBergerBoos"))           t->SetBranchStatus("nBergerBoos",        1);
	if(branches->FindObject("scanpoint"))             t->SetBranchStatus("scanpoint",          1);
	if(branches->FindObject("scanpointy"))            t->SetBranchStatus("scanpointy",         1);
	if(branches->FindObject("statusFree"))            t->SetBranchStatus("statusFree",         1);
	if(branches->FindObject("statusScan"))            t->SetBranchStatus("statusScan",         1);
	if(branches->FindObject("statusScanData"))        t->SetBranchStatus("statusScanData",     1);
	if(branches->FindObject("chi2minGlobalToyPDF"))   t->SetBranchStatus("chi2minGlobalToyPDF",1);
	if(branches->FindObject("chi2minBkgToyPDF"))   	  t->SetBranchStatus("chi2minBkgToyPDF",   1);
	if(branches->FindObject("chi2minToyPDF"))         t->SetBranchStatus("chi2minToyPDF",      1);
	if(branches->FindObject("covQualFree"))           t->SetBranchStatus("covQualFree",        1);
	if(branches->FindObject("covQualScan"))           t->SetBranchStatus("covQualScan",        1);
	if(branches->FindObject("covQualScanData"))       t->SetBranchStatus("covQualScanData",    1);
	if(branches->FindObject("statusFreePDF"))         t->SetBranchStatus("statusFreePDF",      1);
	if(branches->FindObject("statusScanData"))        t->SetBranchStatus("statusScanData",     1);
	if(branches->FindObject("statusScanPDF"))         t->SetBranchStatus("statusScanPDF",      1);
}

///
/// activate a single Tree
///
void ToyTree::activateBranch(const TString& bName){
	t->SetBranchStatus(bName,1);
}


///
/// Activate all branches.
///
void ToyTree::activateAllBranches()
{
	t->SetBranchStatus("*", 1);
}

///
/// Store the current workspace fit parameters as the
/// profile likelihood parameters.
///
void ToyTree::storeParsPll()
{
	if( !w->set(parsName) )
	{
		cout << "ToyTree::storeParsPll() : ERROR : not found in workspace: " << parsName << endl;
		cout << "ToyTree::storeParsPll() :         Workspace printout follows: " << endl;
		w->Print("v");
		assert(0);
	}
	TIterator* it = w->set(parsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) parametersPll[p->GetName()] = p->getVal();
	delete it;
}

///
/// Store the current workspace fit parameters as the
/// free fit result.
///
void ToyTree::storeParsFree()
{
	TIterator* it = w->set(parsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ){
		parametersFree[p->GetName()] = p->getVal();
	}
	delete it;
}

///
/// Store the current workspace fit parameters as the
/// free fit result.
///
void ToyTree::storeParsGau( RooArgSet globalConstraintMeans)
{
	TIterator* it = globalConstraintMeans.createIterator();
	while( RooRealVar* mean = (RooRealVar*) it->Next() ){
		constraintMeans[mean->GetName()] = mean->getVal();
	}
	delete it;
}

///
/// Store the current workspace fit parameters as the
/// scan fit result.
///
void ToyTree::storeParsScan()
{
	TIterator* it = w->set(parsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) parametersScan[p->GetName()] = p->getVal();
	delete it;
}

///
/// Store the fit result parameters as the
/// scan fit result.
///
void ToyTree::storeParsScan(RooFitResult* values)
{
    RooArgList list = values->floatParsFinal();
    list.add(values->constPars());
	TIterator* it = list.createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) {
        parametersScan[p->GetName()] = p->getVal();
    }
	delete it;
}

///
/// Store the current workspace theory parameters.
///
void ToyTree::storeTheory()
{
	TIterator* it = w->set(thName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) theory[p->GetName()] = p->getVal();
	delete it;
}

///
/// Store the current workspace observables.
///
void ToyTree::storeObservables()
{
	TIterator* it = w->set(obsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() ) observables[p->GetName()] = p->getVal();
	delete it;
}

Long64_t ToyTree::GetEntries()
{
	assert(t);
	return t->GetEntries();
}

void ToyTree::GetEntry(Long64_t i)
{
	assert(t);
	t->GetEntry(i);
}

///
/// Get min scanpoint, max scanpoint, and number of steps
/// by looping over the tree.
///
void ToyTree::computeMinMaxN()
{
	if ( scanpointN!=-1 ) return;
	assert(t);
	vector<float> pointsx;
	vector<float> pointsy;
	float _minx =  1e6;
	float _maxx = -1e6;
	float _miny =  1e6;
	float _maxy = -1e6;
	TObjArray* branches = t->GetListOfBranches();
	t->SetBranchStatus("*", 0);
	if(branches->FindObject("scanpoint"))             t->SetBranchStatus("scanpoint",          1);
	if(branches->FindObject("scanpointy"))            t->SetBranchStatus("scanpointy",         1);
	Long64_t nentries = t->GetEntries();
	if ( nentries==0 ) return;
	ProgressBar *pb = NULL;
	if ( !quiet) pb = new ProgressBar(arg, nentries);
	if ( arg->debug ) cout << "ToyTree::computeMinMaxN() : ";
	if ( !quiet) cout << "analysing toys ..." << endl;
	for (Long64_t i = 0; i < nentries; i++){
		// status bar
		if (!quiet) pb->progress();
		t->GetEntry(i);
		// Cut away toys outside a certain range. Also check line 1167 in MethodPluginScan.cpp.
		if ( arg->pluginPlotRangeMin!=arg->pluginPlotRangeMax
				&& !(arg->pluginPlotRangeMin<scanpoint && scanpoint<arg->pluginPlotRangeMax) ) continue;
		_minx = TMath::Min(_minx, scanpoint);
		_maxx = TMath::Max(_maxx, scanpoint);
		_miny = TMath::Min(_miny, scanpointy);
		_maxy = TMath::Max(_maxy, scanpointy);
		pointsx.push_back(scanpoint);
		pointsy.push_back(scanpointy);
	}
	sort(pointsx.begin(), pointsx.end());
	sort(pointsy.begin(), pointsy.end());
	float binWidthx = -1;
	float binWidthy = -1;
	bool foundDifferentBinWidths = false;
	float pointsPrevx = pointsx[0];
	float pointsPrevy = pointsy[0];
	int _nDifferentScanPointsx = 1;
	int _nDifferentScanPointsy = 1;
	for ( unsigned int i=1; i<pointsx.size(); i++ ) {
		if ( pointsx[i] != pointsPrevx ) {
			_nDifferentScanPointsx += 1;
			if ( binWidthx == -1 ) binWidthx = fabs(pointsx[i]-pointsPrevx); // save first bin width so we can compare to others
		}
		if ( pointsy[i] != pointsPrevy ) {
			_nDifferentScanPointsy += 1;
			if ( binWidthy == -1 ) binWidthy = fabs(pointsy[i]-pointsPrevy); // save first bin width so we can compare to others
		}
		if ( binWidthx>-1 && fabs(pointsx[i]-pointsPrevx)>1e-6 && fabs(binWidthx-fabs(pointsx[i]-pointsPrevx))>1e-6 ) foundDifferentBinWidths = true;
		if ( binWidthy>-1 && fabs(pointsy[i]-pointsPrevy)>1e-6 && fabs(binWidthy-fabs(pointsy[i]-pointsPrevy))>1e-6 ) foundDifferentBinWidths = true;
		pointsPrevx = pointsx[i];
		pointsPrevy = pointsy[i];
	}
	if ( arg->debug ) printf("ToyTree::computeMinMaxN() : min(x)=%f, max(x)=%f, n(x)=%i\n", _minx, _maxx, _nDifferentScanPointsx);
	if ( arg->debug && arg->var.size()==2 ) printf("ToyTree::computeMinMaxN() : min(y)=%f, max(y)=%f, n(y)=%i\n", _miny, _maxy, _nDifferentScanPointsy);
	if ( foundDifferentBinWidths ) {
		cout << "\nToyTree::computeMinMaxN() : WARNING : Different bin widths found in the toys!" << endl;
		cout <<   "                                      The p-value histogram will have binning problems.\n" << endl;
	}
	scanpointMin = _minx;
	scanpointMax = _maxx;
	scanpointN = _nDifferentScanPointsx;
	scanpointyMin = _miny;
	scanpointyMax = _maxy;
	scanpointyN = _nDifferentScanPointsy;
	t->SetBranchStatus("*", 1);
	open(); // this is a workaround to fix an issue where the branches get somehow disconnected by reconnecting them
}

///
/// Get minimum of scanpoint variable found in the TTree.
///
float ToyTree::getScanpointMin()
{
	computeMinMaxN();
	return scanpointMin;
}

///
/// Get maximum of scanpoint variable found in the TTree.
///
float ToyTree::getScanpointMax()
{
	computeMinMaxN();
	return scanpointMax;
}

///
/// Get number of different values of the scanpoint variable found in the TTree.
///
int ToyTree::getScanpointN()
{
	computeMinMaxN();
	return scanpointN;
}

///
/// Get minimum of scanpointy variable found in the TTree.
///
float ToyTree::getScanpointyMin()
{
	computeMinMaxN();
	return scanpointyMin;
}

///
/// Get maximum of scanpointy variable found in the TTree.
///
float ToyTree::getScanpointyMax()
{
	computeMinMaxN();
	return scanpointyMax;
}

///
/// Get number of different values of the scanpointy variable found in the TTree.
///
int ToyTree::getScanpointyN()
{
	computeMinMaxN();
	return scanpointyN;
}

///
/// Checks if a certain variable in the work space is an angle.
///
bool ToyTree::isWsVarAngle(TString var)
{
	if ( !w->var(var) ) {
		cout << "ToyTree::isWsVarAngle() : ERROR : variable not found in workspace." << endl;
		assert(0);
	}
	return isAngle(w->var(var));
}

