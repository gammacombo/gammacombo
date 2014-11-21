#include "GammaComboEngine.h"

GammaComboEngine::GammaComboEngine(TString name, int argc, char* argv[])
{
	// time the program
	t.Start();

	// parse the command line options
  arg = new OptParser();
  arg->bookAllOptions();
  arg->parseArguments(argc, argv);

	// configure names
	basename = name;
	fb = new FileNameBuilder(arg, basename);

	// run ROOT in interactive mode, if requested (-i)
  if ( arg->interactive ) theApp = new TApplication("App", &argc, argv);

	// initialize members
	plot = 0;
}

GammaComboEngine::~GammaComboEngine()
{
	delete fb;
}


///
/// Check if a PDF with a certain ID exits.
///
bool GammaComboEngine::pdfExists(int id)
{
	if ( id<0 ) return false;
	if ( id>=this->pdf.size() ) return false;
	if ( this->pdf[id]==0 ) return false;
	return true;
}

///
/// Check if a Combiner with a certain ID exits.
///
bool GammaComboEngine::combinerExists(int id) const
{
	if ( id<0 ) return false;
	if ( id>=this->cmb.size() ) return false;
	if ( this->cmb[id]==0 ) return false;
	return true;
}

///
/// Add a PDF to the GammaComboEngine object.
///
void GammaComboEngine::addPdf(int id, PDF_Abs* pdf, TString title)
{
	if ( pdf==0 ){
		cout << "GammaComboEngine::addPdf() : ERROR : Trying to add zero pointer as the PDF. Exit." << endl;
		exit(1);
	}
	// check if requested id exists already
	if ( pdfExists(id) ){
		cout << "GammaComboEngine::addPdf() : ERROR : Requested PDF id exists already in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	// check if storage is large enough, enlarge if necessary
	if ( id>=this->pdf.size() ){
		for ( int i=this->pdf.size(); i<=id; i++ ) this->pdf.push_back(0);
	}
	this->pdf[id] = pdf;
	if ( title!="" ) this->pdf[id]->setTitle(title);
	this->pdf[id]->setGcId(id);
}

///
/// Add a Combiner to the GammaComboEngine object.
///
void GammaComboEngine::addCombiner(int id, Combiner* cmb)
{
	if ( cmb==0 ){
		cout << "GammaComboEngine::addCombiner() : ERROR : Trying to add zero pointer as the Combiner. Exit." << endl;
		exit(1);
	}
	// check if requested id exists already
	if ( combinerExists(id) ){
		cout << "GammaComboEngine::addCombiner() : ERROR : Requested Combiner id exists already in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	// check if storage is large enough, enlarge if necessary
	if ( id>=this->cmb.size() ){
		for ( int i=this->cmb.size(); i<=id; i++ ) this->cmb.push_back(0);
	}
	this->cmb[id] = cmb;
}

///
/// Add a new Combiner to the GammaComboEngine object, that is the
/// clone of an existing one. After that, the clone can be modified
/// using, e.g., getCombiner(newId)->addPdf(...)
///
void GammaComboEngine::cloneCombiner(int newId, int oldId, TString name, TString title)
{
	if ( combinerExists(newId) ){
		cout << "GammaComboEngine::cloneCombiner() : ERROR : Requested new Combiner id " << newId << " exists already in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	if ( !combinerExists(oldId) ){
		cout << "GammaComboEngine::cloneCombiner() : ERROR : Requested old Combiner id " << oldId << " doesn't exists in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	addCombiner(newId, getCombiner(oldId)->Clone(name, title));
}

///
/// Get a combiner.
/// \param id - combiner ID, set when defining the combiner using addCombiner(), cloneCombiner(), or newCombiner()
///
Combiner* GammaComboEngine::getCombiner(int id) const
{
	if ( !combinerExists(id) ){
		cout << "GammaComboEngine::getCombiner() : ERROR : Requested Combiner id doesn't exist in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	return cmb[id];
}

///
/// Get a PDF.
/// \param id - PDF ID, set when adding the PDF using addPdf()
///
PDF_Abs* GammaComboEngine::getPdf(int id)
{
	if ( !pdfExists(id) ){
		cout << "GammaComboEngine::getPdf() : ERROR : Requested PDF id doesn't exist in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	return pdf[id];
}

PDF_Abs* GammaComboEngine::operator[](int idx)
{
	return getPdf(idx);
}

// const PDF_Abs* GammaComboEngine::operator[](int idx) const
// {
// 	const PDF_Abs* p = (const)getPdf(idx)
// 	return p;
// }

///
/// Add a new Combiner, consisting of the specified PDFs.
/// The pdf arguments refer to the GammaComboEngine ID of the PDFs
/// that should be combined. Add them before using addPdf().
/// Also an empty combiner is possible, that has no PDFs, so it can be filled
/// later. In that case, leave all pdf arguments at -1.
///
void GammaComboEngine::newCombiner(int id, TString name, TString title,
						int pdf1, int pdf2, int pdf3, int pdf4, int pdf5,
						int pdf6, int pdf7, int pdf8, int pdf9, int pdf10,
						int pdf11, int pdf12, int pdf13, int pdf14, int pdf15)
{
	if ( combinerExists(id) ){
		cout << "GammaComboEngine::newCombiner() : ERROR : Requested new Combiner id exists already in GammaComboEngine. Exit." << endl;
		exit(1);
	}
	Combiner *c = new Combiner(arg, name, title);
	if ( pdf1 >-1 ) c->addPdf(getPdf(pdf1 ));
	if ( pdf2 >-1 ) c->addPdf(getPdf(pdf2 ));
	if ( pdf3 >-1 ) c->addPdf(getPdf(pdf3 ));
	if ( pdf4 >-1 ) c->addPdf(getPdf(pdf4 ));
	if ( pdf5 >-1 ) c->addPdf(getPdf(pdf5 ));
	if ( pdf6 >-1 ) c->addPdf(getPdf(pdf6 ));
	if ( pdf7 >-1 ) c->addPdf(getPdf(pdf7 ));
	if ( pdf8 >-1 ) c->addPdf(getPdf(pdf8 ));
	if ( pdf9 >-1 ) c->addPdf(getPdf(pdf9 ));
	if ( pdf10>-1 ) c->addPdf(getPdf(pdf10));
	if ( pdf11>-1 ) c->addPdf(getPdf(pdf11));
	if ( pdf12>-1 ) c->addPdf(getPdf(pdf12));
	if ( pdf13>-1 ) c->addPdf(getPdf(pdf13));
	if ( pdf14>-1 ) c->addPdf(getPdf(pdf14));
	if ( pdf15>-1 ) c->addPdf(getPdf(pdf15));
	addCombiner(id, c);
}

///
/// scale down errors
///
void GammaComboEngine::scaleDownErrors()
{
	for ( int i=0; i<pdf.size(); i++ ){
		// these are the PDFs for the full combination:
	  if (! (i==61 || i==58 || i==60 || i==56 || i==54 || i==40 || i==43)  ) continue;
		float scale = 3.;
	  cout << "Configuration: Scaling down LHCb errors by a factor " << scale <<
			": " << pdf[i]->getTitle() << endl;
		for ( int iObs=0; iObs<pdf[i]->getNobs(); iObs++ ){
			pdf[i]->StatErr[iObs] /= scale;
			pdf[i]->SystErr[iObs] /= scale;
		}
	  pdf[i]->buildCov();
	  pdf[i]->buildPdf();
	}
}

///
/// disable systematics
///
void GammaComboEngine::disableSystematics()
{
  cout << "\nConfiguration: Setting ALL SYSTEMATICS TO ZERO.\n" << endl;
  for ( int i=0; i<pdf.size(); i++ ){
    if ( pdf[i]==0 ) continue;
    for ( int iObs=0; iObs<pdf[i]->getNobs(); iObs++ ) pdf[i]->SystErr[iObs] = 0.;
    pdf[i]->buildCov();
    pdf[i]->buildPdf();
  }
}

///
/// Make an Asimov toy: set all observables set to truth values.
/// The Asimov point needs to be loaded in the combiner before.
/// \param c - combiner which should be set to an asimov toy
///
void GammaComboEngine::setAsimovToy(Combiner* c)
{
	cout << "\nModifying combination \"" << c->getName() << "\": Running an ASIMOV TOY.\n" << endl;
	if ( !c->isCombined() ){
		cout << "GammaComboEngine::setAsimovToy() : ERROR : Can't set an Asimov toy before "
							    "the combiner is combined. Call combine() first." << endl;
		exit(1);
	}

	// set observables to asimov values in workspace
	RooWorkspace* w = c->getWorkspace();
	TIterator* itObs = c->getObservables()->createIterator();
	while(RooRealVar* pObs = (RooRealVar*) itObs->Next()){
		// get theory name from the observable name
		TString pThName = pObs->GetName();
		pThName.ReplaceAll("obs","th");
		// get the theory relation
		RooAbsReal* th = w->function(pThName);
		if ( th==0 ){
			cout << "GammaComboEngine::setAsimovToy() : ERROR : theory relation not found in workspace: " << pThName << endl;
			exit(1);
		}
		// set the observable to what the theory relation predicts
		pObs->setVal(th->getVal());
	}
	delete itObs;

	// write back the asimov values to the PDF object so that when
	// the PDF is printed, the asimov values show up
	for ( int i=0; i<c->getPdfs().size(); i++ ){
		PDF_Abs* pdf = c->getPdfs()[i];
		pdf->setObservableSourceString("Asimov");
		TIterator* itObs = pdf->getObservables()->createIterator();
		while(RooRealVar* pObs = (RooRealVar*) itObs->Next()){
			RooAbsReal* obs =  w->var(pObs->GetName());
			if ( obs==0 ){
				cout << "GammaComboEngine::setAsimovToy() : ERROR : observable not found in workspace: " << pObs->GetName() << endl;
				exit(1);
			}
			pdf->setObservable(pObs->GetName(), obs->getVal());
		}
		delete itObs;
	}
}

///
/// Make an Asimov toy: set all observables set to truth values.
/// The truth values are loaded from a parameter file. If there
/// are more than one points present in that file, the first one
/// is used.
///
void GammaComboEngine::setAsimovToy(Combiner* c, int cId)
{
	if ( cId>=arg->asimov.size() ){
		cout << "GammaComboEngine::setAsimovToy() : ERROR : requesting a non-existent asimov id." << endl;
		return;
	}
	if ( arg->asimov[cId]==0 ){
		cout << "GammaComboEngine::setAsimovToy() : INFO : asimov id 0 = not running an asimov toy" << endl;
		return;
	}
	// load truth values from file
	ParameterCache *pCache = new ParameterCache(arg, getFileBaseName(c));
	TString parfile = getStartParFileFromCommandLine(cId);
	bool loaded = pCache->loadPoints(parfile);

	// If an asimov toy is configured, but no dedicated
	// par file was given to configure the asimov point, and also no dedicated
	// RO par file exists, we use the default par file without asimov.
	if ( isAsimovCombiner(cId) && !loaded ){
		cout << "GammaComboEngine::setAsimovToy() : Asimov parameter file not found. Trying non-Asimov parameter file." << endl;
		TString nonAsimovDefaultFile = pCache->getDefaultFileName();
		nonAsimovDefaultFile.ReplaceAll(Form("Asimov%i",arg->asimov[cId]),"");
		pCache->loadPoints(nonAsimovDefaultFile);
	}
	if ( !loaded ){
		cout << "GammaComboEngine::setAsimovToy() : non-Asimov parameter file not found. Using start values configured in ParameterAbs class." << endl;
	}
	else{
		pCache->setPoint(c,arg->asimov[cId]-1);
	}
	setAsimovToy(c);
}

///
/// print usage and exit
///
void GammaComboEngine::usage()
{
	cout << "GammaComboEngine USAGE\n\n" << endl;
  cout << "Examples:\n"
          "=========\n\n";
  cout << "Select full combination (-c 4), make a 1d Prob scan for r_dk:\n\n"
          "  bin/GammaComboEngine -c 4 -i --var r_dk\n" << endl;
  cout << "Use the --npoints argument to reduce the number of scan points in the Prob method:\n\n"
          "  bin/GammaComboEngine -c 4 -i --var r_dk --npoints 30\n" << endl;
  cout << "Make a 2d Prob scan for r_dk and g. Reverse the order of the --var arguments\n"
          "to swap the axes. Use the --npoints2d argument to increase/reduce the number\n"
          "of scan points.\n\n"
          "  bin/GammaComboEngine -c 4 -i --var r_dk --var g\n" << endl;
  cout << "Compute two cmb and overlay them in the same plot:\n\n"
          "  bin/GammaComboEngine -c 4 -c 5 -i\n" << endl;
  cout << "2d Plot Examples:\n"
          "=================\n"
          "\n"
          "The strategy for 2d scans is to first perform a 1d scan that finds all\n"
          "local minima. Then, a 2d scan is done starting from each of these lcoal\n"
          "minima. The 1d scan is performed in the variable given first.\n\n"
          "Also for 2d plots you can use the --ps option to print the best fit points\n"
          "onto the plot.\n\n"
          "  bin/GammaComboEngine -c 4 -i --var g --var r_dk --npoints2d 40 --ps\n";
  cout << "Available plugin toy control plots:\n"
          "===================================\n"
          "\n"
          "All control plots are produced when option --controlplots is given.\n"
          "If, in addition, -p <n> is given, only one specific plot is produced.\n"
          " (1) technical overview\n"
          " (2) summary\n"
          " (3) nuisances\n"
          " (4) observables\n"
          " (5) chi2 distributions\n"
          " (6) chi2 parabolas\n" << endl;
	print();
  exit(0);
}

///
/// Print the content of this engine.
///
void GammaComboEngine::print()
{
	cout << endl;
  cout << "Available PDFs:" << endl;
  cout << "===============" << endl;
  for ( int i=0; i<pdf.size(); i++ ){
    if ( pdf[i]==0 ) continue;
    printf(" (%2i) %s\n", i, pdf[i]->getTitle().Data());
  }
  cout << endl;
  cout << "Available combinations (to be used as -c <n>):" << endl;
  cout << "==============================================" << endl;
  for ( int i=0; i<cmb.size(); i++ ){
    if ( cmb[i]==0 ) continue;
    printf(" (%i) %s\n", i, cmb[i]->getTitle().Data());
  }
  cout << endl;
}

///
/// Check the combination argument (-c), exit if it is bad.
///
void GammaComboEngine::checkCombinationArg()
{
  if ( arg->combid.size()==0 && arg->addpdf.size()==0 && arg->delpdf.size()==0 ){
    cout << "Please chose a combination ID (-c).\n"
         << "Use the -u option to print a list of available combinations." << endl;
    exit(1);
  }
  for ( int i=0; i<arg->combid.size(); i++ ){
    if ( arg->combid[i] >= cmb.size() ){
      cout << "Please chose a combination ID (-c) less than " << cmb.size()
           << ".\nUse the -u option to print a list of available combinations." << endl;
      exit(1);
    }
    if ( cmb[arg->combid[i]]==0 ){
      cout << "You selected an empty combination.\n"
           << "Use the -u option to print a list of available combinations." << endl;
      exit(1);
    }
  }
}

///
/// Check color argument. See also defineColors().
///
void GammaComboEngine::checkColorArg()
{
	for ( int i=0; i<arg->color.size(); i++ ){
		if ( colorsLine.size()<=arg->color[i] ){
			cout << "No such color. Please choose a color between 0 and " << colorsLine.size()-1 << endl;
			exit(1);
		}
	}
}

///
/// Implement the logic needed for the command line arguments
/// --addpdf and --delpdf
///
void GammaComboEngine::makeAddDelCombinations(TString mode)
{
  if ( !(mode==TString("add") || mode==TString("del")) ) assert(0); // only accept "add" or "del"
  vector<vector<int> >& addDelVector = mode==TString("add") ? arg->addpdf : arg->delpdf;

  for ( int i=0; i<addDelVector.size(); i++ ) {
    int combinerId = addDelVector[i][0];
    cout << (mode==TString("add") ? "--addpdf:" : "--delpdf:");
    cout << " Making a new combination based on combination " << combinerId << endl;
    Combiner *cOld = cmb[combinerId];
    // compute name and title of new combiner
    TString nameNew = cOld->getName();
    TString titleNew = cOld->getTitle();
    for ( int j=1; j<addDelVector[i].size(); j++ ){
      int pdfId = addDelVector[i][j];
      if ( ! pdfExists(pdfId) ){
        cout << "  ERROR: PDF ID not defined: " << pdfId << endl;
        continue;
      }
      if ( mode==TString("add") ){
        nameNew += Form("_addedPdf%i",pdfId);
        titleNew += Form(", added PDF%i",pdfId);
      }
      else {
        nameNew += Form("_withoutPdf%i",pdfId);
        titleNew += Form(", w/o PDF%i",pdfId);
      }
    }
    // make the new combiner
    Combiner *cNew = cOld->Clone(nameNew, titleNew);
    // add pdfs
    for ( int j=1; j<addDelVector[i].size(); j++ ){
      int pdfId = addDelVector[i][j];
      if ( ! pdfExists(pdfId) ) continue;
      if ( mode==TString("add") ){
        cout << "... adding PDF " << pdfId << endl;
        cNew->addPdf(pdf[pdfId]);
      }
      else {
        cout << "... deleting PDF " << pdfId << endl;
        cNew->delPdf(pdf[pdfId]);
      }
    }
    // add to list of combinations to compute this round
    cmb.push_back(cNew);
    arg->combid.push_back(cmb.size()-1);
  }
}

///
/// print parameter structure of the combinations into
/// .dot file
///
void GammaComboEngine::printCombinerStructure()
{
	Graphviz gviz(arg);
  for ( int i=0; i<arg->combid.size(); i++ ){
    int combinerId = arg->combid[i];
    gviz.printCombiner(cmb[combinerId]);
    gviz.printCombinerLayer(cmb[combinerId]);
  }
}

///
/// Override default titles of the combinations, if requested.
///
void GammaComboEngine::customizeCombinerTitles()
{
	for ( int i=0; i<arg->combid.size(); i++ )
  {
    int combinerId = arg->combid[i];
    Combiner *c = cmb[combinerId];
    if ( i<arg->title.size() ){
      if ( arg->title[i]!=TString("default") ) c->setTitle(arg->title[i]);
    }
  }
}

///
/// Compute the file base name of individual combinations.
///
/// \param c - Combiner object
/// \return the filename: basename_combinername[_addPdfN][_delPdfN]_var1[_var2]
///
TString GammaComboEngine::getFileBaseName(Combiner *c)
{
	TString name = basename;
	name += "_"+c->getName();
  name += "_"+arg->var[0];
  if ( arg->var.size()==2 ) name += "_"+arg->var[1];
	return name;
}

///
/// Compute the file base name of when including several into one file.
///
/// \return the filename: basename_combiner1name[_addPdfN][_delPdfN][_combiner2name[_addPdfN][_delPdfN]]_var1[_var2]
///
TString GammaComboEngine::getFileBaseName()
{
	TString name = basename;
  for ( int i=0; i<arg->combid.size(); i++ ){
		name += "_"+cmb[arg->combid[i]]->getName();
		if ( isAsimovCombiner(i) ) name += Form("Asimov%i",arg->asimov[i]);
	}
  name += "_"+arg->var[0];
  if ( arg->var.size()==2 ) name += "_"+arg->var[1];
	return name;
}

///
/// Set up the plot.
///
void GammaComboEngine::setUpPlot(TString name)
{
	if ( arg->plotpluginonly ) name += "_pluginOnly";
  if ( arg->var.size()==1 ){
		plot = new OneMinusClPlot(arg, name);
	}
  else{
		plot = new OneMinusClPlot2d(arg, name);
	}
	plot->disableLegend(arg->plotlegend);
}

///
/// Save the plot to disc.
///
void GammaComboEngine::savePlot()
{
	plot->save();
}

///
/// Define the colors of curves and numbers in the 1D plots.
/// It is important to call this only after new combinations were
/// made using the makeAddDelCombinations() mechanism.
///
void GammaComboEngine::defineColors()
{
	if ( arg->color.size()==0 )
	{
	  // define line colors for 1-CL curves
	  colorsLine.push_back(arg->combid.size()==1 ? kBlue-8 : kBlue-5);
	  colorsLine.push_back(kGreen-8);
	  colorsLine.push_back(kOrange-8);
	  colorsLine.push_back(kViolet-7);

	  // define text colors for drawn central values
	  colorsText.push_back(arg->combid.size()==1 ? kBlack : TColor::GetColor("#23236b"));
	  colorsText.push_back(TColor::GetColor("#234723"));
	  colorsText.push_back(kOrange+3);
	  colorsText.push_back(kViolet-7);
	}
	else
	{
		colorsLine.push_back(TColor::GetColor("#1b9e77"));
		colorsLine.push_back(TColor::GetColor("#d95f02"));
		colorsLine.push_back(TColor::GetColor("#7570b3"));
		colorsLine.push_back(TColor::GetColor("#e7298a"));
		colorsLine.push_back(TColor::GetColor("#66a61e"));
		colorsLine.push_back(TColor::GetColor("#e6ab02"));
	
		ColorBuilder cb;
		for ( int i=0; i<colorsLine.size(); i++ ){
			colorsText.push_back(cb.darklightcolor(colorsLine[i], 0.5));
		}
	}
	
	// default for any additional scanner
	for ( int i=colorsLine.size(); i<arg->combid.size(); i++ ){
		colorsLine.push_back(kBlue-8 + i);
		colorsText.push_back(kBlue-2 + i);
	}
}

void GammaComboEngine::scanStrategy2d(MethodProbScan *scanner, ParameterCache *pCache){

	int nStartingPoints = pCache->getNPoints();
	// if no starting values loaded do the default thing
	if (nStartingPoints==0) {
		scanner->scan2d();
		Combiner *c = scanner->getCombiner();
    MethodProbScan *s1 = new MethodProbScan(c);
    s1->setScanVar1(scanner->getScanVar1Name());
    s1->initScan();
    scanStrategy1d(s1,pCache);
    s1->printLocalMinima();

    MethodProbScan *s2 = new MethodProbScan(c);
    s2->setScanVar1(scanner->getScanVar2Name());
    s2->initScan();
    scanStrategy1d(s2,pCache);
    s2->printLocalMinima();

    vector<RooSlimFitResult*> solutions;
    for ( int i=0; i<s1->getNSolutions(); i++ ) solutions.push_back(s1->getSolution(i));
    for ( int i=0; i<s2->getNSolutions(); i++ ) solutions.push_back(s2->getSolution(i));
    for ( int j=0; j<solutions.size(); j++ ){
      cout << "2D SCAN " << j+1 << " OF " << solutions.size() << " ..." << endl;
      scanner->loadParameters(solutions[j]);
      scanner->scan2d();
    }
	}
	// otherwise load each starting value found
	else {
		cout << "Number of scans to run: " << nStartingPoints << endl;
		for (int i=0; i<nStartingPoints; i++){
			pCache->setPoint(scanner,i);
			scanner->scan2d();
		}
	}
}

///
/// Perform the prob scan.
/// \param scanner - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dProbScan(MethodProbScan *scanner, int cId)
{
	// load start parameters
	ParameterCache *pCache = new ParameterCache(arg, getFileBaseName(scanner->getCombiner()));
	TString startparfile = getStartParFileFromCommandLine(cId);
	bool loaded = pCache->loadPoints(startparfile);

	// If an asimov toy is configured, but no dedicated
	// par file was given to configure the asimov point, and also no dedicated
	// RO par file exists, we use the default par file without asimov.
	if ( isAsimovCombiner(cId) && !loaded ){
		cout << "GammaComboEngine::make1dProbScan() : Asimov start parameters not found. Trying non-Asimov parameters." << endl;
		TString nonAsimovDefaultFile = pCache->getDefaultFileName();
		nonAsimovDefaultFile.ReplaceAll(Form("Asimov%i",arg->asimov[cId]),"");
		pCache->loadPoints(nonAsimovDefaultFile);
	}

	scanner->initScan();
	scanStrategy1d(scanner, pCache);
	scanner->printLocalMinima();
	scanner->calcCLintervals();
	if (!arg->isAction("pluginbatch") && !arg->plotpluginonly){
		if ( arg->plotpulls ) scanner->plotPulls();
		if ( arg->parevol ){
			ParameterEvolutionPlotter plotter(scanner);
			plotter.plotParEvolution();
		}
		if ( isScanVarObservable(scanner->getCombiner(), arg->var[0]) ){
			ParameterEvolutionPlotter plotter(scanner);
			plotter.plotObsScanCheck();
		}
		if ( arg->printnuisances1d.size()>0 ) scanner->printNuisances(arg->printnuisances1d);
		if (!arg->isAction("plugin")){
			scanner->saveScanner(fb->getFileNameScanner(scanner));
			pCache->cacheParameters(scanner);
		}
	}
}

///
/// Perform the 1d plugin scan. Runs toys in batch mode, and
/// reads them back in.
/// \param scannerPlugin - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dPluginScan(MethodPluginScan *scannerPlugin, int cId)
{
	scannerPlugin->initScan();
  if ( arg->isAction("pluginbatch") ){
    scannerPlugin->scan1d(arg->nrun);
  }
  else {
		scannerPlugin->readScan1dTrees(arg->jmin[cId],arg->jmax[cId]);
	}
  scannerPlugin->calcCLintervals();
	if ( !arg->isAction("pluginbatch") ){
		scannerPlugin->saveScanner(fb->getFileNameScanner(scannerPlugin));
	}
}

///
/// Perform the 2d plugin scan. Runs toys in batch mode, and
/// reads them back in.
/// \param scannerPlugin - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make2dPluginScan(MethodPluginScan *scannerPlugin, int cId)
{
	scannerPlugin->initScan();
  if ( arg->isAction("pluginbatch") ){
    scannerPlugin->scan2d(arg->nrun);
  }
  else {
		scannerPlugin->readScan2dTrees(arg->jmin[cId],arg->jmax[cId]);
		scannerPlugin->saveScanner(fb->getFileNameScanner(scannerPlugin));
	}
}

///
/// Make a 1D plot of a Prob scanner. The scanner can either be a "fresh"
/// one, as made in make1dProbScan(), or a loaded one.
/// \param scanner - the scanner to plot
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dProbPlot(MethodProbScan *scanner, int cId)
{
	if (!arg->isAction("pluginbatch") && !arg->plotpluginonly){
		scanner->plotOn(plot);
		int colorId = cId;
		if ( arg->color.size()>cId ) colorId = arg->color[cId];
		scanner->setLineColor(colorsLine[colorId]);
		scanner->setTextColor(colorsText[cId]);
		plot->Draw();
	}
}

///
/// Perform a 1D scan with the following scan strategy.
/// If no starting values loaded do the default thing:
/// 1. scan once, using the start parameters found in the ParameterAbs-derived parameter class
/// 2. scan again from each solution found in the first step
///
void GammaComboEngine::scanStrategy1d(MethodProbScan *scanner, ParameterCache *pCache)
{
	int nStartingPoints = pCache->getNPoints();
	if (nStartingPoints==0) {
		cout << "Scan strategy: Now performing the first scan." << endl;
		scanner->scan1d();
		cout << "Scan strategy: Now performing an additional scan starting at each solution found." << endl;
		cout << "Scan strategy: Number of scans to run: " << scanner->getNSolutions() << endl;
		if ( !(scanner->getArg()->probforce) ){
			for ( int j=0; j<scanner->getNSolutions(); j++ ){
				scanner->loadSolution(j);
				scanner->scan1d(true);
			}
		}
	}
	// otherwise load each starting value found
	else {
		cout << "Number of scans to run: " << nStartingPoints << endl;
		for (int i=0; i<nStartingPoints; i++){
			pCache->setPoint(scanner,i);
			scanner->scan1d();
		}
	}
}

///
/// Make the default 1d plugin plot that shows both the prob and the plugin
/// as points with error bars.
/// \param sPlugin - the plugin scanner
/// \param sProb - the prob scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dPluginPlot(MethodPluginScan *sPlugin, MethodProbScan *sProb, int cId)
{
	make1dProbPlot(sProb, cId);
	sPlugin->setLineColor(kBlack);
  sPlugin->plotOn(plot);
	plot->Draw();
}

///
/// Make the default 2d plugin plot.
/// \param sPlugin - the plugin scanner
///
void GammaComboEngine::make2dPluginPlot(MethodPluginScan *sPlugin)
{
	sPlugin->plotOn(plot);
	// also save the full chi2 histogram
	// OneMinusClPlot2d* plotf = new OneMinusClPlot2d(arg, getFileBaseName(c)+"_plugin_full");
	// sPlugin->plotOn(plotf);
	// plotf->DrawFull();
	// plotf->save();
}

///
/// Make the plugin-only plot that only shows the plugin
/// as continuous lines.
/// \param sPlugin - the plugin scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dPluginOnlyPlot(MethodPluginScan *sPlugin, int cId)
{
	((OneMinusClPlot*)plot)->setPluginMarkers(false);
	int colorId = cId;
	if ( arg->color.size()>cId ) colorId = arg->color[cId];
	sPlugin->setLineColor(colorsLine[colorId]);
	sPlugin->setTextColor(colorsText[colorId]);
	sPlugin->plotOn(plot);
	plot->Draw();
}

///
///
///
void GammaComboEngine::make2dProbScan(MethodProbScan *scanner, int cId)
{
	// load start parameters
	ParameterCache *pCache = new ParameterCache(arg, getFileBaseName(scanner->getCombiner()));
	TString startparfile = getStartParFileFromCommandLine(cId);
	pCache->loadPoints(startparfile);
	// scan
	scanner->initScan();
	scanStrategy2d(scanner,pCache);
  scanner->printLocalMinima();
	// plot
  if (!arg->isAction("pluginbatch")){
		scanner->plotOn(plot);
	  scanner->setLineColor(colorsLine[cId]);
	  if ( arg->printnuisances2dx.size()>0 ) scanner->printNuisances(arg->printnuisances2dx, arg->printnuisances2dy);
	  plot->Draw();
	  OneMinusClPlot2d* plotf = new OneMinusClPlot2d(arg, plot->getName()+"_full");
	  scanner->plotOn(plotf);
	  plotf->DrawFull();
    plotf->save();
    scanner->saveScanner(fb->getFileNameScanner(scanner));
		pCache->cacheParameters(scanner);
  }
}

///
/// Remake a 2D plot from a saved scanner.
/// If we did the scan before and just want to recreate the plot,
/// we can do so using the hCL histograms that were saved in plots/hCL.
///
void GammaComboEngine::make2dProbPlot(MethodProbScan *scanner, int cId)
{
  scanner->loadScanner(fb->getFileNameScanner(scanner));
  scanner->plotOn(plot);
  scanner->setLineColor(colorsLine[cId]);
  plot->Draw();
}

///
/// Helper function for scan(). Fixes parameters, if requested
/// (only possible before combining).
///
void GammaComboEngine::fixParameters(Combiner *c, int cId)
{
	if ( cId<arg->fixParameters.size() ){
		for ( int j=0; j<arg->fixParameters[cId].size(); j++ ){
			c->fixParameter(arg->fixParameters[cId][j].name,arg->fixParameters[cId][j].value);
		}
	}
}

///
/// Helper function for scan(): Checks if for a given combid (the
/// running index of the -c argument) a start parameter file was
/// configured (-l) argument. If so, it is returned, else "default"
/// is returned, causing the default start parameter file to
/// be loaded.
///
TString GammaComboEngine::getStartParFileFromCommandLine(int i)
{
	if ( arg->loadParamsFile.size()<=i ) return "default";
	return arg->loadParamsFile[i];
}

bool GammaComboEngine::isAsimovCombiner(int i)
{
	return i<arg->asimov.size() && arg->asimov[i]>0;
}


///
/// Checks if a given variable name is in the list of observables
/// of a combiner.
/// \param c		- the combiner
/// \param scanVar 	- the scan variable name
/// \return	true if included, else false
///
bool GammaComboEngine::isScanVarObservable(Combiner *c, TString scanVar)
{
	vector<string> obs = c->getObservableNames();
	for ( int i=0; i<obs.size(); i++ ){
		if ( TString(obs[i])==scanVar ) return true;
	}
	return false;
}


///
/// scan engine
///
void GammaComboEngine::scan()
{
	for ( int i=0; i<arg->combid.size(); i++ )
	{
		int combinerId = arg->combid[i];
		Combiner *c = cmb[combinerId];

		// work with a clone - this way we can easily make plots with the
		// same combination in twice (once with asimov, for example)
		c = c->Clone(c->getName(), c->getTitle());

		// fix parameters - only possible before combining
		if ( i<arg->fixParameters.size() ) fixParameters(c, i);

		// configure names to run an Asimov toy - only possible before combining
		if ( isAsimovCombiner(i) ){
			if ( arg->title[i]==TString("default") ) c->setTitle(c->getTitle()+" (Asimov)");
			c->setName(c->getName()+Form("Asimov%i",arg->asimov[i]));
		}

		// combine
		c->combine();
		if ( !c->isCombined() ) continue; // error during combining

		// set an asimov toy - only possible after combining
		if ( isAsimovCombiner(i) ) setAsimovToy(c, i);

		// configure scans for observables
		if ( isScanVarObservable(c, arg->var[0]) ){
			cout << "setting up a scan for an observable ..." << endl;
			// 1. tighten the constraint on the observable
			// 2. add observable to the list of parameters
			c->getWorkspace()->extendSet(c->getParsName(), arg->var[0]);
		}

		// printout
		c->print();
		if ( arg->debug ) c->getWorkspace()->Print("v");

		/////////////////////////////////////////////////////
		//
		// PROB
		//
		/////////////////////////////////////////////////////

		if ( !arg->isAction("plugin") && !arg->isAction("pluginbatch") )
		{
			MethodProbScan *scannerProb = new MethodProbScan(c);
			// pvalue corrector
			PValueCorrection *pvalueCorrector;
			if (arg->coverageCorrectionID>0) {
				pvalueCorrector = new PValueCorrection(arg->coverageCorrectionID, arg->verbose);
				pvalueCorrector->readFiles(getFileBaseName(c),arg->coverageCorrectionPoint,false); // false means for prob
				pvalueCorrector->write("root/pvalueCorrection_prob.root");
				scannerProb->setPValueCorrector(pvalueCorrector);
			}

			// 1D SCANS
			if ( arg->var.size()==1 )
			{
				if ( arg->isAction("plot") ){
					scannerProb->loadScanner(fb->getFileNameScanner(scannerProb));
				}
				else{
					make1dProbScan(scannerProb, i);
				}
				make1dProbPlot(scannerProb, i);
			}
			// 2D SCANS
			else if ( arg->var.size()==2 )
			{
				if ( arg->isAction("plot") ){
					make2dProbPlot(scannerProb, i);
				}
				else{
					make2dProbScan(scannerProb, i);
				}
			}
		}

		/////////////////////////////////////////////////////
		//
		// PLUGIN
		//
		/////////////////////////////////////////////////////

		if ( arg->isAction("plugin") || arg->isAction("pluginbatch") )
		{
			MethodProbScan *scannerProb = new MethodProbScan(c);
			if ( arg->isAction("plot") ){
				scannerProb->loadScanner(fb->getFileNameScanner(scannerProb));
			}
			else{
				make1dProbScan(scannerProb, i);
			}
			MethodPluginScan *scannerPlugin = new MethodPluginScan(scannerProb);
			PValueCorrection *pvalueCorrector2;
			if (arg->coverageCorrectionID>0) {
				pvalueCorrector2 = new PValueCorrection(arg->coverageCorrectionID, arg->verbose);
				pvalueCorrector2->readFiles(getFileBaseName(c),arg->coverageCorrectionPoint,true); // true means for plugin
				pvalueCorrector2->write("root/pvalueCorrection_plugin.root");
				scannerPlugin->setPValueCorrector(pvalueCorrector2);
			}

			//       // Hybrid Plugin: compute a second profile likelihood to define the parameter evolution
			//       if ( arg->pevid.size()==1 )
			//       {
			//         cout << "HYBRID PLUGIN: preparing profile likelihood to be used for parameter evolution:" << endl;
			// // load start parameters
			// ParameterCache *pCache = new ParameterCache(arg, getFileBaseName(cmb[arg->pevid[0]]));
			// pCache->loadPoints();
			//         MethodProbScan *scanner3 = new MethodProbScan(cmb[arg->pevid[0]]);
			//         scanner3->initScan();
			// scanStrategy1d(scanner3,pCache);
			//         scanner3->confirmSolutions();
			//         scanner3->printLocalMinima();
			//         scanner2->setParevolPLH(scanner3);
			//       }

			// 1D SCANS
			if ( arg->var.size()==1 )
			{
				if ( arg->isAction("plot") ){
					scannerPlugin->loadScanner(fb->getFileNameScanner(scannerPlugin));
				}
				else {
					make1dPluginScan(scannerPlugin, i);
				}
				if ( !arg->isAction("pluginbatch") ){
					if ( arg->plotpluginonly ){
						make1dPluginOnlyPlot(scannerPlugin, i);
					}
					else{
						make1dPluginPlot(scannerPlugin, scannerProb, i);
					}
				}
			}
			// 2D SCANS
			else if ( arg->var.size()==2 )
			{
				if ( arg->isAction("plot") ){
					scannerPlugin->loadScanner(fb->getFileNameScanner(scannerPlugin));
				}
				else {
					make2dPluginScan(scannerPlugin, i);
				}
				if ( !arg->isAction("pluginbatch") ){
					make2dPluginPlot(scannerPlugin);
				}
			}
		}

		/////////////////////////////////////////////////////

		if ( i<arg->combid.size()-1 ) {
			cout << "\n--------------------------------------------------\n" << endl;
		}
	}
}

///
/// run the ROOT application, if the -i flag for interactive
/// mode was set.
///
void GammaComboEngine::runApplication()
{
	if ( arg->interactive ) theApp->Run();
}

///
/// run GammaComboEngine, main steering function
///
void GammaComboEngine::run()
{
	if ( arg->usage ) usage(); // print usage and exit
	defineColors();
	checkCombinationArg();
	checkColorArg();
	if ( arg->isQuickhack(6) ) scaleDownErrors();
	if ( arg->nosyst ) disableSystematics();
	makeAddDelCombinations("add");
	makeAddDelCombinations("del");
	defineColors();
	printCombinerStructure();
	customizeCombinerTitles();
	setUpPlot(getFileBaseName());
	scan();
	if (!arg->isAction("pluginbatch")) savePlot();
	t.Stop();
	t.Print();
	runApplication();
}

