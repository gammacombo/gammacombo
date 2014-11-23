/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef OptParser_h
#define OptParser_h

#include <iostream>
#include <stdlib.h>

#include "Utils.h"
#include "tclap/CmdLine.h"

using namespace std;
using namespace Utils;
using namespace TCLAP;

class OptParser
{
	public:
		OptParser();
		~OptParser();

		void bookOption(TString opt);
		void bookAllOptions();
		void bookPlottingOptions();
		void bookPluginOptions();
		void bookProbOptions();
		void bookFlowcontrolOptions();
		void parseArguments(int argc, char* argv[]);
		bool isAction(TString s);
		bool isQuickhack(int id);

		vector<TString>		action;
		vector<vector<int> >	addpdf;  // format is addpdf = [0:[combinerId1,pdfId1,pdfId2], 1:[combinerId1,pdfId1,pdfId2]]
		vector<int>		asimov;
		bool			cacheStartingValues;
		vector<int>		color;
		vector<int>		combid;
		bool			controlplot;
		int 						coverageCorrectionID;
		int 						coverageCorrectionPoint;
		bool            debug;
		vector<vector<int> > delpdf;
		int		          digits;
		bool            enforcePhysRange;
		vector<vector<FixPar> >	fixParameters;
		TString	group;
		TString	groupPos;
		int             id;
		bool            importance;
		bool		        interactive;
		vector<int>   	jmax;
		vector<int>   	jmin;
		TString         jobdir;
		bool            largest;
		vector<TString> loadParamsFile;
		bool            lightfiles;
		int             nBBpoints;
		int             ndiv;
		int             ndivy;
		bool            nosyst;
		int		          npoints1d;
		int		          npoints2dx;
		int		          npoints2dy;
		int             npointstoy;
		int		          nrun;
		int		          ntoys;
		TString 				parsavefile;
		bool		        parevol;
		vector<int>	    pevid;
		bool            plot2dcl;
		int             plotid;
		bool            plotlog;
		bool						plotlegend;
		float           plotlegx;
		float           plotlegy;
		float           plotgroupx;
		float           plotgroupy;
		bool            plotmagnetic;
		int             plotnsigmacont;
		bool            plotpluginonly;
		bool            plotpulls;
		bool            plotprelim;
		int             plotsolutions;
		bool            plotunoff;
		bool            pluginext;
		float           pluginPlotRangeMin;
		float           pluginPlotRangeMax;
		bool		        probforce;
		bool		        probimprove;
		vector<float>   printnuisances1d;
		vector<float>   printnuisances2dx;
		vector<float>   printnuisances2dy;
		bool						printcor;
		vector<int>   	qh;
		vector<TString> relation;
		vector<float>   savenuisances1d;
		vector<float>   savenuisances2dx;
		vector<float>   savenuisances2dy;
		bool		        scanforce;
		float           scanrangeMin;
		float           scanrangeMax;
		float           scanrangeyMin;
		float           scanrangeyMax;
		vector<TString> title;
		bool            usage;
		vector<TString> var;
		bool		        verbose;

	private:
		int 	convertToIntWithCheck(TString parseMe, TString usage);
		void defineOptions();
		void parsePosition(TString parseMe, float &x, float &y);
		void parseRange(TString parseMe, float &min, float &max);
		bool parseAssignment(TString parseMe, TString &name, float &value);
		void parseCombinerPdfList(TCLAP::MultiArg<string> &arg, vector<vector<int> > &output);
		void parseCombinerString(TString parseMe, int& resultCmbId, vector<int>& resultAddPdf, vector<int>& resultDelPdf);
		vector<TString> availableOptions;
		vector<TString> bookedOptions;
};

#endif
