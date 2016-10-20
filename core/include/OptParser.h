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

#include "TRegexp.h"
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
		bool isAsimovCombiner(int id);
		bool isQuickhack(int id);

		vector<TString>	action;
		vector<int>		asimov;
		vector<TString> asimovfile;
		bool			cacheStartingValues;
		vector<int>		color;
		vector<int>		combid;
		vector<vector<int> >	combmodifications; // encodes requested modifications to the combiner ID through the -c 26:+12 syntax,format is [cmbid:[+pdf1,-pdf2,...]]
		bool			controlplot;
		int 			coverageCorrectionID;
		int 			coverageCorrectionPoint;
		bool            debug;
		int		        digits;
		bool            enforcePhysRange;
    vector<int>     fillstyle;
    TString         filenamechange;
		TString         filenameaddition;
		vector<vector<FixPar> >     fixParameters;
		vector<vector<RangePar> >   physRanges;
    vector<vector<TString> >    removeRanges;
    vector<vector<TString> >    randomizeToyVars;
		TString	        group;
		TString	        groupPos;
		TString         hfagLabel;
    TString         hfagLabelPos;
    int             id;
		bool            importance;
    bool            info;
		bool		        interactive;
		vector<int>   	jmax;
		vector<int>   	jmin;
		TString         jobdir;
		bool            largest;
    bool            latex;
		vector<TString> loadParamsFile;
		bool            lightfiles;
    int             batchstartn;
    bool            batcheos;
    int             nbatchjobs;
		int             nBBpoints;
		int             ndiv;
		int             ndivy;
		bool            nosyst;
		int		npoints1d;
		int		npoints2dx;
		int		npoints2dy;
		int             npointstoy;
    int             ncoveragetoys;
		int		nrun;
		int		ntoys;
    int   nsmooth;
		TString 	parsavefile;
		bool		parevol;
		vector<int>	pevid;
		vector<int>     plot2dcl;
    TString         plotext;
		int             plotid;
		bool            plotlog;
		bool		        plotlegend;
		float           plotlegx;
		float           plotlegy;
		float           plotlegsizex;
		float           plotlegsizey;
    TString         plotlegstyle;
		float           plotgroupx;
		float           plotgroupy;
    Double_t        plotHFAGLabelPosX;
    Double_t        plotHFAGLabelPosY;
    Double_t        plotHFAGLabelScale;
		bool            plotmagnetic;
		int             plotnsigmacont;
		bool            plotpluginonly;
		bool            plotpulls;
		bool            plotprelim;
		vector<int>     plotsolutions;
		bool            plotunoff;
    float           plotymin;
    float           plotymax;
		bool            intprob;
		float           pluginPlotRangeMin;
		float           pluginPlotRangeMax;
		bool		probforce;
		bool		probimprove;
		bool		printcor;
    float           printSolX;
    float           printSolY;
		vector<int>   	qh;
    TString         queue;
		vector<TString> relation;
    TString         save;
		vector<float>   savenuisances1d;
		vector<float>   savenuisances2dx;
		vector<float>   savenuisances2dy;
		bool		scanforce;
		float           scanrangeMin;
		float           scanrangeMax;
		float           scanrangeyMin;
		float           scanrangeyMax;
		bool    smooth2d;
		vector<TString> title;
    TString         toyFiles;
		bool            usage;
		vector<TString> var;
		bool		verbose;

    CmdLine cmd;

	private:
		int 		convertToDigitWithCheck(TString parseMe, TString usage);
		int 		convertToIntWithCheck(TString parseMe, TString usage);
		void		defineOptions();
		void		parsePosition(TString parseMe, float &x, float &y, TString usage);
		void		parsePositionAndScale(TString parseMe, Double_t &x, Double_t &y, Double_t &scale, TString usage);
		bool		parseRange(TString parseMe, float &min, float &max);
		bool		parseAssignment(TString parseMe, TString &name, TString &value);
		bool		parseAssignment(TString parseMe, TString &name, float &value);
		void		parseCombinerString(TString parseMe, int& resultCmbId, vector<int>& resultAddDelPdf);
		vector<TString> availableOptions;
		vector<TString> bookedOptions;
};

#endif
