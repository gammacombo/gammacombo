/*
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 */

#include "MethodPluginScan.h"
#include "TArrow.h"
#include "TLatex.h"


///
/// Initialize from a previous Prob scan, setting the profile
/// likelihood. This should be the default.
///
MethodPluginScan::MethodPluginScan(MethodProbScan* s)
: MethodAbsScan(s->getCombiner())
{
    methodName = "Plugin";
    title = s->getTitle();
    scanVar1 = s->getScanVar1Name();
    scanVar2 = s->getScanVar2Name();
    profileLH = s;
    parevolPLH = profileLH;
    setSolutions(s->getSolutions());
    setChi2minGlobal(s->getChi2minGlobal());
    chi2minBkg = s->getChi2minBkg();
    obsDataset = new RooDataSet("obsDataset", "obsDataset", *w->set(obsName));
    obsDataset->add(*w->set(obsName));
    nToys = arg->ntoys;
    nPoints1d  = arg->npointstoy;
    nPoints2dx = arg->npointstoy;
    nPoints2dy = arg->npointstoy;
    BkgToys = NULL;
}

///
/// Constructor, mainly to ensure compatibility with MethodDatasetsPluginScan
///
MethodPluginScan::MethodPluginScan(MethodProbScan* s, PDF_Datasets* pdf, OptParser* opt)
    : MethodAbsScan(opt),
    nToys(opt->ntoys)
    {
        methodName = "Plugin";
        obsName = pdf->getObsName();
        w = pdf->getWorkspace();
        title = s->getTitle();
        scanVar1 = s->getScanVar1Name();
        scanVar2 = s->getScanVar2Name();
        profileLH = s;
        parevolPLH = profileLH;
        setSolutions(s->getSolutions());
        setChi2minGlobal(s->getChi2minGlobal());
        chi2minBkg = s->getChi2minBkg();
        obsDataset = new RooDataSet("obsDataset", "obsDataset", *w->set(obsName));
        obsDataset->add(*w->set(obsName));
        nToys = opt->ntoys;
        BkgToys = NULL;

    };

///
/// Initialize from a Combiner object. This is more difficult,
/// as now we have to set the profile likelihood explicitly,
/// and probably some other things...
/// But to just compute a p-value at a certain point (getPvalue1d())
/// this is fine.
///
    MethodPluginScan::MethodPluginScan(Combiner* comb)
: MethodAbsScan(comb)
{
    methodName = "Plugin";
    title = comb->getTitle();
    profileLH = 0;
    parevolPLH = 0;
    obsDataset = new RooDataSet("obsDataset", "obsDataset", *comb->getWorkspace()->set(obsName));
    obsDataset->add(*comb->getWorkspace()->set(obsName));
    nToys = arg->ntoys;
    nPoints1d  = arg->npointstoy;
    nPoints2dx = arg->npointstoy;
    nPoints2dy = arg->npointstoy;
    BkgToys = NULL;
}

///
/// Set the parameter evolution over the profile likelihood
/// that was previously computed by a MethodProbScan scanner. Usually, the PLH is used that
/// is provided to the constructor. Use this method to use a different evolution for toy
/// generation (Hybrid Plugin).
/// \todo This setting is currently being ignored by the DatasetsPluginScan
///
void MethodPluginScan::setParevolPLH(MethodProbScan* s)
{
    assert(s);
    parevolPLH = s;
}

///
/// Helper function for scan1d(). Gets point in parameter space (in form
/// of a RooFitResult) at which the plugin toy should be generated.
/// The point is taken from the externally provided scanner parevolPLH,
/// which is usually just the profile likelihood, but can also be set
/// to the profile likelihood of a different combination.
/// A couple of sanity checks are performed.
///
/// \param scanpoint - value of the scan parameter for which the point
///                    should be found
///
RooSlimFitResult* MethodPluginScan::getParevolPoint(float scanpoint)
{
    // get point in nuisance parameter space where the toys get generated at
    int iCurveRes = parevolPLH->getHCL()->FindBin(scanpoint)-1;
    if ( !parevolPLH->curveResults[iCurveRes] ){
        cout << "MethodPluginScan::getParevolPoint() : ERROR : curve result not found, id=" << iCurveRes;
        cout << ", scanpoint=" << scanpoint << endl;
        exit(1);
    }

    // check that the scan variable is indeed present
    RooArgList list = parevolPLH->curveResults[iCurveRes]->floatParsFinal();
    list.add(parevolPLH->curveResults[iCurveRes]->constPars());
    RooRealVar* var = (RooRealVar*)list.find(scanVar1);
    if ( !var ){
        cout << "MethodPluginScan::getParevolPoint() : ERROR : "
                                   "scan variable not found in parameter evolution, var=" << scanVar1 << endl;
        cout << "MethodPluginScan::getParevolPoint() : Printout follows:" << endl;
        parevolPLH->curveResults[iCurveRes]->Print();
        exit(1);
    }

    // check if the scan variable here differs from that of
    // the external curve
    if ( fabs((scanpoint-var->getVal())/scanpoint) > 0.01 ){
        cout << "MethodPluginScan::getParevolPoint() : WARNING : "
                                   "scanpoint and parameter evolution point differ by more than 1%:" << endl;
        cout << scanpoint << " " << var->getVal() << endl;
    }

    return parevolPLH->curveResults[iCurveRes];
}

///
/// Generate toys.
///
/// \param nToys - generate this many toys
///
RooDataSet* MethodPluginScan::generateToys(int nToys)
{
    RooRandom::randomGenerator()->SetSeed(0);
    RooMsgService::instance().setStreamStatus(0,kFALSE);
    RooMsgService::instance().setStreamStatus(1,kFALSE);
    RooDataSet* dataset = w->pdf(pdfName)->generate(*w->set(obsName), nToys, AutoBinned(false));
    RooMsgService::instance().setStreamStatus(0,kTRUE);
    RooMsgService::instance().setStreamStatus(1,kTRUE);

    // Test toy generation - print out the first 10 toys to stdout.
    // Triggered by --qh 5
    if ( arg->isQuickhack(5) ){
        if ( w->var("kD_k3pi") )  cout << "kD_k3pi=" << w->var("kD_k3pi")->getVal() << endl;
        if ( w->var("dD_k3pi") )  cout << "dD_k3pi=" << w->var("dD_k3pi")->getVal() << endl;
        if ( w->var("kD_kskpi") ) cout << "kD_kskpi=" << w->var("kD_kskpi")->getVal() << endl;
        if ( w->var("dD_kskpi") ) cout << "dD_kskpi=" << w->var("dD_kskpi")->getVal() << endl;
        for ( int j = 0; j<10 && j<nToys; j++ ){
            const RooArgSet* toyData = dataset->get(j);
            toyData->Print("v");
        }
    }

    // The following code is an embarrasment. Something goes wrong
    // in RooFit's toy generation when a Th2F PDF is used. We try to
    // work around it.
    // Sometimes there is an error from TFoam (the TH2F clearly isn't zero):
    // Error in <TFoam::MakeActiveList>: Integrand function is zero
    // Then it proceeds "generating" observables that are, in every toy, set to their boundaries.
    // This happens only for kD_k3pi_obs and kD_kskpi_obs
    // Workaround: If it happens, flucutate the parameters of the histogram
    // ever so slightly and regenerate.

    // read the generated values for one variable from the
    // first two toys
  //
  vector<TString> affected_var;
  affected_var.push_back("kD_k3pi");
  affected_var.push_back("kD_kskpi");

  for ( vector<TString>::iterator aff_var_it = affected_var.begin(); aff_var_it != affected_var.end(); aff_var_it++) {

    TString aff_obs = *aff_var_it + "_obs";

    bool hasAffObs = false;
    float generatedValues[2];
    for ( int i=0; i<2; i++ ){
      const RooArgSet* toyData = dataset->get(i);
      TIterator* it = toyData->createIterator();
      while(RooRealVar* var = (RooRealVar*)it->Next()){
        if ( TString(var->GetName()).Contains(aff_obs) ){
          hasAffObs = true;
          generatedValues[i] = var->getVal();
          continue;
        }
      }
      delete it;
    }

    // check if they are the same, if so, fluctuate and regenerate
    if ( hasAffObs && generatedValues[0]==generatedValues[1] ){
      delete dataset;
      TString dD_aff_var = *aff_var_it;
      dD_aff_var.ReplaceAll("kD","dD");

      cout << aff_obs << " GENERATION ERROR AT " << *aff_var_it << "=" << w->var(*aff_var_it)->getVal()
        << " " << dD_aff_var << "=" << w->var(dD_aff_var)->getVal() << endl;
      TRandom3 r;
      w->var(*aff_var_it)->setVal(r.Gaus(w->var(*aff_var_it)->getVal(),0.05));
      w->var(dD_aff_var)->setVal(r.Gaus(w->var(dD_aff_var)->getVal(),0.04));
      cout << aff_obs << " SECOND GENERATION AT " << *aff_var_it << "=" << w->var(*aff_var_it)->getVal()
      << " " << dD_aff_var << "=" << w->var(dD_aff_var)->getVal() << endl;

      RooMsgService::instance().setStreamStatus(0,kFALSE);
      RooMsgService::instance().setStreamStatus(1,kFALSE);
      dataset = w->pdf(pdfName)->generate(*w->set(obsName), nToys, AutoBinned(false));
      RooMsgService::instance().setStreamStatus(0,kTRUE);
      RooMsgService::instance().setStreamStatus(1,kTRUE);       for ( int i=0; i<2; i++ ){
        const RooArgSet* toyData = dataset->get(i);
        TIterator* it = toyData->createIterator();
        while(RooRealVar* var = (RooRealVar*)it->Next()){
          if ( TString(var->GetName()).Contains(aff_obs) ){
            generatedValues[i] = var->getVal();
            continue;
          }
        }
        delete it;
      }
      cout << aff_obs << " NEW VALUES : toy 0: " << generatedValues[0] << " toy 1: " << generatedValues[1] << endl;
    }

  }

    return dataset;
}

///
/// Compute the p-value at a certain point in parameter space using
/// the plugin method. The precision of the p-value will depend on
/// the number of toys that were generated, more than 100 should
/// be a good start (ntoys command line option).
///
/// \param plhScan  Defines the point in parameter space (=nuisances) at
///                 which the p-value gets computed. That is, it all toys
///                 are being generated at this point. It also defines the
///                 value of the scan parameter and provides the "scan"
///                 chi2 value of "the data".
/// \param chi2minGlobal chi2 value at global minimum
/// \param t        Stores all toys and fit results that are being generated.
///                 If a ToyTree is provided, entries will be added to the
///                 existing tree, and all new entries will have the
///                 "id" branch set to the id parameter. If no tree is
///                 provided, a new one will be created. This will be saved
///                 to disk and deleted from memory afterwards.
/// \param id       An arbitrary number identifying this particular run of
///                 getPvalue1d(). E.g. in the coverage tests, we need to
///                 run many times for different toy data sets that replace
///                 the nominal "data". The ToyTree also contains a 'run' branch
///                 that holds the run number (=batch job).
/// \param f        A fitter object. If not given, a new one will be created.
///                 It may be useful to use an external fitter so that the
///                 fitter object can compute some fit statistics for an entire
///                 1-CL scan.
/// \param pb       A progress bar object used to print nice progress output.
/// \return         the p-value.
///
void MethodPluginScan::computePvalue1d(RooSlimFitResult* plhScan, double chi2minGlobal, ToyTree* t, int id,
        Fitter* f, ProgressBar *pb)
{
    // Check inputs.
    assert(plhScan);
    assert(t);
    assert(f);
    assert(pb);
    if ( !plhScan->hasParameter(scanVar1) ){
        cout << "MethodPluginScan::getPvalue1d() : ERROR : scan variable not found in plhScan. Exit." << endl;
        assert(0);
    }

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    FitResultCache frCache(arg);
    frCache.storeParsAtFunctionCall(w->set(parsName));
    frCache.initRoundRobinDB(w->set(parsName));

    // Set nuisances. This is the point in parameter space where
    // the toys need to be generated.
    setParameters(w, parsName, plhScan, true);

  // Kenzie-Cousins-Highland (randomize nuisance parameters within a uniform range)
  if ( arg->isAction("uniform") ) {
    //   set parameter ranges to their bb range (should be something wide 95, 99% CL)
    const RooArgSet* pars = w->set(toysName) ? w->set(toysName) : w->set(parsName);
    TIterator* it = pars->createIterator();
    while(RooRealVar* var = (RooRealVar*)it->Next()){
      setLimit( var, "bboos" );
    }
    if (verbose) {
      cout << "Uniform generating from:" << endl;
      pars->Print("v");
    }
    randomizeParameters(w, parsName);
    if (verbose) {
      cout << "Set:" << endl;
      w->set(parsName)->Print("v");
    }
  }

  // Cousins-Highland (randomize nuisance parameters according to a Gaussian with their
  // best fit value and uncertainty from the PLH scan)
  if ( arg->isAction("gaus") ) {
    if (verbose) {
      cout << "Gaussian generating from:" << endl;
      plhScan->floatParsFinal().Print("v");
    }
    randomizeParametersGaussian(w, toysName, plhScan);
    if (verbose) {
      cout << "Set:" << endl;
      w->set(parsName)->Print("v");
    }
  }

    // save nuisances for start parameters
    frCache.storeParsAtGlobalMin(w->set(parsName));

    // set and fix scan point
    RooRealVar *par = w->var(scanVar1);
    par->setConstant(true);
    float scanpoint = par->getVal();

    // get the chi2 of the data
    t->scanpoint = scanpoint;
    t->id = id;
    t->storeParsPll();
    t->storeTheory();
    t->chi2min = plhScan->minNll();
    t->chi2minGlobal = chi2minGlobal;
    t->chi2minBkg = chi2minBkg;

    // Importance sampling
    int nActualToys = nToys;
    if ( arg->importance ){
        float plhPvalue = TMath::Prob(t->chi2min - t->chi2minGlobal,1);
        nActualToys = nToys*importance(plhPvalue);
        pb->skipSteps(nToys-nActualToys);
    }

    // Draw all toy datasets in advance. This is much faster.
    RooDataSet *toyDataSet = generateToys(nActualToys);
    if(id==0) BkgToys = new RooDataSet(*toyDataSet,"BkgToys");

    for ( int j = 0; j<nActualToys; j++ )
    {
        // status bar
        pb->progress();

        //
        // 1. Generate toys
        //    (or select the right one)
        //
        const RooArgSet* toyData = toyDataSet->get(j);
        setParameters(w, obsName, toyData);
        t->storeObservables();

        //
        // 2. scan fit
        //
        par->setVal(scanpoint);
        par->setConstant(true);
        f->setStartparsFirstFit(frCache.getRoundRobinNminus(0));
        f->setStartparsSecondFit(frCache.getParsAtGlobalMin());
        f->fit();
        if ( f->getStatus()==1 ){
            f->setStartparsFirstFit(frCache.getRoundRobinNminus(1));
            f->setStartparsSecondFit(frCache.getRoundRobinNminus(2));
            f->fit();
        }
        t->chi2minToy = f->getChi2();
        t->statusScan = f->getStatus();
        t->storeParsScan();
        // for CLs method
        if(id==0){
            t->chi2minBkgBkgToy = f->getChi2();
            chi2minBkgBkgToysvector.push_back(f->getChi2());
            // std::cout << id << "\t" << t->chi2minBkgBkgToy << std::endl;
        }
        else{
            t->chi2minBkgBkgToy = chi2minBkgBkgToysvector[j];
            // std::cout << id << "\t" << t->chi2minBkgBkgToy << std::endl;
        }

        //
        // 3. free fit
        //
        par->setConstant(false);
        f->fit();
        if ( f->getStatus()==1 ){
            f->fit();
        }
        t->chi2minGlobalToy = f->getChi2();
        t->statusFree = f->getStatus();
        t->scanbest = ((RooRealVar*)w->set(parsName)->find(scanVar1))->getVal();
        t->storeParsFree();
        if(id==0){
            t->chi2minGlobalBkgToy = f->getChi2();
            chi2minGlobalBkgToysvector.push_back(f->getChi2());
            // std::cout << id << "\t" << t->chi2minGlobalBkgToy << std::endl;
        }
        else{
            t->chi2minGlobalBkgToy = chi2minGlobalBkgToysvector[j];
            // std::cout << id << "\t" << t->chi2minGlobalBkgToy << std::endl;
        }

        //
        // Bkg.1 generate bkg-only toys (for CLs method)
        //          (or select the right one)
        //
        const RooArgSet* toyDataBkg = BkgToys->get(j);
        setParameters(w, obsName, toyDataBkg);
        // t->storeObservables();

        //
        // Bkg.2 fit to bkg-only toys (for CLs method)
        //

        par->setVal(scanpoint);
        par->setConstant(true);
        f->setStartparsFirstFit(frCache.getRoundRobinNminus(0));
        f->setStartparsSecondFit(frCache.getParsAtGlobalMin());
        f->fit();
        if ( f->getStatus()==1 ){
            f->setStartparsFirstFit(frCache.getRoundRobinNminus(1));
            f->setStartparsSecondFit(frCache.getRoundRobinNminus(2));
            f->fit();
        }
        t->chi2minBkgToy = f->getChi2();
        // t->statusScan = f->getStatus();
        par->setConstant(false);

        //
        // 4. store
        //
        if ( t->statusFree==0 ) frCache.storeParsRoundRobin(w->set(parsName));
        t->fill();
    }

    // clean up
    setParameters(w, parsName, frCache.getParsAtFunctionCall());
    setParameters(w, obsName, obsDataset->get(0));
    delete toyDataSet;
}

double MethodPluginScan::getPvalue1d(RooSlimFitResult* plhScan, double chi2minGlobal, ToyTree* t, int id)
{
    // Create a ToyTree to store the results of all toys
    // (or use the supplied one so we can have a ToyTree
    // that holds a full scan).
    ToyTree *myTree = 0;
    if ( !t ){
        myTree = new ToyTree(combiner);
        myTree->init();
    }
    else{
        myTree = t;
    }

    // Create a fitter
    Fitter *myFit = new Fitter(arg, w, combiner->getPdfName());

    // Create a progress bar
    ProgressBar *myPb = new ProgressBar(arg, nToys);

    // do the work
    cout << "MethodPluginScan::getPvalue1d() : computing p-value ..." << endl;
    computePvalue1d(plhScan, chi2minGlobal, myTree, id, myFit, myPb);

    // compute p-value
    if ( arg->controlplot ) {
        ControlPlots cp(myTree);
        cp.ctrlPlotChi2();
    }
    TH1F *h = analyseToys(myTree, id);
    float scanpoint = plhScan->getParVal(scanVar1);
    double pvalue = h->GetBinContent(h->FindBin(scanpoint));
    delete h;

    if ( !t ){
        myTree->writeToFile(Form("root/getPvalue1d_"+name+"_"+scanVar1+"_run%i.root", arg->nrun));
        delete myTree;
    }
    delete myFit;
    delete myPb;
    return pvalue;
}

///
/// Perform the 1d Plugin scan.
/// Saves chi2 values in a root tree, together with the full fit result for each toy.
/// If option --lightfiles is given, the tree will only contain the essentials (min Chi2).
/// If a combined PDF for the toy generation is given by setParevolPLH(), this
/// will be used to generate the toys.
///
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
int MethodPluginScan::scan1d(int nRun)
{
    Fitter *myFit = new Fitter(arg, w, combiner->getPdfName());
    RooRandom::randomGenerator()->SetSeed(0);

    // Set limit to all parameters.
    combiner->loadParameterLimits();

    // Define scan parameter and scan range.
    RooRealVar *par = w->var(scanVar1);
    assert(par);
    float min = hCL->GetXaxis()->GetXmin();
    float max = hCL->GetXaxis()->GetXmax();

    if ( arg->verbose ){
        cout << endl;
        cout << "Plugin configuration:" << endl;
        cout << "  combination:    " << title << endl;
        cout << "  scan variable:  " << scanVar1 << endl;
        cout << "  scan range:     " << min << " ... " << max << endl;
        cout << "  scan steps:     " << nPoints1d << endl;
        cout << "  par. evolution: " << (parevolPLH!=profileLH?parevolPLH->getTitle():"same as combination") << endl;
        cout << "  nToys:          " << nToys << endl;
        cout << endl;
    }

    // Set up toy root tree
    ToyTree t(combiner);
    t.init();
    t.nrun = nRun;

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    FitResultCache frCache(arg);
    frCache.storeParsAtFunctionCall(w->set(parsName));

    // for the progress bar: if more than 100 steps, show 50 status messages.
    int allSteps = nPoints1d*nToys;
    ProgressBar *pb = new ProgressBar(arg, allSteps);

    // start scan
    if ( arg->debug ) cout << "MethodPluginScan::scan1d() : ";
    cout << "PLUGIN scan starting ..." << endl;
    for ( int i=0; i<nPoints1d; i++ )
    {
        float scanpoint = min + (max-min)*(double)i/(double)nPoints1d + hCL->GetBinWidth(1)/2.;
        t.scanpoint = scanpoint;

        // don't scan in unphysical region
        if ( scanpoint < par->getMin() || scanpoint > par->getMax() ) continue;

        // Get nuisances. This is the point in parameter space where
        // the toys need to be generated.
        RooSlimFitResult* plhScan = getParevolPoint(scanpoint);

        // do the work
        computePvalue1d(plhScan, profileLH->getChi2minGlobal(), &t, i, myFit, pb);

        // reset
        setParameters(w, parsName, frCache.getParsAtFunctionCall());
        setParameters(w, obsName, obsDataset->get(0));
    }

    if ( arg->debug ) myFit->print();
    TString dirname = "root/scan1dPlugin";
  if ( arg->isAction("bb") ) dirname += "BergerBoos";
  if ( arg->isAction("uniform") ) dirname += "Uniform";
  if ( arg->isAction("gaus") ) dirname += "Gaus";
  dirname += "_"+name+"_"+scanVar1;
    system("mkdir -p "+dirname);
  TString fname = "/scan1dPlugin";
  if ( arg->isAction("bb") ) fname += "BergerBoos";
  if ( arg->isAction("uniform") ) fname += "Uniform";
  if ( arg->isAction("gaus") ) fname += "Gaus";
  fname += Form("_"+name+"_"+scanVar1+"_run%i.root",nRun);
    t.writeToFile((dirname+fname).Data());
    delete myFit;
    delete pb;
    return 0;
}

///
/// Perform the 2d Plugin scan.
/// Saves chi2 values in a root tree, together with the full fit result for each toy.
/// \param nRun Part of the root tree file name to facilitate parallel production.
///
void MethodPluginScan::scan2d(int nRun)
{
    RooRandom::randomGenerator()->SetSeed(0);

    // Set limit to all parameters.
    combiner->loadParameterLimits();

    // Define scan parameters and scan range.
    RooRealVar *par1 = w->var(scanVar1);
    assert(par1);
    float min1 = hCL2d->GetXaxis()->GetXmin();
    float max1 = hCL2d->GetXaxis()->GetXmax();
    RooRealVar *par2 = w->var(scanVar2);
    assert(par2);
    float min2 = hCL2d->GetYaxis()->GetXmin();
    float max2 = hCL2d->GetYaxis()->GetXmax();

    if ( arg->verbose ){
        cout << endl;
        cout << "Plugin configuration:" << endl;
        cout << "  combination:           " << title << endl;
        cout << "  scan variables:        x=" << scanVar1 << ", y=" << scanVar2 << endl;
        cout << "  scan range:            x=[" << min1 << ", " << max1 << "], y=[" << min2 << ", " << max2 << "]" << endl;
        cout << "  scan steps:            x=" << nPoints2dx << ", y=" << nPoints2dy << endl;
        cout << "  par. evolution:        " << (parevolPLH!=profileLH?parevolPLH->getTitle():"same as combination") << endl;
        cout << "  par. evol. scan steps: x=" << profileLH->curveResults2d[1].size() << ", y=" << profileLH->curveResults2d.size() << endl;
        cout << "  nToys :                " << nToys << endl;
        cout << endl;
    }

    // Set up root tree.
    ToyTree t(combiner);
    t.init();
    t.nrun = nRun;

    // Save parameter values that were active at function
    // call. We'll reset them at the end to be transparent
    // to the outside.
    FitResultCache frCache(arg);
    frCache.storeParsAtFunctionCall(w->set(parsName));

    // for the status bar
    int allSteps = nPoints2dx*nPoints2dy*nToys;
    ProgressBar *pb = new ProgressBar(arg, allSteps);

    // limit number of warnings
    int nWarnExtPointDiffer = 0;
    int nWarnExtPointDifferMax = 10;

    //
    // 1. assume we have already the global minimum
    //

    // start scan
    cout << "MethodPluginScan::scan2d() : starting ..." << endl;
    for ( int i1=0; i1<nPoints2dx; i1++ ) {
        for ( int i2=0; i2<nPoints2dy; i2++ )
        {
            float scanpoint1 = min1 + (max1-min1)*(double)i1/(double)nPoints2dx + hCL2d->GetXaxis()->GetBinWidth(1)/2.;
            float scanpoint2 = min2 + (max2-min2)*(double)i2/(double)nPoints2dy + hCL2d->GetYaxis()->GetBinWidth(1)/2.;
            t.scanpoint = scanpoint1;
            t.scanpointy = scanpoint2;

            // don't scan in unphysical region
            if ( scanpoint1 < par1->getMin() || scanpoint1 > par1->getMax() ) continue;
            if ( scanpoint2 < par2->getMin() || scanpoint2 > par2->getMax() ) continue;

            // Get the global chi2 minimum from the fit to data.
            t.chi2minGlobal = profileLH->getChi2minGlobal();

            // Get nuisances. This is the point in parameter space where
            // the toys need to be generated.
            RooArgList *extCurveResult = 0;
            {
                int iCurveRes1 = profileLH->getHCL2d()->GetXaxis()->FindBin(scanpoint1)-1;
                int iCurveRes2 = profileLH->getHCL2d()->GetYaxis()->FindBin(scanpoint2)-1;
                if ( !profileLH->curveResults2d[iCurveRes1][iCurveRes2] ) {
                    printf("MethodPluginScan::scan2d() : WARNING : curve result not found, "
                            "id=[%i,%i], val=[%f,%f]\n", iCurveRes1, iCurveRes2, scanpoint1, scanpoint2);
                }
                else {
                    if ( arg->debug ){
                        printf("MethodPluginScan::scan2d() : loading start parameters from external 1-CL curve: "
                                "id=[%i,%i], val=[%f,%f]\n", iCurveRes1, iCurveRes2, scanpoint1, scanpoint2);
                    }
                    extCurveResult = new RooArgList(profileLH->curveResults2d[iCurveRes1][iCurveRes2]->floatParsFinal());

          // Set nuisances. This is the point in parameter space where
          // the toys need to be generated.
          setParameters(w, parsName, extCurveResult);

          // Kenzie-Cousins-Highland (randomize nuisance parameters within a uniform range)
          if ( arg->isAction("uniform") ) {
            // set parameter ranges to their bb range (should be something wide 95, 99% CL)
            const RooArgSet* pars = w->set(toysName) ? w->set(toysName) : w->set(parsName);
            TIterator* it = pars->createIterator();
            while ( RooRealVar* var = (RooRealVar*)it->Next() ) {
              setLimit( var, "bboos" );
            }
            if (verbose) {
              cout << "Uniform generating from:" << endl;
              pars->Print("v");
            }
            randomizeParameters(w, parsName);
            if (verbose) {
              cout << "Set:" << endl;
              w->set(parsName)->Print("v");
            }
          }

          // Cousins-Highland (randomize nuisance parameters according to a Gaussian with their
          // best fit value and uncertainty from the PLH scan)
          if ( arg->isAction("gaus") ) {
            if (verbose) {
              cout << "Gaussian generating from:" << endl;
              profileLH->curveResults2d[iCurveRes1][iCurveRes2]->floatParsFinal().Print("v");
            }
            randomizeParametersGaussian(w, toysName, profileLH->curveResults2d[iCurveRes1][iCurveRes2]);
            if (verbose) {
              cout << "Set:" << endl;
              w->set(parsName)->Print("v");
            }
          }

                    t.chi2min = profileLH->curveResults2d[iCurveRes1][iCurveRes2]->minNll();

                    // check if the scan variable here differs from that of
                    // the external curve
                    RooArgList list = profileLH->curveResults2d[iCurveRes1][iCurveRes2]->floatParsFinal();
                    list.add(profileLH->curveResults2d[iCurveRes1][iCurveRes2]->constPars());
                    RooRealVar* var1 = (RooRealVar*)list.find(scanVar1);
                    RooRealVar* var2 = (RooRealVar*)list.find(scanVar2);
                    if ( var1 && var2 ) {
                        // print warnings
                        if ( fabs((scanpoint1-var1->getVal())/scanpoint1) > 0.01 || fabs((scanpoint2-var2->getVal())/scanpoint2) > 0.01 ) {
                                if ( nWarnExtPointDiffer<nWarnExtPointDifferMax || arg->debug ) {
                                        if ( fabs((scanpoint1-var1->getVal())/scanpoint1) > 0.01 )
                                                cout << "MethodPluginScan::scan2d() : WARNING : scanpoint1 and external point differ by more than 1%: "
                                                                                      << "scanpoint1=" << scanpoint1 << " var1=" << var1->getVal() << endl;
                                        if ( fabs((scanpoint2-var2->getVal())/scanpoint2) > 0.01 )
                                                cout << "MethodPluginScan::scan2d() : WARNING : scanpoint2 and external point differ by more than 1%: "
                                                                                      << "scanpoint2=" << scanpoint2 << " var2=" << var2->getVal() << endl;
                                }
                                if ( nWarnExtPointDiffer==0 ) {
                                        cout << endl;
                                        cout << "                                       Try using the same number of scan points for both Plugin and Prob." << endl;
                                        cout << "                                       See --npoints, --npoints2dx, --npoints2dy, --npointstoy" << endl;
                                        cout << endl;
                                }
                                if ( nWarnExtPointDiffer==nWarnExtPointDifferMax ) {
                                        cout << "MethodPluginScan::scan2d() : WARNING : scanpoint1 and external point differ by more than 1%: [further warnings suppressed.]" << endl;
                                }
                                nWarnExtPointDiffer++;
                        }
                    }
                    else {
                        cout << "MethodPluginScan::scan2d() : WARNING : variable 1 or 2 not found"
                                              ", var1=" << scanVar1 << ", var2=" << scanVar1 << endl;
                        cout << "MethodPluginScan::scan2d() : Printout follows:" << endl;
                        profileLH->curveResults2d[iCurveRes1][iCurveRes2]->Print();
                        exit(1);
                    }
                }
            }

            // set and fix scan point
            par1->setVal(scanpoint1);
            par2->setVal(scanpoint2);
            par1->setConstant(true);
            par2->setConstant(true);

            // save nuisances to ToyTree tree
            t.storeParsPll();
            t.storeTheory();

            // Draw toy datasets in advance. This is much faster.
            RooDataSet *toyDataSet = generateToys(nToys);

            for ( int j=0; j<nToys; j++ )
            {
                // status bar
                pb->progress();

                //
                // 1. Load toy dataset
                //
                const RooArgSet* toyData = toyDataSet->get(j);
                setParameters(w, obsName, toyData);
                t.storeObservables();

                //
                // 2. scan fit
                //
                par1->setVal(scanpoint1);
                par2->setVal(scanpoint2);
                par1->setConstant(true);
                par2->setConstant(true);
                RooFitResult *r;
                if ( !arg->scanforce ) r = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
                else                   r = fitToMinForce(w, name);
                t.chi2minToy = r->minNll();
                t.statusScan = 0;
                t.storeParsScan();
                delete r;

                //
                // 3. free fit
                //
                par1->setVal(scanpoint1);
                par2->setVal(scanpoint2);
                par1->setConstant(false);
                par2->setConstant(false);
                if ( !arg->scanforce ) r = fitToMinBringBackAngles(w->pdf(pdfName), false, -1);
                else                   r = fitToMinForce(w, name);
                t.chi2minGlobalToy = r->minNll();
                t.statusFree = 0;
                t.scanbest = ((RooRealVar*)w->set(parsName)->find(scanVar1))->getVal();
                t.scanbesty = ((RooRealVar*)w->set(parsName)->find(scanVar2))->getVal();
                t.storeParsFree();
                delete r;

                //
                // 4. store
                //
                t.fill();
            }

            // reset
            setParameters(w, parsName, frCache.getParsAtFunctionCall());
            setParameters(w, obsName, obsDataset->get(0));
            delete toyDataSet;
        }
    }

    // save tree
    TString dirname = "root/scan2dPlugin";
  if ( arg->isAction("bb") ) dirname += "BergerBoos";
  if ( arg->isAction("uniform") ) dirname += "Uniform";
  if ( arg->isAction("gaus") ) dirname += "Gaus";
  dirname += "_"+name+"_"+scanVar1+"_"+scanVar2;
    system("mkdir -p "+dirname);
  TString fname = "/scan2dPlugin";
  if ( arg->isAction("bb") ) fname += "BergerBoos";
  if ( arg->isAction("uniform") ) fname += "Uniform";
  if ( arg->isAction("gaus") ) fname += "Gaus";
  fname += Form("_"+name+"_"+scanVar1+"_"+scanVar2+"_run%i.root",nRun);
    t.writeToFile((dirname+fname).Data());
    delete pb;
}

///
/// Analyse toys that were written either by a scan
/// or a by getPvalue(). Create a histogram of p-values
/// vs scanoints with as many bins for the scanpoint as
/// found in the ToyTree.
///
/// \param t    A ToyTree set up for reading (open() was called).
/// \param id   Only consider entries that have the id branch set to this value.
///             This is used e.g. by the coverage tests to distinguish the different
///             coverage toys.
///             Default is -1 which uses all entries regardless of their id.
/// \return     A new histogram that contains the p-values vs the scanpoint.
///
TH1F* MethodPluginScan::analyseToys(ToyTree* t, int id)
{
    /// \todo replace this such that there's always one bin per scan point, but still the range is the scan range.
    /// \todo Also, if we use the min/max from the tree, we have the problem that they are not exactly
    /// the scan range, so that the axis won't show the lowest and highest number.
    /// \todo If the scan range was changed after the toys were generate, we absolutely have
    /// to derive the range from the root files - else we'll have bining effects.

    float halfBinWidth = (t->getScanpointMax()-t->getScanpointMin())/(float)t->getScanpointN()/2;
    if ( t->getScanpointN()==1 ) halfBinWidth = 1.;
    TH1F *hCL          = new TH1F(getUniqueRootName(), "hCL", t->getScanpointN(), t->getScanpointMin()-halfBinWidth, t->getScanpointMax()+halfBinWidth);
    TH1F *h_better     = (TH1F*)hCL->Clone("h_better");
    // histogram to store number of toys which enter CLb p Value calculation
    TH1F *h_better_clb        = (TH1F*)hCL->Clone("h_better_clb");
    TH1F *h_all        = (TH1F*)hCL->Clone("h_all");
    // numbers of all bkg toys
    TH1F *h_all_bkg           = (TH1F*)hCL->Clone("h_all_bkg");
    TH1F *h_background = (TH1F*)hCL->Clone("h_background");
    TH1F *h_gof        = (TH1F*)hCL->Clone("h_gof");

    // map of vectors for CLb quantiles
    std::map<int,std::vector<double> > sampledSchi2Values;
    std::map<int,std::vector<double> > sampledBValues;
    std::map<int,std::vector<double> > sampledSBValues;

    Long64_t nentries  = t->GetEntries();
    Long64_t nfailed   = 0;
    Long64_t nwrongrun = 0;
    Long64_t ntoysid   = 0; // if id is not -1, this will count the number of toys with that id

    t->activateCoreBranchesOnly(); // speeds up the event loop
    ProgressBar pb(arg, nentries);
    if ( arg->debug ) cout << "MethodPluginScan::analyseToys() : ";
    cout << "building p-value histogram ..." << endl;

    for (Long64_t i = 0; i < nentries; i++)
    {
        pb.progress();
        t->GetEntry(i);

        // std::cout << t->chi2minGlobalToy << "\t" << t->chi2minToy << "\t" << t->chi2min << "\t" << t->scanpoint << std::endl;
        // std::cout << t->chi2minGlobalBkgToy  << "\t" << t->chi2minBkgBkgToy << "\t" << t->chi2minBkg << "\t" << t->scanpoint << std::endl;
        if ( id!=-1 && fabs(t->id-id)>0.001 ) continue; ///< only select entries with given id (unless id==-1)
        ntoysid++;

        // apply cuts
        if ( ! (fabs(t->chi2minToy)<500 && fabs(t->chi2minGlobalToy)<500
                    && t->statusFree==0. && t->statusScan==0. )
           ){
            if(arg->debug){
                std::cout << "DEBUG::MethodPluginScan::AnalyseToys::TOYFAILED" <<std::endl;
                std::cout << "chi2minToy: " << t->chi2minToy << " <500? chi2minGlobalToy: " << t->chi2minGlobalToy << " <500?" <<std::endl; 
                std::cout << "statusFree: " << t->statusFree << " ==0? statusScan: " << t->statusScan << " ==0?" <<std::endl; 
            }
            nfailed++;
            continue;
        }

        // toys from a wrong run
        if ( id!=-1 && ! (fabs(t->chi2minGlobal-chi2minGlobal)<0.2) ){
            nwrongrun++;
        }

        // Cut away toys outside a certain range. This is needed to remove
        // low statistics spikes to get publication quality log plots.
        // Also check line 272 in ToyTree.cpp.
        if ( arg->pluginPlotRangeMin!=arg->pluginPlotRangeMax
                && !(arg->pluginPlotRangeMin<t->scanpoint
                    && t->scanpoint<arg->pluginPlotRangeMax) ) continue;

        // use profile likelihood from internal scan, not the one found in the root files
        if ( arg->intprob ){
            t->chi2min = profileLH->getChi2min(t->scanpoint);
        }

        // Check if toys are in physical region.
        // Don't enforce t.chi2min-t.chi2minGlobal>0, else it can be hard because due
        // to little fluctuaions the best fit point can be missing from the plugin plot...
        bool inPhysicalRegion = t->chi2minToy - t->chi2minGlobalToy>0; //&& t.chi2min-t.chi2minGlobal>0
        int iBinBestFit = hCL->GetMaximumBin();
        float bestfitpoint = hCL->GetBinCenter(iBinBestFit);
        if(getSolution()){
            bestfitpoint= ((RooRealVar*) getSolution()->floatParsFinal().find(scanVar1))->getVal();
        }
        else std::cout << "WARNING: No solution found, will approximate to the best scan point" << std::endl;

        // build test statistics
        // chi2minBkgBkgToy is the best fit of the bkg pdf of bkg-only toy, chi2minGlobalBkgToy is the best global fit of the bkg-only toy
        // chi2minBkgToy is the best fit at scanpoint of bkg-only toy
        double teststat_measured = t->chi2min - t->chi2minGlobal;
        double sb_teststat_toy= t->chi2minToy - t->chi2minGlobalToy;
        double b_teststat_toy = t->chi2minBkgToy - t->chi2minGlobalBkgToy;
        if (arg->teststatistic ==1){ // use one-sided test statistic
            teststat_measured = bestfitpoint <= t->scanpoint ? teststat_measured : 0.; // if mu < muhat then q_mu = 0
            sb_teststat_toy = t->scanbest <= t->scanpoint ? sb_teststat_toy : 0.; // if mu < muhat then q_mu = 0
            b_teststat_toy = t->scanbestBkg <= t->scanpoint ? b_teststat_toy : 0.;  // if mu < muhat then q_mu = 0
        }
        // the usage of the two-sided test statistic is default


        if ( inPhysicalRegion && sb_teststat_toy > teststat_measured ){
            h_better->Fill(t->scanpoint);
        }

        if ( inPhysicalRegion && b_teststat_toy > teststat_measured ) {
            h_better_clb->Fill(t->scanpoint);
        }

        // goodness-of-fit
        if ( inPhysicalRegion && t->chi2minGlobalToy > t->chi2minGlobal ){
            h_gof->Fill(t->scanpoint);
        }

        // all toys
        if ( inPhysicalRegion ){
            h_all->Fill(t->scanpoint);
        }

        // all bkg toys
        if ( inPhysicalRegion ){
            h_all_bkg->Fill(t->scanpoint);
        }

        // use the unphysical events to estimate background (be careful with this,
        // at least inspect the control plots to judge if this can be at all reasonable)
        if ( !inPhysicalRegion ){
            h_background->Fill(t->scanpoint);
        }

        int hBin = h_all->FindBin(t->scanpoint);
        if ( sampledBValues.find(hBin) == sampledBValues.end() ) sampledBValues[hBin] = std::vector<double>();
        if ( sampledSBValues.find(hBin) == sampledSBValues.end() ) sampledSBValues[hBin] = std::vector<double>();
        if ( sampledSchi2Values.find(hBin) == sampledSchi2Values.end() ) sampledSchi2Values[hBin] = std::vector<double>();

        if(inPhysicalRegion){
            sampledSchi2Values[hBin].push_back(sb_teststat_toy);
        }

        if(b_teststat_toy<0&&b_teststat_toy>-1.e-4) b_teststat_toy=0.0;

        if( inPhysicalRegion ){
            // bkgTestStatVal = t->scanbestBkgfitBkg <= 0. ? bkgTestStatVal : 0.;  // if muhat < mu then q_mu = 0
            // if(hBin==2){
            //     // std::cout << bkgTestStatVal << std::endl;
            //     bkg_pvals->Fill(TMath::Prob(bkgTestStatVal,1));
            // }
            // bkgTestStatVal = t->scanbestBkgfitBkg <= t->scanpoint ? bkgTestStatVal : 0.;  // if muhat < mu then q_mu = 0
            sampledBValues[hBin].push_back( b_teststat_toy );
            // chi2minBkgToy is the best fit at scanpoint of bkg-only toy, chi2minGlobalBkgToy is the best global fit of the bkg-only toy
            // sbTestStatVal = t->scanbestBkg <= t->scanpoint ? sbTestStatVal : 0.; // if muhat < mu then q_mu = 0
            sampledSBValues[hBin].push_back( b_teststat_toy );
        }

    }

    if ( arg->debug ) cout << "MethodPluginScan::analyseToys() : ";
    if ( id==-1 ){
        cout << "read an average of ";
        cout << (nentries-nfailed)/nPoints1d << " toys per scan point." << endl;
    }
    else{
        cout << "read ";
        cout << ntoysid << " toys at ID " << id << endl;
    }
    if ( arg->debug ) cout << "MethodPluginScan::analyseToys() : ";
    cout << "fraction of failed toys: " << (double)nfailed/(double)nentries*100. << "%." << endl;
    if ( arg->debug ) cout << "MethodPluginScan::analyseToys() : ";
    cout << "fraction of background toys: " << h_background->GetEntries()/(double)nentries*100. << "%." << endl;
    if ( id==-1 && nwrongrun>0 ){
        cout << "\nMethodPluginScan::analyseToys() : WARNING : Read toys that differ in global chi2min (wrong run) : "
            << (double)nwrongrun/(double)(nentries-nfailed)*100. << "%.\n" << endl;
    }

    for (int i=1; i<=h_better->GetNbinsX(); i++)
    {
        float nbetter = h_better->GetBinContent(i);
        float nbetter_clb = h_better_clb->GetBinContent(i);
        float nall = h_all->GetBinContent(i);
        float nall_bkg = h_all_bkg->GetBinContent(i);
        float nbackground = h_background->GetBinContent(i);
        if ( nall == 0. ) continue;

        // subtract background
        // float p = (nbetter-nbackground)/(nall-nbackground);
        // hCL->SetBinContent(i, p);
        // hCL->SetBinError(i, sqrt(p * (1.-p)/(nall-nbackground)));

        // don't subtract background
        float p = nbetter/nall;
        float p_clb = nbetter_clb / nall_bkg;
        // attempt to correct for undercoverage
        if (pvalueCorrectorSet) {
            p = pvalueCorrector->transform(p);
        }
        hCL->SetBinContent(i, p);
        hCL->SetBinError(i, sqrt(p * (1.-p)/nall));
        float p_bkg = TMath::Min(p/hCL->GetBinContent(1), 1.);


        // determine CLs value
        // CLs values in data
        int nDataAboveBkgExp = 0;
        double dataTestStat = p>0 ? TMath::ChisquareQuantile(1.-p,1) : 1.e10;
        for (int j=0; j<sampledBValues[i].size(); j++ ) {
            if ( sampledBValues[i][j] >= dataTestStat ) nDataAboveBkgExp += 1;
        }

        float dataCLb    = p_clb;
        float dataCLbErr = sqrt( dataCLb * (1.-dataCLb) / sampledBValues[i].size() );
        if ( p/dataCLb >= 1. ) {
          hCLsFreq->SetBinContent(i, 1.);
          hCLsFreq->SetBinError  (i, 0.);
        }
        else if ( dataTestStat == 1.e10 ) {
          hCLsFreq->SetBinContent(i, hCL->GetBinContent(i) );
          hCLsFreq->SetBinError  (i, hCL->GetBinError(i) );
        }
        else if ( hCLsFreq->GetBinCenter(i) <= hCLsFreq->GetBinCenter(h_better->GetMaximumBin()) ) {
          hCLsFreq->SetBinContent(i, 1.);
          hCLsFreq->SetBinError  (i, 0.);
        }
        else {
          hCLsFreq->SetBinContent(i, p / dataCLb);
          hCLsFreq->SetBinError  (i, (p/dataCLb) * sqrt( sq( hCL->GetBinError(i) / hCL->GetBinContent(i) ) + sq( dataCLbErr / dataCLb ) ) );
        }
        if (arg->debug) {
          cout << "At scanpoint " << std::scientific << hCL->GetBinCenter(i) << ": ===== number of toys for pValue calculation: " << nbetter << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue:         " << p << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue CLb:         " << p_clb << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue CLsFreq: " << hCLsFreq->GetBinContent(i) << endl;
          cout << "At scanpoint " << hCL->GetBinCenter(i) << ": ===== pValue CLs: " << p_bkg << endl;
        }

        /// Now make the histograms for the CLs banana plots

        // /// TLimit implementation
        // // // the quantiles of the CLb distribution (for expected CLs)
        // std::vector<double> probs  = { TMath::Prob(4,1), TMath::Prob(1,1), 0.5, 1.-TMath::Prob(1,1), 1.-TMath::Prob(4,1) };
        // std::vector<double> clb_vals  = { 1.-TMath::Prob(4,1), 1.-TMath::Prob(1,1), 0.5, TMath::Prob(1,1), TMath::Prob(4,1) };
        // std::vector<double> quantiles = Quantile<double>( sampledSBValues[i], probs );
        // std::vector<double> clsb_vals;
        // //for (int k=0; k<quantiles.size(); k++) clsb_vals.push_back( TMath::Prob( quantiles[k], 1 ) );
        // for (int k=0; k<quantiles.size(); k++ ){
        //   // asymptotic as chi2
        //   //clsb_vals.push_back( TMath::Prob( quantiles[k], 1 ) );
        //   // from toys
        //   clsb_vals.push_back(getVectorFracAboveValue( sampledSchi2Values[i], quantiles[k] ) );
        // }

        // // check
        // if ( arg->debug ) {
        //   cout << i << endl;
        //   cout << "Quants: ";
        //   for (int k=0; k<quantiles.size(); k++) cout << quantiles[k] << " , ";
        //   cout << endl;
        //   cout << "CLb: ";
        //   for (int k=0; k<clb_vals.size(); k++) cout << clb_vals[k] << " , ";
        //   cout << endl;
        //   cout << "CLsb: ";
        //   for (int k=0; k<clsb_vals.size(); k++) cout << clsb_vals[k] << " , ";
        //   cout << endl;
        //   cout << "CLs: ";
        //   for (int k=0; k<clsb_vals.size(); k++) cout << clsb_vals[k]/clb_vals[k] << " , ";
        //   // for (int k=0; k<clsb_vals.size(); k++) cout << clsb_vals[k]/probs[k] << " , ";
        //   cout << endl;
        // }       

        // // Matt's idea
        // hCLsExp->SetBinContent   ( i, TMath::Min( clsb_vals[2] / clb_vals[2] , 1.) );
        // hCLsErr1Up->SetBinContent( i, TMath::Min( clsb_vals[1] / clb_vals[1] , 1.) );
        // hCLsErr1Dn->SetBinContent( i, TMath::Min( clsb_vals[3] / clb_vals[3] , 1.) );
        // hCLsErr2Up->SetBinContent( i, TMath::Min( clsb_vals[0] / clb_vals[0] , 1.) );
        // hCLsErr2Dn->SetBinContent( i, TMath::Min( clsb_vals[4] / clb_vals[4] , 1.) );



        /// approach via quantiles of p-value distribution
        std::vector<double> clsb_vals;
        std::vector<double> clb_vals;
        std::vector<double> cls_vals;

        if(sampledBValues[i].size()!=sampledSBValues[i].size()){
            std::cout << "MethodPluginScan::analyseToys(): Not the same number of entries in sampledBValues and sampledSBValues!" <<std::endl;
            exit(EXIT_FAILURE);
        }

        TH1F *bkg_pvals_cls  = new TH1F("bkg_clsvals", "bkg cls p values", 50, -0.1, 1.1);
        bkg_pvals_cls->SetLineColor(1);
        bkg_pvals_cls->SetLineWidth(3);
        TH1F *bkg_pvals_clsb  = new TH1F("bkg_clsbvals", "bkg clsb p values", 50, -0.1, 1.1);
        bkg_pvals_clsb->SetLineColor(2);
        bkg_pvals_clsb->SetLineWidth(3);
        TH1F *bkg_pvals_clb  = new TH1F("bkg_clbvals", "bkg clb p values", 50, -0.1, 1.1);
        bkg_pvals_clb->SetLineColor(3);
        bkg_pvals_clb->SetLineWidth(3);

        for(int j=0; j<sampledBValues[i].size(); j++){
            double clsb_val = getVectorFracAboveValue( sampledSchi2Values[i], sampledSBValues[i][j]); // p_cls+b value for each bkg-only toy
            double clb_val = getVectorFracAboveValue( sampledBValues[i], sampledSBValues[i][j]); // p_clb value for each bkg-only toy CAUTION: duplicate use of sampledBValues
            double cls_val = clsb_val/clb_val;

            clsb_vals.push_back(clsb_val);
            clb_vals.push_back(clb_val);
            // if(cls_val<=1.){
            cls_vals.push_back(cls_val);
            bkg_pvals_cls->Fill(TMath::Min( cls_val , 1.));
            bkg_pvals_clsb->Fill(TMath::Min( clsb_val , 1.));
            bkg_pvals_clb->Fill(TMath::Min( clb_val , 1.));
            // }
        }
        TCanvas *canvasdebug = new TCanvas("canvasdebug", "canvas1", 1200, 1000);
        bkg_pvals_cls->Draw();
        bkg_pvals_clsb->Draw("same");
        bkg_pvals_clb->Draw("same");
        std::string pvalue_outstream;
        pvalue_outstream ="p_values" + std::to_string(i) + ".pdf";
        canvasdebug->SaveAs(pvalue_outstream.c_str());


        std::vector<double> probs  = {TMath::Prob(4,1)/2., TMath::Prob(1,1)/2., 0.5, 1.-(TMath::Prob(1,1)/2.), 1.-(TMath::Prob(4,1)/2.) };
        // std::vector<double> probs  = {TMath::Prob(4,1), TMath::Prob(1,1), 0.5, 1.-(TMath::Prob(1,1)), 1.-(TMath::Prob(4,1)) };
        std::vector<double> quantiles_clsb = Quantile<double>( clsb_vals, probs );
        std::vector<double> quantiles_clb = Quantile<double>( clb_vals, probs );
        std::vector<double> quantiles_cls = Quantile<double>( cls_vals, probs );

        // check
        if ( arg->debug ) {
          cout << i << endl;
          cout << "Quants: ";
          for (int k=0; k<probs.size(); k++) cout << probs[k] << " , ";
          cout << endl;
          cout << "CLb: ";
          for (int k=0; k<quantiles_clb.size(); k++) cout << quantiles_clb[k] << " , ";
          cout << endl;
          cout << "CLsb: ";
          for (int k=0; k<quantiles_clsb.size(); k++) cout << quantiles_clsb[k] << " , ";
          cout << endl;
          cout << "CLs: ";
          for (int k=0; k<quantiles_cls.size(); k++) cout << quantiles_cls[k] << " , ";
          cout << endl;
          // for (int k=0; k<quantiles_cls.size(); k++) cout << quantiles_clsb[k]/quantiles_clb[k] << " , ";
          // cout << endl;
        }       

        // //effective method -> works robustly (cf. TLimit class), but is actually wrong
        // hCLsExp->SetBinContent   ( i, TMath::Min( quantiles_clsb[2]/quantiles_clb[2] , 1.) );
        // hCLsErr1Up->SetBinContent( i, TMath::Min( quantiles_clsb[3]/quantiles_clb[3] , 1.) );
        // hCLsErr1Dn->SetBinContent( i, TMath::Min( quantiles_clsb[1]/quantiles_clb[1] , 1.) );
        // hCLsErr2Up->SetBinContent( i, TMath::Min( quantiles_clsb[4]/quantiles_clb[4] , 1.) );
        // hCLsErr2Dn->SetBinContent( i, TMath::Min( quantiles_clsb[0]/quantiles_clb[0] , 1.) );

        // //ideal method, but prone to fluctuations
        hCLsExp->SetBinContent   ( i, TMath::Min( quantiles_cls[2] , 1.) );
        hCLsErr1Up->SetBinContent( i, TMath::Min( quantiles_cls[3] , 1.) );
        hCLsErr1Dn->SetBinContent( i, TMath::Min( quantiles_cls[1] , 1.) );
        hCLsErr2Up->SetBinContent( i, TMath::Min( quantiles_cls[4] , 1.) );
        hCLsErr2Dn->SetBinContent( i, TMath::Min( quantiles_cls[0] , 1.) );

    }

    if ( arg->controlplot && arg->cls.size()>0) makeControlPlotsCLs( sampledBValues, sampledSchi2Values );

    // goodness-of-fit
    if ( id==-1 ){
        int iBinBestFit = hCL->GetMaximumBin();
        float assumedbestfitpoint = hCL->GetBinCenter(iBinBestFit);
        float nGofBetter = h_gof->GetBinContent(iBinBestFit);
        float nall = h_all->GetBinContent(iBinBestFit);
        float fitprobabilityVal = nGofBetter/nall;
        float fitprobabilityErr = sqrt(fitprobabilityVal * (1.-fitprobabilityVal)/nall);
        if ( arg->debug ) cout << "MethodPluginScan::analyseToys() : ";
        cout << "fit prob of best-fit point (" << assumedbestfitpoint << "): "
            << Form("(%.1f+/-%.1f)%%", fitprobabilityVal*100., fitprobabilityErr*100.) << endl;
    }

    t->activateAllBranches();
    delete h_better;
    delete h_all;
    delete h_background;
    return hCL;
}

///
/// Read in the TTrees that were produced by scan1d().
/// Fills the 1-CL histogram.
///
/// \param runMin Number of first root file to read.
/// \param runMax Number of lase root file to read.
///
void MethodPluginScan::readScan1dTrees(int runMin, int runMax, TString fName)
{

    TChain *c = new TChain("plugin");
    int nFilesMissing = 0;
    int nFilesRead = 0;
  // configure file name
  TString dirname = "root/scan1dPlugin";
  if ( arg->isAction("bb") ) dirname += "BergerBoos";
  if ( arg->isAction("uniform") ) dirname += "Uniform";
  if ( arg->isAction("gaus") ) dirname += "Gaus";
  dirname += "_"+name+"_"+scanVar1;
    TString fileNameBase = dirname+"/scan1dPlugin";
  if ( arg->isAction("bb") ) fileNameBase += "BergerBoos";
  if ( arg->isAction("uniform") ) fileNameBase += "Uniform";
  if ( arg->isAction("gaus") ) fileNameBase += "Gaus";
  fileNameBase += "_"+name+"_"+scanVar1+"_run";
  // read different files if requested
  if ( arg->toyFiles != "" && arg->toyFiles != "default" ) fileNameBase = arg->toyFiles;
    if ( arg->debug ) cout << "MethodPluginScan::readScan1dTrees() : ";
    cout << "reading files: " << fileNameBase+"*.root" << endl;
    for (int i=runMin; i<=runMax; i++){
        TString file = Form(fileNameBase+"%i.root", i);
        if ( !FileExists(file) ){
            cout << "WARNING : File not found: " + file + " ..." << endl;
            nFilesMissing += 1;
            continue;
        }
        if ( arg->verbose ) cout << "reading " + file << endl;
        c->Add(file);
        nFilesRead += 1;
    }
    if ( arg->debug ) cout << "MethodPluginScan::readScan1dTrees() : ";
    cout << "read toy files: " << nFilesRead;
    if ( nFilesRead==0 ){
        if ( arg->debug ) cout << "MethodPluginScan::readScan1dTrees() : ";
        cerr << "ERROR : no files read!" << endl;
        exit(EXIT_FAILURE);
    }

    ToyTree t(combiner, c);
    t.open();

    if ( arg->controlplot ) {
        ControlPlots cp(&t);
        if ( arg->plotid==0 || arg->plotid==1 ) cp.ctrlPlotMore(profileLH);
        if ( arg->plotid==0 || arg->plotid==2 ) cp.ctrlPlotChi2();
        if ( arg->plotid==0 || arg->plotid==3 ) cp.ctrlPlotNuisances();
        if ( arg->plotid==0 || arg->plotid==4 ) cp.ctrlPlotObservables();
        if ( arg->plotid==0 || arg->plotid==5 ) cp.ctrlPlotChi2Distribution();
        if ( arg->plotid==0 || arg->plotid==6 ) cp.ctrlPlotChi2Parabola();
        if ( arg->plotid==0 || arg->plotid==7 ) cp.ctrlPlotPvalue();
        cp.saveCtrlPlots();
    }

    if ( hCL ) delete hCL;
    hCL = analyseToys(&t, -1);
}

///
/// Read in the TTrees that were produced by scan2d().
/// Fills the 1-CL histogram.
///
/// \param runMin Number of first root file to read.
/// \param runMax Number of lase root file to read.
///
void MethodPluginScan::readScan2dTrees(int runMin, int runMax)
{
    TChain *chain = new TChain("plugin");
    int nFilesMissing = 0;
    int nFilesRead = 0;
  // configure file name
  TString dirname = "root/scan2dPlugin";
  if ( arg->isAction("bb") ) dirname += "BergerBoos";
  if ( arg->isAction("uniform") ) dirname += "Uniform";
  if ( arg->isAction("gaus") ) dirname += "Gaus";
  dirname += "_"+name+"_"+scanVar1+"_"+scanVar2;
    TString fileNameBase = dirname+"/scan2dPlugin";
  if ( arg->isAction("bb") ) fileNameBase += "BergerBoos";
  if ( arg->isAction("uniform") ) fileNameBase += "Uniform";
  if ( arg->isAction("gaus") ) fileNameBase += "Gaus";
  fileNameBase += "_"+name+"_"+scanVar1+"_"+scanVar2+"_run";
  // read different file if requested
  if ( arg->toyFiles != "" && arg->toyFiles != "default" ) fileNameBase = arg->toyFiles;

  if ( arg->debug ) cout << "MethodPluginScan::readScan2dTrees() : ";
    cout << "reading files: " << fileNameBase+"*.root" << endl;
    for (int i=runMin; i<=runMax; i++) {
        TString file = Form(fileNameBase+"%i.root", i);
        if ( !FileExists(file) ) {
            if ( arg->verbose ) cout << "ERROR : File not found: " + file + " ..." << endl;
            nFilesMissing += 1;
            continue;
        }
        if ( arg->verbose ) cout << "reading " + file + " ..." << endl;
        chain->Add(file);
        nFilesRead += 1;
    }
    cout << "read toy files: " << nFilesRead;
    cout << ", missing files: " << nFilesMissing << endl;
    if ( nFilesRead==0 ){
        if ( arg->debug ) cout << "MethodPluginScan::readScan2dTrees() : ";
        cout << "ERROR : no files read!" << endl;
        exit(1);
    }

    ToyTree t(combiner, chain);
    t.open();

    if ( arg->controlplot ) {
        ControlPlots cp(&t);
        //if ( arg->plotid==0 || arg->plotid==1 ) cp.ctrlPlotMore(profileLH);
        if ( arg->plotid==0 || arg->plotid==2 ) cp.ctrlPlotChi2();
        if ( arg->plotid==0 || arg->plotid==3 ) cp.ctrlPlotNuisances();
        if ( arg->plotid==0 || arg->plotid==4 ) cp.ctrlPlotObservables();
        if ( arg->plotid==0 || arg->plotid==5 ) cp.ctrlPlotChi2Distribution();
        if ( arg->plotid==0 || arg->plotid==6 ) cp.ctrlPlotChi2Parabola();
        if ( arg->plotid==0 || arg->plotid==7 ) cp.ctrlPlotPvalue();
        cp.saveCtrlPlots();
    }

    float halfBinWidthx = (t.getScanpointMax()-t.getScanpointMin())/(float)t.getScanpointN()/2;
    float halfBinWidthy = (t.getScanpointyMax()-t.getScanpointyMin())/(float)t.getScanpointyN()/2;
    if ( t.getScanpointN()==1 )  halfBinWidthx = 1.;
    if ( t.getScanpointyN()==1 ) halfBinWidthy = 1.;
    if ( hCL2d ) delete hCL2d;
    hCL2d          = new TH2F(getUniqueRootName(), "hCL2d", t.getScanpointN(), t.getScanpointMin()-halfBinWidthx, t.getScanpointMax()+halfBinWidthx,
                                                            t.getScanpointyN(), t.getScanpointyMin()-halfBinWidthx, t.getScanpointyMax()+halfBinWidthx);
    TH2F *h_better = (TH2F*)hCL2d->Clone("h_better");
    TH2F *h_all    = (TH2F*)hCL2d->Clone("h_all");

    Long64_t nentries = t.GetEntries();
    Long64_t nfailed = 0;
    Long64_t nwrongrun = 0;
    Long64_t ntoysid   = 0; // if id is not -1, this will count the number of toys with that id

    t.activateCoreBranchesOnly(); // speeds up the event loop
    ProgressBar pb(arg, nentries);
    if ( arg->debug ) cout << "MethodPluginScan::readScan2dTrees() : ";
    cout << "building p-value histogram ..." << endl;

    for (Long64_t i = 0; i < nentries; i++)
    {
        pb.progress();
        t.GetEntry(i);
        if ( arg->id!=-1 && fabs(t.id-arg->id)>0.001 ) continue; ///< only select entries with given id (unless id==-1)
        ntoysid++;

        // apply cuts
        if ( ! (t.chi2minToy > -1e10 && t.chi2minGlobalToy > -1e10
                    && t.chi2minToy-t.chi2minGlobalToy>0
                    // \todo uncomment this line once chi2minGlobal gets stored in the saved scanner
                    //&& fabs((chi2minGlobal_t-chi2minGlobal)/(chi2minGlobal_t+chi2minGlobal))<0.01 // reject files from other runs
                    && t.chi2minToy<1000
               ))
        {
            nfailed++;
            continue;
        }

        // toys from a wrong run
        if ( arg->id!=-1 && ! (fabs(t.chi2minGlobal-chi2minGlobal)<0.2) ){
            nwrongrun++;
        }

        // use profile likelihood from internal scan, not the one found in the root files
        if ( arg->intprob ){
            int iBin = profileLH->getHchisq2d()->FindBin(t.scanpoint,t.scanpointy);
            t.chi2min = profileLH->getHchisq2d()->GetBinContent(iBin);
        }

        // Check if toys are in physical region.
        bool inPhysicalRegion = t.chi2minToy-t.chi2minGlobalToy>0;

        // build test statistic
        if ( inPhysicalRegion && t.chi2minToy-t.chi2minGlobalToy > t.chi2min-t.chi2minGlobal ) {
            h_better->Fill(t.scanpoint,t.scanpointy);
        }

        // all toys
        if ( inPhysicalRegion ){
            h_all->Fill(t.scanpoint,t.scanpointy);
        }
    }

    if ( arg->debug ) cout << "MethodPluginScan::readScan2dTrees() : ";
    if ( arg->id==-1 ){
        cout << "read an average of " << (nentries-nfailed)/nPoints2dx/nPoints2dy << " toys per scan point." << endl;
    }
    else{
        cout << "read ";
        cout << ntoysid << " toys at ID " << arg->id << endl;
    }
    if ( arg->debug ) cout << "MethodPluginScan::readScan2dTrees() : ";
    cout << "fraction of failed toys: " << (double)nfailed/(double)nentries*100. << "%." << endl;

    // compute 1-CL
    for (int i=1; i<=h_better->GetNbinsX(); i++){
        for (int j=1; j<=h_better->GetNbinsY(); j++){
            float nbetter = h_better->GetBinContent(i,j);
            float nall = h_all->GetBinContent(i,j);
            if ( nall == 0. ) continue;
            float p = nbetter/nall;
            hCL2d->SetBinContent(i, j, p);
            hCL2d->SetBinError(i, j, sqrt(p * (1.-p)/nall));
        }
    }

    t.activateAllBranches();
    delete h_better;
    delete h_all;
}

///
/// Importance sampling for low p-values: Returns a value
/// between 0.05 and 1 which can be used to scale down the
/// number of toys to be generated at each scan step. The
/// function is designed such that it logarithmic p-value
/// plots look nice. Below a certain p-value, 1e-4, it
/// returns 0.
/// \param pvalue The expected p-value, e.g. from the profile likelihood
/// \return Fraction between 0.1 and 1
///
double MethodPluginScan::importance(double pvalue)
{
    double f1 = 0.05;  ///< the minimum fraction we allow
    double co = 1e-5;  ///< the p-value for which we don't generate toys anymore.
    if ( pvalue<co ) return 0.0;
    double f = (1.-pvalue)/pvalue/30.;
    if ( f>1. ) return 1.;
    if ( f<f1 ) return f1;
    return f;
}


///
/// make control plots for the CLs method. ToDo: this does not really belong here, but in the ControlPlots class,
/// but for the moment I don't see a way how to put it there.
/// The Chisquare quantile plots have to be updated as well.
/// 
void MethodPluginScan::makeControlPlotsCLs(map<int, vector<double> > bVals, map<int, vector<double> > sbVals)
{
  // the quantiles of the CLb distribution (for expected CLs)
  std::vector<double> probs  = { TMath::Prob(4,1), TMath::Prob(1,1), 0.5, 1.-TMath::Prob(1,1), 1.-TMath::Prob(4,1) };
  std::vector<double> clb_vals  = { 1.-TMath::Prob(4,1), 1.-TMath::Prob(1,1), 0.5, TMath::Prob(1,1), TMath::Prob(4,1) };

  for ( int i=1; i<= hCLs->GetNbinsX(); i++ ) {

    std::vector<double> quantiles = Quantile<double>( bVals[i], probs );
    std::vector<double> clsb_vals;
    for (int k=0; k<quantiles.size(); k++ ){
      clsb_vals.push_back( getVectorFracAboveValue( sbVals[i], quantiles[k] ) );
    }
    TCanvas *c = newNoWarnTCanvas( Form("q%d",i), Form("q%d",i) );
    double max = *(std::max_element( bVals[i].begin(), bVals[i].end() ) );
    TH1F *hb = new TH1F( Form("hb%d",i), "hbq", 50,0, max );
    TH1F *hsb = new TH1F( Form("hsb%d",i), "hsbq", 50,0, max );

    for ( int j=0; j<bVals[i].size(); j++ ) hb->Fill( bVals[i][j] );
    for ( int j=0; j<sbVals[i].size(); j++ ) hsb->Fill( sbVals[i][j] );

    double dataVal = TMath::ChisquareQuantile( 1.-hCL->GetBinContent(i),1 );
    TArrow *lD = new TArrow( dataVal, 0.6*hsb->GetMaximum(), dataVal, 0., 0.15, "|>" );

    vector<TLine*> qLs;
    for ( int k=0; k<quantiles.size(); k++ ) {
      qLs.push_back( new TLine( quantiles[k], 0, quantiles[k], 0.8*hsb->GetMaximum() ) );
    }
    TLatex *lat = new TLatex();
    lat->SetTextColor(kRed);
    lat->SetTextSize(0.6*lat->GetTextSize());
    lat->SetTextAlign(22);

    hsb->GetXaxis()->SetTitle("Test Statistic Value");
    hsb->GetYaxis()->SetTitle("Entries");
    hsb->GetXaxis()->SetTitleSize(0.06);
    hsb->GetYaxis()->SetTitleSize(0.06);
    hsb->GetXaxis()->SetLabelSize(0.06);
    hsb->GetYaxis()->SetLabelSize(0.06);
    hsb->SetLineWidth(2);
    hb->SetLineWidth(2);
    hsb->SetFillColor(kBlue);
    hb->SetFillColor(kRed);
    hsb->SetFillStyle(3003);
    hb->SetFillStyle(3004);
    hb->SetLineColor(kRed);
    hsb->SetLineColor(kBlue);

    //TGraph *gb = Utils::smoothHist(hb, 0);
    //TGraph *gsb = Utils::smoothHist(hsb, 1);

    //gb->SetLineColor(kRed+1);
    //gb->SetLineWidth(4);
    //gsb->SetLineColor(kBlue+1);
    //gsb->SetLineWidth(4);

    hsb->Draw();
    hb->Draw("same");
    //gb->Draw("Lsame");
    //gsb->Draw("Lsame");

    qLs[0]->SetLineWidth(2);
    qLs[0]->SetLineStyle(kDashed);
    qLs[4]->SetLineWidth(2);
    qLs[4]->SetLineStyle(kDashed);
    qLs[1]->SetLineWidth(3);
    qLs[3]->SetLineWidth(3);
    qLs[2]->SetLineWidth(5);

    for ( int k=0; k<quantiles.size(); k++ ){
      qLs[k]->SetLineColor(kRed);
      qLs[k]->Draw("same");
    }
    lat->DrawLatex( quantiles[0], hsb->GetMaximum(), "-2#sigma" );
    lat->DrawLatex( quantiles[1], hsb->GetMaximum(), "-1#sigma" );
    lat->DrawLatex( quantiles[2], hsb->GetMaximum(), "<B>" );
    lat->DrawLatex( quantiles[3], hsb->GetMaximum(), "+1#sigma" );
    lat->DrawLatex( quantiles[4], hsb->GetMaximum(), "+2#sigma" );

    lD->SetLineColor(kBlack);
    lD->SetLineWidth(5);
    lD->Draw("same");

    TLegend *leg = new TLegend(0.74,0.54,0.94,0.7);
    leg->SetHeader(Form("p=%4.2g",hCLs->GetBinCenter(i)));
    leg->SetFillColor(0);
    leg->AddEntry(hb,"B-only Toys","LF");
    leg->AddEntry(hsb,"S+B Toys","LF");
    leg->AddEntry(lD,"Data","L");
    leg->Draw("same");
    c->SetLogy();
    savePlot(c,Form("cls_testStatControlPlot_p%d",i) );
  }

  TCanvas *c = newNoWarnTCanvas( "cls_ctr", "CLs Control" );
  hCLsFreq->SetLineColor(kBlack);
  hCLsFreq->SetLineWidth(3);
  hCLsExp->SetLineColor(kRed);
  hCLsExp->SetLineWidth(3);

  hCLsErr1Up->SetLineColor(kBlue+2);
  hCLsErr1Up->SetLineWidth(2);
  hCLsErr1Dn->SetLineColor(kBlue+2);
  hCLsErr1Dn->SetLineWidth(2);

  hCLsErr2Up->SetLineColor(kBlue+2);
  hCLsErr2Up->SetLineWidth(2);
  hCLsErr2Up->SetLineStyle(kDashed);
  hCLsErr2Dn->SetLineColor(kBlue+2);
  hCLsErr2Dn->SetLineWidth(2);
  hCLsErr2Dn->SetLineStyle(kDashed);

  hCLsFreq->GetXaxis()->SetTitle("POI");
  hCLsFreq->GetYaxis()->SetTitle("Raw CLs");
  hCLsFreq->GetXaxis()->SetTitleSize(0.06);
  hCLsFreq->GetYaxis()->SetTitleSize(0.06);
  hCLsFreq->GetXaxis()->SetLabelSize(0.06);
  hCLsFreq->GetYaxis()->SetLabelSize(0.06);

  hCLsFreq->Draw("L");
  hCLsErr2Up->Draw("Lsame");
  hCLsErr2Dn->Draw("Lsame");
  hCLsErr1Up->Draw("Lsame");
  hCLsErr1Dn->Draw("Lsame");
  hCLsExp->Draw("Lsame");
  hCLsFreq->Draw("Lsame");

  savePlot(c, "cls_ControlPlot");

}

