#include "ToyTree.h"

ToyTree::ToyTree(Combiner *c, TChain* t)
{
	setCombiner(c); // load properties from the combiner
	this->initMembers(t);
	this->storeObs  = true;
	this->storeTh   = true;
}

ToyTree::ToyTree(PDF_Generic_Abs *p, TChain* t){
	assert(p);
	this->comb = NULL;
	this->w = p->getWorkspace();
	this->name = p->getName();
	this->arg = p->getArg();
	this->pdfName  = "pdf_"+p->getPdfName();
	this->obsName  = "obs_"+p->getPdfName();
	this->parsName = "par_"+p->getPdfName();
	this->thName   = "";
	this->initMembers(t);
	this->storeObs  = false;
	this->storeTh   = false;
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
	chi2minToy          = 0.;
	chi2minGlobalToy    = 0.;
	scanbest            = 0.;
	scanbesty           = 0.;
	nrun                = 0.;
	id                  = 0.;
	statusFree          = -5.;
	statusScan          = -5.;
	statusScanData      = -5.;
	nBergerBoos         = 0.;
	BergerBoos_id       = 0.;
	genericProbPValue   = 0.;
	covQualFree         = -2.;
	covQualScan         = -2.;
	covQualScanData     = -2.;
	statusFreePDF       = -5.;
	statusScanPDF       = -5.;
	chi2minToyPDF       = 0.;
	chi2minGlobalToyPDF = 0.;
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
	cout << "ToyTree::writeToFile() : saving root tree to " << fName << " ..." << endl;
	TFile *f = new TFile(fName, "recreate");
	t->Write();
	f->Close();
}

void ToyTree::writeToFile()
{
	assert(t);
	cout << "ToyTree::writeToFile() : saving root tree ..." << endl;
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
	t->Branch("BergerBoos_id",    &BergerBoos_id,     "BergerBoos_id/F");
	t->Branch("chi2min",          &chi2min,           "chi2min/F");
	t->Branch("chi2minGlobal",    &chi2minGlobal,     "chi2minGlobal/F");
	t->Branch("chi2minGlobalToy", &chi2minGlobalToy,  "chi2minGlobalToy/F");
	t->Branch("chi2minToy",       &chi2minToy,        "chi2minToy/F");
	t->Branch("covQualFree",      &covQualFree,       "covQualFree/F");
	t->Branch("covQualScan",      &covQualScan,       "covQualScan/F");
	t->Branch("covQualScanData",  &covQualScanData,   "covQualScanData/F");
	t->Branch("genericProbPValue",&genericProbPValue, "genericProbPValue/F");
	t->Branch("id",               &id,                "id/F");
	t->Branch("nBergerBoos",      &nBergerBoos,       "nBergerBoos/F");
	t->Branch("nrun",             &nrun,              "nrun/F");
	t->Branch("scanbest",         &scanbest,          "scanbest/F");
	t->Branch("scanbesty",        &scanbesty,         "scanbesty/F");
	t->Branch("scanpoint",        &scanpoint,         "scanpoint/F");
	t->Branch("scanpointy",       &scanpointy,        "scanpointy/F");
	t->Branch("statusFree",       &statusFree,        "statusFree/F");
	t->Branch("statusScan",       &statusScan,        "statusScan/F");
	t->Branch("statusScanData",   &statusScanData,    "statusScanData/F");

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
		// gau constraints for B2MuMu Combinations
		if(!this->storeTh){
			delete it; it = w->set("combconstraints")->createIterator();
			while ( RooAbsPdf* gau = (RooAbsPdf*)it->Next() )
			{
				std::vector<TString> pars = Utils::getParsWithName("ean", *gau->getVariables());

				constraintMeans.insert(pair<TString,float>(pars[0],w->var(pars[0])->getVal()));
				t->Branch(TString(pars[0]), &constraintMeans[w->var(pars[0])->GetName()], TString(pars[0])+"/F");
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
	t->SetBranchAddress("BergerBoos_id",    &BergerBoos_id);
	t->SetBranchAddress("chi2min",          &chi2min);
	t->SetBranchAddress("chi2minGlobal",    &chi2minGlobal);
	t->SetBranchAddress("chi2minGlobalToy", &chi2minGlobalToy);
	t->SetBranchAddress("chi2minToy",       &chi2minToy);
	t->SetBranchAddress("covQualFree",      &covQualFree);
	t->SetBranchAddress("covQualScan",      &covQualScan);
	t->SetBranchAddress("covQualScanData",  &covQualScanData);
	t->SetBranchAddress("genericProbPValue",&genericProbPValue);
	t->SetBranchAddress("nBergerBoos",      &nBergerBoos);
	t->SetBranchAddress("scanbest",         &scanbest);
	t->SetBranchAddress("scanbesty",        &scanbesty);
	t->SetBranchAddress("scanpoint",        &scanpoint);
	t->SetBranchAddress("scanpointy",       &scanpointy);
	t->SetBranchAddress("statusFree",       &statusFree);
	t->SetBranchAddress("statusScan",       &statusScan);
	t->SetBranchAddress("statusScanData",   &statusScanData);
	// new conditionally set to values
	if(branches->FindObject("chi2minGlobalToyPDF"))   t->SetBranchAddress("chi2minGlobalToyPDF",&chi2minGlobalToyPDF);
	if(branches->FindObject("chi2minToyPDF"))         t->SetBranchAddress("chi2minToyPDF",      &chi2minToyPDF);
	if(branches->FindObject("covQualFree"))           t->SetBranchAddress("covQualFree",        &covQualFree);
	if(branches->FindObject("covQualScan"))           t->SetBranchAddress("covQualScan",        &covQualScan);
	if(branches->FindObject("covQualScanData"))       t->SetBranchAddress("covQualScanData",    &covQualScanData);
	if(branches->FindObject("statusFreePDF"))         t->SetBranchAddress("statusFreePDF",      &statusFreePDF);
	if(branches->FindObject("statusScanData"))        t->SetBranchAddress("statusScanData",     &statusScanData);
	if(branches->FindObject("statusScanPDF"))         t->SetBranchAddress("statusScanPDF",      &statusScanPDF);
}

///
/// Activate only core branches to speed up reading them.
///
void ToyTree::activateCoreBranchesOnly()
{
	TObjArray* branches = t->GetListOfBranches();
	t->SetBranchStatus("*", 0); // perhaps we need ".*" in certain root versions?
	t->SetBranchStatus("BergerBoos_id", 1);
	t->SetBranchStatus("chi2min", 1);
	t->SetBranchStatus("chi2minGlobal", 1);
	t->SetBranchStatus("chi2minGlobalToy", 1);
	t->SetBranchStatus("chi2minToy", 1);
	t->SetBranchStatus("genericProbPValue", 1);
	t->SetBranchStatus("id", 1);
	t->SetBranchStatus("nBergerBoos", 1);
	t->SetBranchStatus("scanpoint", 1);
	t->SetBranchStatus("scanpointy", 1);
	t->SetBranchStatus("statusFree", 1);
	t->SetBranchStatus("statusScan", 1);
	t->SetBranchStatus("statusScanData", 1);
	if(branches->FindObject("chi2minGlobalToyPDF"))   t->SetBranchStatus("chi2minGlobalToyPDF",1);
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
void ToyTree::storeParsGau()
{
	TIterator* i = w->set("combconstraints")->createIterator();
	while( RooAbsPdf* gau = (RooAbsPdf*)i->Next() ){
		std::vector<TString> pars = Utils::getParsWithName("ean", *gau->getVariables());
		if(pars.size() == 0){
			std::cout << "ERROR in PDF_B_MuMu_CombCMSLHCb_WS140401::initConstraintMeans - No var with sub-string 'ean' found in set" << std::endl;
			return;
		}
		if(pars.size() > 1){
			std::cout << "ERROR in PDF_B_MuMu_CombCMSLHCb_WS140401::initConstraintMeans - More than one var with sub-string 'ean' found in set" << std::endl;
			return;
		}
		constraintMeans[w->var(pars[0])->GetName()] = w->var(pars[0])->getVal();
	}

	delete i;
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
	vector<float> points;
	float _min = 1e6;
	float _max = -1e6;
	t->SetBranchStatus("*", 0);
	t->SetBranchStatus("scanpoint", 1);
	Long64_t nentries = t->GetEntries();
	if ( nentries==0 ) return;
	float printFreq = nentries>101 ? 100 : nentries;  // for the status bar
	for (Long64_t i = 0; i < nentries; i++){
		// status bar
		if ( (((int)i % (int)(nentries/printFreq)) == 0))
			cout << "ToyTree::computeMinMaxN() : reading toys " << Form("%.0f",(float)i/(float)nentries*100.) << "%   \r" << flush;
		t->GetEntry(i);
		// Cut away toys outside a certain range. Also check line 1167 in MethodPluginScan.cpp.
		if ( arg->pluginPlotRangeMin!=arg->pluginPlotRangeMax
				&& !(arg->pluginPlotRangeMin<scanpoint && scanpoint<arg->pluginPlotRangeMax) ) continue;
		_min = TMath::Min(_min, scanpoint);
		_max = TMath::Max(_max, scanpoint);
		points.push_back(scanpoint);
	}
	cout << "ToyTree::computeMinMaxN() : reading toys done.      " << endl;
	sort(points.begin(), points.end());
	float binWidth = -1;
	bool foundDifferentBinWidths = false;
	float pointsPrev = points[0];
	int _n = 1;
	for ( unsigned int i=1; i<points.size(); i++ )
	{
		if ( points[i] != pointsPrev )
		{
			_n+=1;  //  count number of different scan points
			if ( binWidth==-1 ) binWidth = fabs(points[i]-pointsPrev); // save first bin width so we can compare to others
		}
		if ( binWidth>-1 && fabs(points[i]-pointsPrev)>1e-6
				&& fabs(binWidth-fabs(points[i]-pointsPrev))>1e-6 ) foundDifferentBinWidths = true;
		pointsPrev = points[i];
	}
	if ( arg->debug ) printf("ToyTree::computeMinMaxN() : min=%f, max=%f, n=%i\n", _min, _max, _n);
	if ( foundDifferentBinWidths )
	{
		cout << "\nToyTree::computeMinMaxN() : WARNING : Different bin widths found in the toys!" << endl;
		cout <<   "ToyTree::computeMinMaxN() : WARNING : The 1-CL histogram will have binning problems.\n" << endl;
	}
	scanpointMin = _min;
	scanpointMax = _max;
	scanpointN = _n;
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

