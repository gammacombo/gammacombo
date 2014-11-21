/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Nov 2014
 *
 * Class that plots the parameter evolution of the nuisance parameters
 * from a prob scan.
 *
 **/

#ifndef ParameterEvolutionPlotter_h
#define ParameterEvolutionPlotter_h

#include "TCanvas.h"
#include "MethodProbScan.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ParameterEvolutionPlotter
{
	public:

		ParameterEvolutionPlotter(MethodProbScan *scanner);
		~ParameterEvolutionPlotter();

		void	plotParEvolution();
		void	plotObsScanCheck();

	private:

		vector<float>	getLocalMinimaY(TH1F* h);

		OptParser 	*arg;			///< command line arguments
		RooWorkspace 	*w;			///< a clone of the scanner's workspace
		const vector<RooSlimFitResult*>&	allResults;		///< all results of all scan points
		const vector<RooSlimFitResult*>&	curveResults;		///< only the results of scan points that were accepted into the CL curve
		TString		title;			///< canvas title
		TString		name;			///< scanner name, part of the file name of the plots
		TString		parsName;		///< name of parameter set inside the workspace
		TString		obsName;		///< name of observables set inside the workspace
		TString		scanVar1;		///< name of the can variable
		vector<float>   localChi2;		///< chi2 values at the local minima
};

#endif
