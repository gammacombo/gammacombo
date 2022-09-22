#include "PValueCorrection.h"

#include <cassert>
#include <cmath>

using namespace std;

PValueCorrection::PValueCorrection(TString _transFunc, bool _verbose):
    transFunc(_transFunc),
    verbose(_verbose)
{
    allowedFuncs.push_back("none");
    allowedFuncs.push_back("p1");
    allowedFuncs.push_back("p1+exp");
    allowedFuncs.push_back("p1+1/x");
    checkValid();
}

PValueCorrection::PValueCorrection(int id, bool _verbose):
    transFunc(""),
    verbose(_verbose)
{
    if (id==0) transFunc = "none";
    else if (id==1) transFunc = "p1";
    else if (id==2) transFunc = "p1+exp";
    else if (id==3) transFunc = "p1+1/x";
    else {
        cout << "ERROR -- PValueCorrection::PValueCorrection(int) -- correction id must be in the range [0,3]" << endl;
        exit(1);
    }
    allowedFuncs.push_back("none");
    allowedFuncs.push_back("p1");
    allowedFuncs.push_back("p1+exp");
    allowedFuncs.push_back("p1+1/x");
    checkValid();
}

PValueCorrection::~PValueCorrection(){

    if (h_pvalue_before) delete h_pvalue_before;
    if (h_pvalue_after) delete h_pvalue_after;
}

void PValueCorrection::setFitParam(int i, double val){
    while ( i < fitParams.size() ) {
        fitParams.push_back(0.);
    }
    if (i==fitParams.size()) fitParams.push_back(val);
    else fitParams[i] = val;
}

void PValueCorrection::checkValid()
{
    if (find(allowedFuncs.begin(),allowedFuncs.end(),transFunc)==allowedFuncs.end()) {
        cout << "ERROR -- " << transFunc << " is not a valid transform function" << endl;
        exit(1);
    }
}

void PValueCorrection::checkParams()
{
    if (transFunc=="none") assert(fitParams.size()==0);
    if (transFunc=="p1") assert(fitParams.size()==2);
    if (transFunc=="p1+exp") assert(fitParams.size()==4);
    if (transFunc=="p1+1/x") assert(fitParams.size()==4);
}

void PValueCorrection::fitHist(TH1 *h)
{
    checkValid();
    fitParams.clear();

    if (transFunc=="none") return;
    if (transFunc=="p1") fitString = "pol1";
    if (transFunc=="p1+exp") fitString = "[0] + [1]*x + [2]*exp(-1.*[3]*x)";
    if (transFunc=="p1+1/x") fitString = "[0] + [1]*x + [2]/(x+[3])";

    fitFunc = TF1("fit",fitString,0.,1.);
    verbose ? h->Fit(&fitFunc,"N") : h->Fit(&fitFunc,"NQ");
    for (int f=0; f<fitFunc.GetNumberFreeParameters(); f++){
        fitParams.push_back(fitFunc.GetParameter(f));
    }

    checkParams();
}

double PValueCorrection::transform(double x)
{
    double y = -999.; // this should be getting return in the range [0,1]
    checkValid();
    checkParams();

    if (transFunc=="none"){
        y = x;
    }
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

void PValueCorrection::printCoverage(float n68, float n95, float n99, float n, TString name){
    cout << "PValueCorrection::printCoverage(): " << name << endl;
  cout<<"  eta=68.27%: alpha="<<Form("%.4f",1.*n68/n)<<" +/- "<<Form("%.4f",sqrt(n68*(n-n68)/n)/n)
      <<"  alpha-eta="<<        Form("%.4f",1.*n68/n-0.6827) << endl
      <<"  eta=95.45%: alpha="<<Form("%.4f",1.*n95/n)<<" +/- "<<Form("%.4f",sqrt(n95*(n-n95)/n)/n)
      <<"  alpha-eta="<<        Form("%.4f",1.*n95/n-0.9545) << endl
      <<"  eta=99.73%: alpha="<<Form("%.4f",1.*n99/n)<<" +/- "<<Form("%.4f",sqrt(n99*(n-n99)/n)/n)
      <<"  alpha-eta="<<        Form("%.4f",1.*n99/n-0.9973) << endl;
}

void PValueCorrection::write(TString fname){
    TFile *f = TFile::Open(fname.Data(),"RECREATE");
    write(f);
    f->Close();
    delete f;

}

void PValueCorrection::write(TFile *f){
    f->cd();
    if (verbose) cout << "PValueCorrection::write() -- writing to file " << f->GetName() << endl;
    fitFunc.SetName(TString(h_pvalue_before->GetName())+"_fit");
    h_pvalue_before->Write();
    h_pvalue_after->Write();
    fitFunc.Write();
}

void PValueCorrection::readFiles(TString name, int id, bool isPlugin){

    // read coverage test files
    name = "root/"+name;
    TSystemDirectory dir(name,name);
    TList *files = dir.GetListOfFiles();
    TChain *fChain = new TChain("tree");
    int foundFiles=0;
    if (files){
        TSystemFile *file;
        TString fname;
        TIter next(files);
        while ((file=(TSystemFile*)next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.Contains(Form("id%d",id)) && fname.EndsWith(".root")) {
                //cout << fname << endl;
                fChain->Add(name+"/"+fname);
                foundFiles++;
            }
        }
    }

    cout << "PValueCorrector::readFiles() -- found " << foundFiles << " files in " << dir.GetName() << " with total of " << fChain->GetEntries() << " entries" << endl;
    cout << "PValueCorrector::readFiles() -- will apply on the fly correction of type ";
    isPlugin ? cout << " plugin"<< endl : cout << " prob" << endl;

    // fill histogram from tree
    h_pvalue_before = new TH1F("h_pvalue_before","p-value",50,0.,1.);
    h_pvalue_after = new TH1F("h_pvalue_after","p-value",50,0.,1.);


    // set up root tree for reading
    float tSol = 0.0;
    float tChi2free = 0.0;
    float tChi2scan = 0.0;
    float tSolGen = 0.0;
    float tPvalue = 0.0;
    fChain->SetBranchAddress("sol",      &tSol);
    fChain->SetBranchAddress("chi2free", &tChi2free);
    fChain->SetBranchAddress("chi2scan", &tChi2scan);
    fChain->SetBranchAddress("solGen",   &tSolGen);
    fChain->SetBranchAddress("pvalue",   &tPvalue);

    // initialize loop variables
    Long64_t nentries = fChain->GetEntries();
    Long64_t nfailed = 0;
    float n68 =0.;
    float n95 =0.;
    float n99 =0.;

    for (Long64_t i = 0; i < nentries; i++)
    {
        fChain->GetEntry(i);
        // apply cuts (we'll count the failed ones later)
        if ( ! (tChi2free > -1e10 && tChi2scan > -1e10
             && tChi2scan-tChi2free>0
             && tSol != 0.0 ///< exclude some pathological jobs from when tSol wasn't set yet
             && tChi2free < 500 && tChi2scan < 500
          )){
            nfailed++;
            continue;
        }
        float pvalue = -999.;
        if (isPlugin) {
            pvalue = tPvalue;
        }
        else {
            pvalue = TMath::Prob(tChi2scan-tChi2free,1);
        }

        if ( pvalue > TMath::Prob(1,1) ) n68++;
        if ( pvalue > TMath::Prob(4,1) ) n95++;
        if ( pvalue > TMath::Prob(9,1) ) n99++;

        h_pvalue_before->Fill(pvalue);
    }

    fitHist(h_pvalue_before);
    if (verbose) printCoverage(n68,n95,n99,float(nentries-nfailed),"Before Correction");

    n68=0.;
    n95=0.;
    n99=0.;
    for (Long64_t i = 0; i < nentries; i++)
    {
        fChain->GetEntry(i);
        // apply cuts (we'll count the failed ones later)
        if ( ! (tChi2free > -1e10 && tChi2scan > -1e10
             && tChi2scan-tChi2free>0
             && tSol != 0.0 ///< exclude some pathological jobs from when tSol wasn't set yet
             && tChi2free < 500 && tChi2scan < 500
          )){
          continue;
        }
        float pvalue = -999.;
        if (isPlugin) {
            pvalue = transform(tPvalue);
        }
        else {
            pvalue = transform(TMath::Prob(tChi2scan-tChi2free,1));
        }

        if ( pvalue > TMath::Prob(1,1) ) n68++;
        if ( pvalue > TMath::Prob(4,1) ) n95++;
        if ( pvalue > TMath::Prob(9,1) ) n99++;

        h_pvalue_after->Fill(pvalue);
    }

    if (verbose) printCoverage(n68,n95,n99,float(nentries-nfailed),"After Correction");
    checkParams();
}
