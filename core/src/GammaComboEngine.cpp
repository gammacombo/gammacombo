#include "GammaComboEngine.h"

GammaComboEngine::GammaComboEngine(TString name, int argc, char* argv[])
{
	// time the program
	t.Start();

	// print the copyright banner
	printBanner();

	// parse the command line options
	arg = new OptParser();
	arg->bookAllOptions();
	arg->parseArguments(argc, argv);

	// configure names
	execname = argv[0];
	if (arg->filenameaddition!="") name += "_"+arg->filenameaddition;
	m_fnamebuilder = new FileNameBuilder(arg, name);

  // make batch scripts if appropriate and exit
  m_batchscriptwriter = new BatchScriptWriter(argc, argv);

	// run ROOT in interactive mode, if requested (-i)
	if ( arg->interactive ) theApp = new TApplication("App", &argc, argv);
	else gROOT->SetBatch(false);

	// initialize members
	plot = 0;
}

GammaComboEngine::~GammaComboEngine()
{
	delete m_fnamebuilder;
  delete m_batchscriptwriter;
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
/// \todo implement a more generic version of this function. As it is,
/// it is only useful for the LHCb gamma combination.
///
void GammaComboEngine::scaleDownErrors()
{
	for ( int i=0; i<pdf.size(); i++ ){
		// these are the PDFs for the full combination:
		//if (! (i==61 || i==58 || i==60 || i==56 || i==54 || i==40 || i==43)  ) continue;
		float scale = 1.;
		if ( i==25  ) scale = sqrt(5. * 50./3.);
		if ( i==60  ) scale = sqrt(50./3.);
		//if ( i==23  ) scale = sqrt(50.);
		if ( scale==1. ) continue;
		cout << "Configuration: Scaling down LHCb errors by a factor " << scale <<
			": " << pdf[i]->getTitle() << endl;
		for ( int iObs=0; iObs<pdf[i]->getNobs(); iObs++ ){
			pdf[i]->StatErr[iObs] /= scale;
			pdf[i]->SystErr[iObs] /= scale;
		}
		pdf[i]->setErrorSourceString("Scaled!");
		pdf[i]->buildCov();
		pdf[i]->buildPdf();
	}

	int i=0;

	// switch off Afav
	i = 25;
	pdf[i]->ScaleError("afav_dk_kpi_obs",200.);
	pdf[i]->buildCov();
	pdf[i]->buildPdf();

	// scale dD_kpi
	i = 7;
	pdf[i]->ScaleError("dD_kpi_obs",1./20.);
	pdf[i]->buildCov();
	pdf[i]->buildPdf();

	cout << endl;
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
void GammaComboEngine::setAsimovObservables(Combiner* c)
{
	if ( !c->isCombined() ){
		cout << "GammaComboEngine::setAsimovObservables() : ERROR : Can't set an Asimov toy before "
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
			cout << "GammaComboEngine::setAsimovObservables() : ERROR : theory relation not found in workspace: " << pThName << endl;
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
				cout << "GammaComboEngine::setAsimovObservables() : ERROR : observable not found in workspace: " << pObs->GetName() << endl;
				exit(1);
			}
			pdf->setObservable(pObs->GetName(), obs->getVal());
		}
		delete itObs;
	}
}

///
/// Load start parameters.
///
/// \param s - the scanner
/// \param cId - combiner id
/// \param pCache - parameter cache
///
void GammaComboEngine::loadStartParameters(MethodProbScan *s, ParameterCache *pCache, int cId)
{
	cout << "Start parameter configuration:\n" << endl;
	TString startparfile;
	TString startparfile2 = m_fnamebuilder->getFileNameStartPar(s);
	TString startparfile3;
	if ( arg->isAsimovCombiner(cId) ){
		startparfile3= m_fnamebuilder->getFileNameStartPar(s); // this gets the start parameter file of the Asimov combiner
		startparfile3.ReplaceAll(m_fnamebuilder->getAsimovCombinerNameAddition(arg->asimov[cId]),"");
	}
	bool filefound = false;
	// try the file provided through --parfile
	if ( arg->loadParamsFile.size()>cId && ! arg->loadParamsFile[cId].EqualTo("default") ) {
		startparfile = arg->loadParamsFile[cId];
		filefound = FileExists(startparfile);
		if ( !filefound ){
			cout << "\n ERROR: --parfile not found: " << startparfile << endl;
			cout << "  Will now look for default files.\n" << endl;
		}
	}
	// requested file not found, try default
	if ( ! filefound ){
		startparfile = startparfile2;
		filefound = FileExists(startparfile);
	}
	// for Asimov combiners, also try non-Asimov start parameters file
	if ( ! filefound && arg->isAsimovCombiner(cId) ){
		startparfile = startparfile3;
		filefound = FileExists(startparfile);
	}
	// still not found
	if ( ! filefound ){
		cout << "  No start parameter file was found, will use the default values" << endl;
		cout << "  configured in ParameterAbs class. Start parameter files are searched" << endl;
		cout << "  in the following order:" << endl;
		cout << "  1. --parfile option" << endl;
		cout << "  2. default start paramter file: " << startparfile2 << endl;
		if ( arg->isAsimovCombiner(cId) ){
			cout << "  3. for Asimov combiners: corresponding non-Asimov start parameter file: " << startparfile3 << endl;
		}
	}
	else {
		cout << "  Loading start parameters from file: " << startparfile << endl;
		bool loaded = pCache->loadPoints(startparfile);
		if ( !loaded ){
			cout << "  Error loading file. Exit." << endl;
			exit(1);
		}
	}
	cout << endl;
}

///
/// Configure the names and titles of Asimov combiners.
/// It adds things like "Asimov3" to the combiner name,
/// where 3 means the 3rd Asimov point from the configured
/// Asimov parameter file (typically *_genpoints.dat).
///
void GammaComboEngine::configureAsimovCombinerNames(Combiner* c, int i)
{
	if ( arg->asimov[i]==0 ){
		cout << "\n--asimov 0 : ignoring generator point ID 0" << endl;
		return;
	}
	cout << "\n--asimov : setting up an ASIMOV TOY based on combination \"" << c->getName() << "\"" << endl;
	if ( arg->title[i]==TString("default") ) c->setTitle(c->getTitle()+" (Asimov)");
	c->setName(c->getName() + m_fnamebuilder->getAsimovCombinerNameAddition(arg->asimov[i]));
	cout <<   "           Asimov combiner name: \"" << c->getName() << "\"" << endl;
}

///
/// Make an Asimov toy: set all observables set to truth values.
/// The truth values are loaded from a parameter file.
///
void GammaComboEngine::loadAsimovPoint(Combiner* c, int cId)
{
	if ( arg->asimov[cId]==0 ) return;
	cout << "\nAsimov point configuration:\n" << endl;
	ParameterCache *pCache = new ParameterCache(arg);
	TString asimovfile;
	TString asimovfile2 = m_fnamebuilder->getFileNameAsimovPar(c);
	TString asimovfile3 = m_fnamebuilder->getFileNameStartPar(c); // this gets the start parameter file of the Asimov combiner
	asimovfile3.ReplaceAll(m_fnamebuilder->getAsimovCombinerNameAddition(arg->asimov[cId]),"");
	TString asimovfile4 = m_fnamebuilder->getFileNamePar(c); // this gets the result parameter file of the Asimov combiner
	asimovfile4.ReplaceAll(m_fnamebuilder->getAsimovCombinerNameAddition(arg->asimov[cId]),"");
	bool filefound = false;
	// try the file provided through --asimovfile
	if ( arg->asimovfile.size()>cId && ! arg->asimovfile[cId].EqualTo("default") ) {
		asimovfile = arg->loadParamsFile[cId];
		filefound = FileExists(asimovfile);
	}
	// requested file not found, try default
	if ( ! filefound ){
		asimovfile = asimovfile2;
		filefound = FileExists(asimovfile);
	}
	// requested file not found, try the start parameter file of the corresponding non-Asimov combiner
	if ( ! filefound ){
		asimovfile = asimovfile3;
		filefound = FileExists(asimovfile);
	}
	// requested file not found, try the result parameter file of the corresponding non-Asimov combiner
	if ( ! filefound ){
		asimovfile = asimovfile4;
		filefound = FileExists(asimovfile);
	}
	// if no parameter file exits, we use point from the ParameterAbs class
	if ( ! filefound ){
		cout << "  No Asimov point parameter file found. Using start values configured in ParameterAbs class." << endl;
		cout << "  Point files are looked for in the following order:" << endl;
		cout << "  1. --asimovfile" << endl;
		cout << "  2. " << asimovfile2 << endl;
		cout << "  3. " << asimovfile3 << endl;
		cout << "  4. " << asimovfile4 << endl;
	}
	else {
		cout << "  Loading Asimov points from file: " << asimovfile << endl;
		bool loaded = pCache->loadPoints(asimovfile);
		if ( !loaded ){
			cout << "  Error loading file. Exit." << endl;
			exit(1);
		}
		cout << "  Setting point number: " << arg->asimov[cId] << endl;
		pCache->setPoint(c,arg->asimov[cId]-1);
	}
	setAsimovObservables(c);
}

///
/// print usage and exit
///
void GammaComboEngine::usage()
{
	cout << "USAGE\n\n"
		"  # Compute combination 1, make a 1D Prob scan for variable a_gaus:\n"
		"  " << execname << " -c 1 -i --var a_gaus --ps 1\n\n"
		"  # Add combinations 1, 2, 3, add to the same plot:\n"
		"  " << execname << " -c 1 -c 2 -c 3 -i --var a_gaus --ps 1\n\n"
		"  # Add or remove measurements to/from existing cominers:\n"
		"  " << execname << " -i --var a_gaus -c 3:+1,+2,-3\n\n"
		"  # Make a 2D Prob scan\n"
		"  " << execname << " -c 4 --var a_gaus --var b_gaus -i\n\n"
		"  # Re-plot a previous 2D Prob scan with 2D CL and best fit points\n"
		"  " << execname << " -c 4 --var a_gaus --var b_gaus -i -a plot --2dcl --ps 1\n" << endl;
	//cout << "Available plugin toy control plots:\n"
	//"===================================\n"
	//"\n"
	//"All control plots are produced when option --controlplots is given.\n"
	//"If, in addition, -p <n> is given, only one specific plot is produced.\n"
	//" (1) technical overview\n"
	//" (2) summary\n"
	//" (3) nuisances\n"
	//" (4) observables\n"
	//" (5) chi2 distributions\n"
	//" (6) chi2 parabolas\n" << endl;
	print();
	exit(0);
}

///
/// Print the available PDFs.
///
void GammaComboEngine::printPdfs()
{
	cout << "AVAILABLE MEASUREMENTS" << endl;
	cout << endl;
	for ( int i=0; i<pdf.size(); i++ ){
		if ( pdf[i]==0 ) continue;
		if      ( i< 10 ) printf("   (%i) %s\n", i, pdf[i]->getTitle().Data());
		else if ( i<100 ) printf("  (%2i) %s\n", i, pdf[i]->getTitle().Data());
		else              printf( " (%3i) %s\n", i, pdf[i]->getTitle().Data());
	}
	cout << endl;
}

///
/// Print the availabe Combinations.
///
void GammaComboEngine::printCombinations()
{
	cout << "AVAILABLE COMBINATIONS" << endl;
	cout << endl;
	for ( int i=0; i<cmb.size(); i++ ){
		if ( cmb[i]==0 ) continue;
		if      ( i< 10 ) printf("   (%i) %s\n", i, cmb[i]->getTitle().Data());
		else if ( i<100 ) printf("  (%2i) %s\n", i, cmb[i]->getTitle().Data());
		else              printf( " (%3i) %s\n", i, cmb[i]->getTitle().Data());
	}
	cout << endl;
}

///
/// Print the content of this engine.
///
void GammaComboEngine::print()
{
	printPdfs();
	printCombinations();
}

///
/// Check the combination argument (-c), exit if it is bad.
///
void GammaComboEngine::checkCombinationArg()
{
	if ( arg->combid.size()==0 ){
		cout << "Please chose a combination ID (-c).\n" << endl;
		printCombinations();
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
/// Check Asimov arg: when only one --asimov argument is given
/// with the ID 0, it won't do anything. Print a warning in that
/// case.
///
void GammaComboEngine::checkAsimovArg()
{
	if ( arg->asimov.size()==1 && arg->asimov[0]==0 ){
		cout << "WARNING : --asimov 0 found, this won't do anything." << endl;
		cout << "          To run an Asimov toy, the generation point ID" << endl;
		cout << "          needs to be different from 0." << endl;
		cout << endl;
	}
}

///
/// Check color argument and exit if a non-existing color was requested through
/// the --color argument. Colors for one-dimensional plots are defined in
/// GammaComboEngine::defineColors(). Colors for two-dimensional pltos are
/// defined in OneMinusClPlot2d::OneMinusClPlot2d().
///
void GammaComboEngine::checkColorArg()
{
	for ( int i=0; i<arg->color.size(); i++ ){
		// colors for one-dimensional plots
		if ( arg->var.size()==1 ){
			if ( colorsLine.size()<=arg->color[i] ){
				cout << "Argument error --color: No such color for one-dimensional plots. Please choose a color between 0 and " << colorsLine.size()-1 << endl;
				exit(1);
			}
		}
		// colors for two-dimensional plots
		else if ( arg->var.size()==2 ){
			OneMinusClPlot2d p(arg);
			int nMaxColors = p.getNumberOfDefinedColors();
			if ( nMaxColors<=arg->color[i] ){
				cout << "Argument error --color: No such color for two-dimensional plots. Please choose a color between 0 and " << nMaxColors-1 << endl;
				exit(1);
			}
		}
	}
}

///
/// Implement the logic needed to create new combinations on the fly
/// using the -c 26:+12 syntax
///
void GammaComboEngine::makeAddDelCombinations()
{
	// sanity check: the combid and combmodifications vectors should
	// be the same size
	if ( arg->combmodifications.size() != arg->combid.size() ){
		cout << "GammaComboEngine::makeAddDelCombinations() : ERROR : internal inconsistency. \n"
															  "combid and combmodifications vectors not of same size." << endl;
		assert(0);
	}
	// loop over list of modifications
	for ( int i=0; i<arg->combmodifications.size(); i++ ) {
		// get combiner that is to be modified
		Combiner *cOld = cmb[arg->combid[i]];
		// see if anything is to be added or subtracted
		if ( arg->combmodifications[i].size()==0 ) continue;
		// there are modifications to be done!
		cout << "-c : Making a new combination based on combination " << arg->combid[i] << endl;
		// compute name and title of new combiner
		TString nameNew = cOld->getName();
		TString titleNew = cOld->getTitle();
		for ( int j=0; j<arg->combmodifications[i].size(); j++ ){
			int pdfId = abs(arg->combmodifications[i][j]);
			if ( ! pdfExists(pdfId) ){
				cout << "\nERROR: measurement of given ID does not exist: " << pdfId << endl;
				cout << "       Here is a list of available measurements:" << endl;
				printPdfs();
				exit(1);
			}
			if ( arg->combmodifications[i][j]>0 ){
				nameNew += Form("+%i",pdfId);
				titleNew += Form(", + Meas.%i",pdfId);
			}
			else {
				nameNew += Form("-%i",pdfId);
				titleNew += Form(", w/o Meas.%i",pdfId);
			}
		}
		// make the new combiner
		Combiner *cNew = cOld->Clone(nameNew, titleNew);
		// add/delete pdfs
		for ( int j=0; j<arg->combmodifications[i].size(); j++ ){
			int pdfId = abs(arg->combmodifications[i][j]);
			if ( arg->combmodifications[i][j]>0 ){
				cout << "... adding measurement " << pdfId << endl;
				cNew->addPdf(pdf[pdfId]);
			}
			else {
				cout << "... deleting measurement " << pdfId << endl;
				cNew->delPdf(pdf[pdfId]);
			}
		}
		// add to list of combinations to compute this round
		cmb.push_back(cNew);
		arg->combid[i] = cmb.size()-1;
	}
}

///
/// print parameter structure of the combinations into
/// .dot file
///
void GammaComboEngine::printCombinerStructure(Combiner *c)
{
	Graphviz gviz(arg);
	gviz.printCombiner(c);
	gviz.printCombinerLayer(c);
}

///
/// Override default titles of the combinations, if requested
/// on the command line.
///
void GammaComboEngine::customizeCombinerTitles()
{
	for ( int i=0; i<arg->combid.size(); i++ ){
		int combinerId = arg->combid[i];
		Combiner *c = cmb[combinerId];
		if ( i<arg->title.size() ){
			if ( arg->title[i]!=TString("default") ) c->setTitle(arg->title[i]);
		}
	}
}

///
/// Set up the plot.
///
void GammaComboEngine::setUpPlot()
{
	if ( arg->var.size()==1 ){
		plot = new OneMinusClPlot(arg, m_fnamebuilder->getFileNamePlot(cmb), "p-value curves");
	}
	else{
		plot = new OneMinusClPlot2d(arg, m_fnamebuilder->getFileNamePlot(cmb), "p-value contours");
	}
	plot->disableLegend(arg->plotlegend);
}

///
/// Save the plot to disc.
///
void GammaComboEngine::savePlot()
{
  if ( arg->hfagLabel!="" ) HFAGLabel( arg->hfagLabel, arg->plotHFAGLabelPosX, arg->plotHFAGLabelPosY, arg->plotHFAGLabelScale );
	plot->save();
}

///
/// Define the colors of curves and numbers in the 1D plots.
/// It is important to call this only after new combinations were
/// made using the makeAddDelCombinations() mechanism.
///
void GammaComboEngine::defineColors()
{
	// no --color option was given on the command line
	//if ( arg->color.size()==0 )
	//{
		// define line colors for 1-CL curves
		colorsLine.push_back(arg->combid.size()==1 ? kBlue-8 : kBlue-5);
		colorsLine.push_back(kGreen-8);
		colorsLine.push_back(kOrange-8);
		colorsLine.push_back(kMagenta-6);

		// define text colors for drawn central values
		colorsText.push_back(arg->combid.size()==1 ? kBlack : TColor::GetColor("#23236b"));
		colorsText.push_back(TColor::GetColor("#234723"));
		colorsText.push_back(kOrange+3);
		colorsText.push_back(kMagenta-8);
	//}
	//else
	//{
		colorsLine.push_back(TColor::GetColor("#1b9e77")); // sea green
		colorsLine.push_back(TColor::GetColor("#d95f02")); // dark orange
		colorsLine.push_back(TColor::GetColor("#7570b3")); // medium purple
		colorsLine.push_back(TColor::GetColor("#e7298a")); // medium violet red
		colorsLine.push_back(TColor::GetColor("#66a61e")); // forest green
		colorsLine.push_back(TColor::GetColor("#e6ab02")); // goldenrod
    colorsLine.push_back(TColor::GetColor("#a6761d")); // chocolate
    colorsLine.push_back(TColor::GetColor("#e31a1c")); // red
    colorsLine.push_back(TColor::GetColor("#984ea3")); // darkish purple
    colorsLine.push_back(kBlack); // black

		// from http://colorbrewer2.org with:
		//   number of data classes: 6
		//   nature of data:         qualitative
		//   second colour scheme

		ColorBuilder cb;
		for ( int i=4; i<colorsLine.size(); i++ ){
      //colorsText.push_back(cb.darklightcolor(colorsLine[i], 0.5));
		  colorsText.push_back( colorsLine[i] );
    }
    //}

	// default for any additional scanner
	for ( int i=colorsLine.size(); i<arg->combid.size(); i++ ){
		colorsLine.push_back(kBlue-8 + i);
		colorsText.push_back(kBlue-2 + i);
	}

  for ( int i=0; i<arg->combid.size(); i++ ) {
    if ( i>= arg->fillstyle.size() ) fillStyles.push_back( 1001 );
    else fillStyles.push_back( arg->fillstyle[i] );
  }
}

///
/// Define scan strategy for a 2D scan.
///
void GammaComboEngine::scanStrategy2d(MethodProbScan *scanner, ParameterCache *pCache)
{
	int nStartingPoints = pCache->getNPoints();
	// if no starting values loaded do the default thing
	if ( nStartingPoints==0 ){
		cout << "\nPerforming default 2D scan:\n"
			" 1. scan in first variable:  " + scanner->getScanVar1Name() + "\n"
			" 2. scan in second variable: " + scanner->getScanVar2Name() + "\n"
			" 3. scan starting from each solution found in 1. and 2." << endl;
		Combiner *c = scanner->getCombiner();
		cout << "\n1D scan for X variable, " + scanner->getScanVar1Name() + ":\n" << endl;
		MethodProbScan *s1 = new MethodProbScan(c);
		s1->setScanVar1(scanner->getScanVar1Name());
		s1->initScan();
		scanStrategy1d(s1,pCache);
		if ( arg->verbose ) s1->printLocalMinima();

		cout << "\n1D scan for Y variable, " + scanner->getScanVar2Name() + ":\n" << endl;
		MethodProbScan *s2 = new MethodProbScan(c);
		s2->setScanVar1(scanner->getScanVar2Name());
		s2->setXscanRange(arg->scanrangeyMin,arg->scanrangeyMax);
		s2->initScan();
		scanStrategy1d(s2,pCache);
		if ( arg->verbose ) s2->printLocalMinima();

		cout << "\n2D scan for " + scanner->getScanVar1Name() + " and " + scanner->getScanVar2Name() + ":\n" << endl;
		vector<RooSlimFitResult*> solutions;
		for ( int i=0; i<s1->getNSolutions(); i++ ) solutions.push_back(s1->getSolution(i));
		for ( int i=0; i<s2->getNSolutions(); i++ ) solutions.push_back(s2->getSolution(i));
		// \todo remove similar solutions from list
		for ( int j=0; j<solutions.size(); j++ ){
			cout << "2D scan " << j+1 << " of " << solutions.size() << " ..." << endl;
			scanner->loadParameters(solutions[j]);
			scanner->scan2d();
		}
		delete s1;
		delete s2;
	}
	// otherwise load each starting value found
	else {
		cout << "\nPerforming 2D scan from provided starting points." << endl;
		cout << "Number of scans to run: " << nStartingPoints << endl;
		for (int i=0; i<nStartingPoints; i++){
			pCache->setPoint(scanner,i);
			scanner->scan2d();
		}
	}
}


///
/// Perform the prob scan.
///
/// \param scanner - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dProbScan(MethodProbScan *scanner, int cId)
{
	// load start parameters
	ParameterCache *pCache = new ParameterCache(arg);
	loadStartParameters(scanner, pCache, cId);

	scanner->initScan();
	scanStrategy1d(scanner, pCache);
	cout << "\nResults:" << endl;
	cout <<   "========\n" << endl;
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
		if (!arg->isAction("plugin")){
			scanner->saveScanner(m_fnamebuilder->getFileNameScanner(scanner));
			pCache->cacheParameters(scanner,m_fnamebuilder->getFileNamePar(scanner));
		}
	}
}

///
/// Perform the 1D plugin scan. Runs toys in batch mode, and
/// reads them back in.
///
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
		scannerPlugin->calcCLintervals();
	}
	if ( !arg->isAction("pluginbatch") ){
		scannerPlugin->saveScanner(m_fnamebuilder->getFileNameScanner(scannerPlugin));
	}
}

///
/// Perform the 2D plugin scan. Runs toys in batch mode, and
/// reads them back in.
///
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
		scannerPlugin->saveScanner(m_fnamebuilder->getFileNameScanner(scannerPlugin));
		// plot chi2
		cout << "making full chi2 plot ..." << endl;
		OneMinusClPlot2d* plotf = new OneMinusClPlot2d(arg, plot->getName()+"_plugin_full", "p-value histogram: "+scannerPlugin->getTitle());
		scannerPlugin->plotOn(plotf);
		plotf->DrawFull();
		plotf->save();
	}
}

///
/// Perform the 1D berger boos scan. Runs toys in batch mode, and
/// reads them back in.
///
/// \param scannerPlugin - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dBergerBoosScan(MethodBergerBoosScan *scannerBergerBoos, int cId)
{
	scannerBergerBoos->initScan();
  scannerBergerBoos->setNBergerBoosPointsPerScanpoint( arg->nBBpoints );
	if ( arg->isAction("bbbatch") ){
		scannerBergerBoos->scan1d(arg->nrun);
	}
	else {
		scannerBergerBoos->readScan1dTrees(arg->jmin[cId],arg->jmax[cId]);
		scannerBergerBoos->calcCLintervals();
	}
	if ( !arg->isAction("bbbatch") ){
		scannerBergerBoos->saveScanner(m_fnamebuilder->getFileNameScanner(scannerBergerBoos));
	}
}

/// Perform the coverage scanner
///
/// \param scanner - the scanner to run the scan with
/// \param cId - the id of this combination on the command line
void GammaComboEngine::make1dCoverageScan(MethodCoverageScan *scanner, int cId)
{
  // load coverage point parameters (this can be done automatically)
  ParameterCache *pCache = new ParameterCache(arg);
  if ( arg->loadParamsFile.size() != arg->combid.size() ) {
    cout << "\nERROR : For a Coverage scan you must pass a parameter file (--parfile) to throw the toys from. You need one parfile per combiner" << endl;
    exit(1);
  }
  pCache->loadPoints( arg->loadParamsFile[cId] );

  // do scan
  scanner->initScan();
  scanner->setParameterCache( pCache ); // this can be passed directly to scan
  if ( arg->isAction("coveragebatch") ) {
    scanner->scan1d(arg->nrun);
  }
  else {
    scanner->readScan1dTrees( arg->jmin[cId], arg->jmax[cId] );
    scanner->saveScanner( m_fnamebuilder->getFileNameScanner( scanner ) );
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
		scanner->setDrawSolution(arg->plotsolutions[cId]);
		scanner->plotOn(plot);
		int colorId = cId;
		if ( arg->color.size()>cId ) colorId = arg->color[cId];
		scanner->setLineColor(colorsLine[colorId]);
		scanner->setTextColor(colorsText[colorId]);
    scanner->setFillStyle(fillStyles[cId]);
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
	cout << "Scan strategy:" << endl;
	cout << "==============\n" << endl;
	int nStartingPoints = pCache->getNPoints();
	if ( nStartingPoints==0 ) {
		cout << "1. perform an initial scan" << endl;
		cout << "2. perform an additional scan starting from each solution found\n" << endl;
		cout << "first scan ..." << endl;
		scanner->scan1d();
		if ( !arg->probforce ){
      vector<RooSlimFitResult*> firstScanSolutions = scanner->getSolutions();
			for ( int i=0; i<firstScanSolutions.size(); i++ ){
        cout << "Scan i: " << i << endl;
        //scanner->loadSolution(i);
        scanner->loadParameters(firstScanSolutions[i]);
        scanner->scan1d(true);
			}
		}
	}
	// otherwise load each starting value found
	else {
		cout << "Scanning from each point found in start parameter file.\n" << endl;
		for (int i=0; i<nStartingPoints; i++){
			cout << "scan " << i+1 << " of " << nStartingPoints << " ..." << endl;
			pCache->setPoint(scanner,i);
			scanner->scan1d();
		}
	}
}

///
/// Make the default 1D plugin plot:
///  - curve for the prob scan
///  - points with error bars for the plugin scan
///
/// \param sPlugin - the plugin scanner
/// \param sProb - the prob scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dPluginPlot(MethodPluginScan *sPlugin, MethodProbScan *sProb, int cId)
{
	if ( arg->isQuickhack(17) ) {
    make1dPluginOnlyPlot(sPlugin, cId);
    sProb->setLineColor(kBlack);
    sProb->setDrawSolution(arg->plotsolutions[cId]);
    sProb->plotOn(plot);
  }
  else {
    make1dProbPlot(sProb, cId);
    sPlugin->setLineColor(kBlack);
    sPlugin->setDrawSolution(arg->plotsolutions[cId]);
    sPlugin->plotOn(plot);
  }
	plot->Draw();
}

///
/// Make the default 2D plugin plot:
///  - contours for the the prob scan
///  - contours for the the plugin scan
///
/// \param sPlugin - the plugin scanner
/// \param sProb - the prob scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make2dPluginPlot(MethodPluginScan *sPlugin, MethodProbScan *sProb, int cId)
{
	if ( arg->isQuickhack(18) ) {
    sProb->setTitle(sProb->getTitle() + "PROB");
    sPlugin->setTitle(sPlugin->getTitle() + "PLUGIN");
  }
  else {
    sProb->setTitle(sProb->getTitle() + " (Prob)");
    sPlugin->setTitle(sPlugin->getTitle() + " (Plugin)");
  }
	sProb->setDrawSolution(arg->plotsolutions[cId]);
	sProb->setLineColor(colorsLine[cId]);
	sPlugin->setDrawSolution(arg->plotsolutions[cId]);
	if ( arg->isQuickhack(17) ) {
    sPlugin->plotOn(plot);
    sProb->plotOn(plot);
  }
  else {
    sProb->plotOn(plot);
    sPlugin->plotOn(plot);
  }
	plot->Draw();
}

///
/// Make the plugin-only plot that only shows the plugin
/// as continuous lines.
///
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
  sPlugin->setFillStyle(fillStyles[cId]);
	sPlugin->setDrawSolution(arg->plotsolutions[cId]);
	sPlugin->plotOn(plot);
	plot->Draw();
}

///
/// Make the plugin-only 2D plot.
///
/// \param sPlugin - the plugin scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make2dPluginOnlyPlot(MethodPluginScan *sPlugin, int cId)
{
	sPlugin->setDrawSolution(arg->plotsolutions[cId]);
	sPlugin->plotOn(plot);
	plot->Draw();
}

///
/// Make the 1D coverage plot
///
/// \param scanner - the coverage scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make1dCoveragePlot(MethodCoverageScan *scanner, int cId)
{
  scanner->plot();
}

///
/// Make a 2D prob scan.
/// - load start parameters
/// - perform scan
/// - save scanner and parameters
///
/// \param scanner - the scanner
/// \param cId - the id of this combination on the command line
///
void GammaComboEngine::make2dProbScan(MethodProbScan *scanner, int cId)
{
	// load start parameters
	ParameterCache *pCache = new ParameterCache(arg);
	loadStartParameters(scanner, pCache, cId);
	// scan
	scanner->initScan();
	scanStrategy2d(scanner,pCache);
	cout << endl;
	scanner->printLocalMinima();
	// save
	scanner->saveScanner(m_fnamebuilder->getFileNameScanner(scanner));
	pCache->cacheParameters(scanner, m_fnamebuilder->getFileNamePar(scanner));
}

///
/// Make the 2D plot for a prob scanner.
///
void GammaComboEngine::make2dProbPlot(MethodProbScan *scanner, int cId)
{
	// plot full
	OneMinusClPlot2d* plotf = new OneMinusClPlot2d(arg, m_fnamebuilder->getFileNamePlotSingle(cmb, cId)+"_full", "p-value histogram: "+scanner->getTitle());
	scanner->plotOn(plotf);
	plotf->DrawFull();
	plotf->save();
	// contour plot
	scanner->setDrawSolution(arg->plotsolutions[cId]);
	scanner->setLineColor(colorsLine[cId]);
	scanner->plotOn(plot);
	// only draw the plot once when multiple scanners are plotted,
	// else we end up with too many graphs, and the transparency setting
	// gets screwed up
	if ( cId==arg->combid.size()-1 ){
		plot->Draw();
		plot->Show();
	}
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
/// Helper function for scan(). Adjusts ranges, if requested
/// (only possible before combining).
///
void GammaComboEngine::adjustRanges(Combiner *c, int cId)
{
	if ( cId<arg->physRanges.size() ){
		for ( int j=0; j<arg->physRanges[cId].size(); j++ ){
      c->adjustPhysRange(arg->physRanges[cId][j].name, arg->physRanges[cId][j].min, arg->physRanges[cId][j].max);
		}
	}
  if ( cId<arg->removeRanges.size() ){
    for ( int j=0; j<arg->removeRanges[cId].size(); j++ ) {
      if ( arg->removeRanges[cId][j] == "all" ) {
        const RooArgSet *pars = (RooArgSet*)c->getParameters();
        TIterator *it = pars->createIterator();
        while ( RooRealVar* par = (RooRealVar*)it->Next() ) {
          par->removeRange();
        }
      }
      else {
        c->adjustPhysRange( arg->removeRanges[cId][j], -999, -999 );
      }
    }
  }
}

///
/// Helper function for scan(): Makes named sets for any toy variations that were requested
///
void GammaComboEngine::setupToyVariationSets(Combiner *c, int cId)
{
  if ( cId < arg->randomizeToyVars.size() ) {
    TString toyVarList = "";
    for ( int j=0; j<arg->randomizeToyVars[cId].size(); j++ ) {
      toyVarList += arg->randomizeToyVars[cId][j];
      if ( j < arg->randomizeToyVars[cId].size()-1 ) toyVarList += ",";
    }
    c->getWorkspace()->defineSet( "toy_"+c->getPdfName(), toyVarList.Data() );
  }
}

///
/// Helper function for scan(): Checks if for a given combid (the
/// running index of the -c argument) a start parameter file was
/// configured (-l) argument. If so, it is returned, else the default
/// name is returned.
///
TString GammaComboEngine::getStartParFileName(int cId)
{
	if ( arg->loadParamsFile.size()<=cId ) return m_fnamebuilder->getFileNameStartPar(cmb[cId]);
	if ( arg->loadParamsFile[cId].EqualTo("default") ) return m_fnamebuilder->getFileNameStartPar(cmb[cId]);
	return arg->loadParamsFile[cId];
}

///
/// Checks if a given variable name is in the list of observables
/// of a combiner. The check is slightly sloppy as it ignores the
/// unique ID.
///
/// \param c		- the combiner
/// \param scanVar 	- the scan variable name
/// \return	true if included, else false
///
bool GammaComboEngine::isScanVarObservable(Combiner *c, TString scanVar)
{
	vector<string> obs = c->getObservableNames();
	for ( int i=0; i<obs.size(); i++ ){
		if ( scanVar.Contains(obs[i]) ) return true;
	}
	return false;
}

///
/// Helper function to set up a scan for an observable, tightens
/// the chi2 constraint.
///
/// \param c		- the combiner
/// \param scanVar 	- the scan variable name (must be an observable)
///
void GammaComboEngine::tightenChi2Constraint(Combiner *c, TString scanVar)
{
	cout << "\n--var " << scanVar << ": Setting up a scan for an observable ..." << endl;
	PDF_Abs* pdf = c->getPdfProvidingObservable(scanVar);
	if ( pdf==0 ){
		cout << "GammaComboEngine::tightenChi2Constraint() : ERROR : no PDF found that contains the observable '" << scanVar << "'. Exit." << endl;
		exit(1);
	}
	float scale = 0.1;
	cout << "... observable error is multiplied by a factor " << scale << endl;
	pdf->ScaleError(scanVar, scale);
	pdf->buildCov();
	pdf->buildPdf();
}

///
/// write batch scripts
///
void GammaComboEngine::writebatchscripts()
{
  m_batchscriptwriter->writeScripts(arg, &cmb);
  exit(0);
}

///
/// make latex
///
void GammaComboEngine::makeLatex(Combiner *c)
{
  for ( unsigned int p=0; p < c->getPdfs().size(); p++) {
    PDF_Abs *pdf = c->getPdfs()[p];
    LatexMaker m( c->getName(), pdf );
    m.writeFile();
  }
}

///
/// save workspace
///
void GammaComboEngine::saveWorkspace( Combiner *c, int i )
{
  // if --pr then make the ranges
  if ( arg->enforcePhysRange ) setLimit( c->getParameters(), "phys" );

  // first write a copy THE PDF into the workspace
  const RooAbsPdf *thePdf         = c->getPdf();
  const RooArgSet *theParameters  = c->getParameters();
  const RooArgSet *theObservables = c->getObservables();

  RooAbsPdf *savePdf = (RooAbsPdf*)thePdf->Clone("ThePdf");
  c->getWorkspace()->import( *savePdf, Silence() );
  c->getWorkspace()->defineSet( "TheParameters", *theParameters );
  c->getWorkspace()->defineSet( "TheObservables", *theObservables );

  // set the name of the workspace associated with combiner id
  int cId = arg->combid[i];
  c->getWorkspace()->SetName( Form("w%d",cId) );

  // if the first instance then write directly to file
  if ( i==0 ) {
    c->getWorkspace()->writeToFile( arg->save );
  }
  // otherwise add it to the file
  else {
    TFile *tf = TFile::Open( arg->save, "UPDATE" );
    c->getWorkspace()->Write();
    tf->Close();
  }
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

		// fix parameters according to the command line - only possible before combining
		fixParameters(c, i);

		// configure names to run an Asimov toy - only possible before combining
		if ( arg->isAsimovCombiner(i) ) configureAsimovCombinerNames(c, i);

		// configure scans for observables - this part is only possible before combining
		if ( isScanVarObservable(c, arg->var[0]) ){
			tightenChi2Constraint(c, arg->var[0]);
		}
		if ( arg->var.size()==2 && isScanVarObservable(c, arg->var[1]) ){
			tightenChi2Constraint(c, arg->var[1]);
		}

		// combine
		c->combine();
		if ( !c->isCombined() ) continue; // error during combining

		// adjust ranges according to the command line - only possible before combining
		adjustRanges(c, i);

    // set up parameter sets for the parameters to vary within the toys (if requested)
    setupToyVariationSets(c, i);

		// make graphviz dot files
		printCombinerStructure(c);

		// set an asimov toy - only possible after combining
		if ( arg->isAsimovCombiner(i) ) loadAsimovPoint(c, i);

		// configure scans for observables - this part is only possible after combining
		// add the observable(s) to the list of parameters
		if ( isScanVarObservable(c, arg->var[0]) ){
			c->getWorkspace()->extendSet(c->getParsName(), arg->var[0]);
		}
		if ( arg->var.size()==2 && isScanVarObservable(c, arg->var[1]) ){
			c->getWorkspace()->extendSet(c->getParsName(), arg->var[1]);
		}

		// printout and latex
		c->print();
		if ( arg->debug ) c->getWorkspace()->Print("v");
    if ( arg->save != "" ) saveWorkspace( c, i );
    if ( arg->latex ) makeLatex( c );
    if ( arg->info || arg->latex || arg->save!="" ) continue;

		/////////////////////////////////////////////////////
		//
		// PROB
		//
		/////////////////////////////////////////////////////

		if ( !arg->isAction("plugin") && !arg->isAction("pluginbatch") && !arg->isAction("coverage") && !arg->isAction("coveragebatch") && !arg->isAction("bb") && !arg->isAction("bbbatch") )
		{
			MethodProbScan *scannerProb = new MethodProbScan(c);
			// pvalue corrector
			if ( arg->coverageCorrectionID>0 ) {
				PValueCorrection *pvalueCorrector = new PValueCorrection(arg->coverageCorrectionID, arg->verbose);
				pvalueCorrector->readFiles(m_fnamebuilder->getFileBaseName(c),arg->coverageCorrectionPoint,false); // false means for prob
				pvalueCorrector->write("root/pvalueCorrection_prob.root");
				scannerProb->setPValueCorrector(pvalueCorrector);
			}

			// 1D SCANS
			if ( arg->var.size()==1 )
			{
				if ( arg->isAction("plot") ){
					scannerProb->loadScanner(m_fnamebuilder->getFileNameScanner(scannerProb));
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
					scannerProb->loadScanner(m_fnamebuilder->getFileNameScanner(scannerProb));
				}
				else{
					make2dProbScan(scannerProb, i);
				}
				make2dProbPlot(scannerProb, i);
			}
		}

		/////////////////////////////////////////////////////
		//
		// PLUGIN
		//
		/////////////////////////////////////////////////////

		if ( arg->isAction("plugin") || arg->isAction("pluginbatch") )
		{
			// 1D SCANS
			if ( arg->var.size()==1 )
			{
				if ( arg->isAction("pluginbatch") ){
					MethodProbScan *scannerProb = new MethodProbScan(c);
					make1dProbScan(scannerProb, i);
					MethodPluginScan *scannerPlugin = new MethodPluginScan(scannerProb);
					make1dPluginScan(scannerPlugin, i);
				}
				//if ( arg->isAction("pluginhybridbatch") ){
				//// Hybrid Plugin: compute a second profile likelihood to define the parameter evolution
				//cout << "HYBRID PLUGIN: preparing profile likelihood to be used for parameter evolution:" << endl;
				//ParameterCache *pCache = new ParameterCache(arg, m_fnamebuilder->getFileBaseName(cmb[arg->pevid[0]]));
				//pCache->loadPoints();
				//MethodProbScan *scanner3 = new MethodProbScan(cmb[arg->pevid[0]]);
				//scanner3->initScan();
				//scanStrategy1d(scanner3,pCache);
				//scanner3->confirmSolutions();
				//scanner3->printLocalMinima();
				//scanner2->setParevolPLH(scanner3);
				//}
				else if ( arg->isAction("plugin") ){
					// Create the Prob scanner: load from disc if it exists, else redo the scan.
					// We don't need the prob scanner for the plugin only plot, if we either just
					// want to replot it.
					MethodProbScan *scannerProb = new MethodProbScan(c);
					if (    !arg->plotpluginonly
						|| ( arg->plotpluginonly && !arg->isAction("plot") ) ){
						if ( FileExists(m_fnamebuilder->getFileNameScanner(scannerProb)) ){
							scannerProb->loadScanner(m_fnamebuilder->getFileNameScanner(scannerProb));
						}
						else {
							cout << "\nWARNING : Couldn't load the Prob scanner, will rerun the Prob" << endl;
							cout <<   "          scan now. You should have run the Prob scan locally" << endl;
							cout <<   "          before running the Plugin scan." << endl;
							cout <<   "          missing file: " << m_fnamebuilder->getFileNameScanner(scannerProb) << endl;
							cout << endl;
							make1dProbScan(scannerProb, i);
						}
					}
					// create Plugin scanner
					MethodPluginScan *scannerPlugin = new MethodPluginScan(scannerProb);
					if ( arg->isAction("plot") ){
						scannerPlugin->loadScanner(m_fnamebuilder->getFileNameScanner(scannerPlugin));
					}
					else {
						if ( arg->coverageCorrectionID>0 ) {
							PValueCorrection *pvalueCorrector= new PValueCorrection(arg->coverageCorrectionID, arg->verbose);
							pvalueCorrector->readFiles(m_fnamebuilder->getFileBaseName(c),arg->coverageCorrectionPoint,true); // true means for plugin
							pvalueCorrector->write("root/pvalueCorrection_plugin.root");
							scannerPlugin->setPValueCorrector(pvalueCorrector);
						}
						make1dPluginScan(scannerPlugin, i);
					}
					if ( arg->plotpluginonly ){
						make1dPluginOnlyPlot(scannerPlugin, i);
					}
					else {
						make1dPluginPlot(scannerPlugin, scannerProb, i);
					}
				}


			}
			// 2D SCANS
			else if ( arg->var.size()==2 ) {
				if ( arg->isAction("pluginbatch") ){
					MethodProbScan *scannerProb = new MethodProbScan(c);
					make2dProbScan(scannerProb, i);
					MethodPluginScan *scannerPlugin = new MethodPluginScan(scannerProb);
					make2dPluginScan(scannerPlugin, i);
				}
				else if ( arg->isAction("plugin") ){
					MethodProbScan *scannerProb = new MethodProbScan(c);
					if ( ! ( arg->isAction("plot") && arg->plotpluginonly ) ){
						// we don't need the prob scanner if we just want to replot the plugin only
						scannerProb->loadScanner(m_fnamebuilder->getFileNameScanner(scannerProb));
					}
					MethodPluginScan *scannerPlugin = new MethodPluginScan(scannerProb);
					if ( arg->isAction("plot") ){
						scannerPlugin->loadScanner(m_fnamebuilder->getFileNameScanner(scannerPlugin));
					}
					else {
						make2dPluginScan(scannerPlugin, i);
					}
					if ( arg->plotpluginonly ){
						make2dPluginOnlyPlot(scannerPlugin, i);
					}
					else {
						make2dPluginPlot(scannerPlugin, scannerProb, i);
					}
				}
			}
		}

		/////////////////////////////////////////////////////
		//
		// COVERAGE
		//
		/////////////////////////////////////////////////////

    if ( arg->isAction("coverage") || arg->isAction("coveragebatch") )
    {
			if ( arg->var.size()!=1 ) {
        cerr << "ERROR -- you can only scan in 1D for a coverage check" << endl;
        exit(1);
      }
      MethodCoverageScan *coverageScan = new MethodCoverageScan(c);
      if ( arg->isAction("coveragebatch") ) {
        make1dCoverageScan( coverageScan, i );
      }
      else if ( arg->isAction("coverage") ) {
        if ( arg->isAction("plot") ) {
          if ( FileExists(m_fnamebuilder->getFileNameScanner(coverageScan)) ) {
            coverageScan->loadScanner(m_fnamebuilder->getFileNameScanner(coverageScan));
          }
          else {
            cout << "\nERROR : Couldn't load the coverage scanner: " << m_fnamebuilder->getFileNameScanner(coverageScan) << endl;
            exit(1);
          }
        }
        else {
          make1dCoverageScan( coverageScan, i );
        }
        make1dCoveragePlot( coverageScan, i );
      }
    }

		/////////////////////////////////////////////////////

		if ( i<arg->combid.size()-1 ) {
			cout << "\n-- now starting -c " << arg->combid[i+1] << " ------------------------------------------------------------------\n" << endl;
		}
	}
}

///
/// run the ROOT application, if the -i flag for interactive
/// mode was set.
///
void GammaComboEngine::runApplication()
{
	if ( arg->interactive ){
		cout << "Exit with Ctrl+c" << endl;
		theApp->Run();
	}
}

///
/// print the initial banner
///
void GammaComboEngine::printBanner()
{
	const char* VTAG="0.9";
	cout << endl
		<< "\033[1mGammaCombo v" << VTAG << " -- Developed by Till Moritz Karbach\033[0m " << endl
		<< "                   Copyright (C) 2014, moritz.karbach@gmail.com" << endl
		<< "                   All rights reserved under GPLv3, http://www.gnu.org/licenses/gpl.txt" << endl << endl ;
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
	checkAsimovArg();
	//scaleDownErrors();
	if ( arg->nosyst ) disableSystematics();
	makeAddDelCombinations();
  if ( arg->nbatchjobs>0 ) writebatchscripts();
	customizeCombinerTitles();
	setUpPlot();
	scan(); // most thing gets done here
  if ( arg->info || arg->latex || arg->save!="" ) return; // if only info is requested then we can go home
	if (!arg->isAction("pluginbatch") && !arg->isAction("coveragebatch") && !arg->isAction("coverage") ) savePlot();
	cout << endl;
	t.Stop();
	t.Print();
	runApplication();
}

