/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Aug 2014
 *
 **/

#ifndef GammaComboEngine_h
#define GammaComboEngine_h

#include "ColorBuilder.h"
#include "Combiner.h"
#include "FileNameBuilder.h"
#include "Graphviz.h"
#include "MethodPluginScan.h"
#include "MethodProbScan.h"
#include "OneMinusClPlot.h"
#include "OneMinusClPlot2d.h"
#include "OneMinusClPlotAbs.h"
#include "OptParser.h"
#include "PDF_Abs.h"
#include "ParameterCache.h"
#include "ParameterEvolutionPlotter.h"
#include "TApplication.h"
#include "TColor.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

///
/// The main GammaCombo scanning engine, controlling
/// the application.
///

class GammaComboEngine
{
	public:

		GammaComboEngine(TString name, int argc, char* argv[]);
		~GammaComboEngine();

		void			addPdf(int id, PDF_Abs* pdf, TString title="");
		void			addCombiner(int id, Combiner* cmb);
		void			cloneCombiner(int newId, int oldId, TString name, TString title);
		inline TString		getBasename() const {return basename;}
		Combiner* 		getCombiner(int id) const;
		TString			getFileBaseName();
		PDF_Abs*		getPdf(int id);
		inline OptParser* 	getArg(){return arg;};
		void			newCombiner(int id, TString name, TString title,
					int pdf1=-1, int pdf2=-1, int pdf3=-1, int pdf4=-1, int pdf5=-1,
					int pdf6=-1, int pdf7=-1, int pdf8=-1, int pdf9=-1, int pdf10=-1,
					int pdf11=-1, int pdf12=-1, int pdf13=-1, int pdf14=-1, int pdf15=-1);
		void			print();
		void			printPdfs();
		void			printCombinations();
		void			run();
		void			runApplication();
		static void             scanStrategy1d(MethodProbScan *scanner, ParameterCache *pCache);
		void			scanStrategy2d(MethodProbScan *scanner, ParameterCache *pCache);
		PDF_Abs* 		operator[](int idx);

	private:

		void			makeAddDelCombinations();
		void			checkColorArg();
		void			checkCombinationArg();
		bool			combinerExists(int id) const;
		void			customizeCombinerTitles();
		void			defineColors();
		void			disableSystematics();
		void			fixParameters(Combiner *c, int cId);
		TString			getFileBaseName(Combiner *c);
		TString			getStartParFileFromCommandLine(int cId);
		bool			isAsimovCombiner(int cId);
		bool			isScanVarObservable(Combiner *c, TString scanVar);
		void			make1dPluginOnlyPlot(MethodPluginScan *sPlugin, int cId);
		void			make1dPluginPlot(MethodPluginScan *sPlugin, MethodProbScan *sProb, int cId);
		void			make1dPluginScan(MethodPluginScan *scannerPlugin, int cId);
		void			make1dProbPlot(MethodProbScan *scanner, int cId);
		void			make1dProbScan(MethodProbScan *scanner, int cId);
		void			make2dPluginOnlyPlot(MethodPluginScan *sPlugin);
		void			make2dPluginPlot(MethodPluginScan *sPlugin, MethodProbScan *sProb, int cId);
		void			make2dPluginScan(MethodPluginScan *scannerPlugin, int cId);
		void			make2dProbPlot(MethodProbScan *scanner, int cId);
		void			make2dProbScan(MethodProbScan *scanner, int cId);
		void			printCombinerStructure();
		void			printBanner();
		bool			pdfExists(int id);
		void			savePlot();
		void			scaleDownErrors();
		void			scan();
		void			setAsimovObservables(Combiner* c);
		void			loadAsimovPoint(Combiner* c, int cId);
		void			setUpPlot(TString name);
		void            tightenChi2Constraint(Combiner *c, TString scanVar);
		void			usage();

		TStopwatch 		t;
		TApplication* 		theApp;
		OptParser*		arg;
		vector<PDF_Abs*>	pdf;
		vector<Combiner*> 	cmb;
		FileNameBuilder*	m_fnamebuilder;

		OneMinusClPlotAbs*	plot;
		vector<int> 		colorsLine;
		vector<int> 		colorsText;
		TString 		basename;
		TString 		execname;
};

#endif
