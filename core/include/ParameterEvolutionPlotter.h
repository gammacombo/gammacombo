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

#include "TROOT.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "MethodProbScan.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ParameterEvolutionPlotter
{
    public:

        ParameterEvolutionPlotter(MethodProbScan *scanner);
        ~ParameterEvolutionPlotter();

        void    plotParEvolution();
        void    plotObsScanCheck();

    private:

        void             getLocalMinPositions();
        void             drawLinesAtMinima(TVirtualPad *pad);
        void             drawVerticalRedLine(TVirtualPad *pad, float xpos);
        TGraph*          makeChi2Graph(vector<RooSlimFitResult*> results);
        TGraph*          makeEvolutionGraph(vector<RooSlimFitResult*> results, TString parName);
        TGraphErrors*    makeEvolutionGraphErrors(vector<RooSlimFitResult*> results, TString parName);
        void             saveEvolutionPlots();
        TCanvas*         selectNewCanvas(TString title);
        TVirtualPad*     selectNewPad();
        void             updateCurrentCanvas();

        OptParser   *arg;           ///< command line arguments
        RooWorkspace    *w;         ///< a clone of the scanner's workspace
        vector<RooSlimFitResult*>   allResults;     ///< all results of all scan points
        vector<RooSlimFitResult*>   curveResults;       ///< only the results of scan points that were accepted into the CL curve
        TString     title;          ///< canvas title
        TString     name;           ///< scanner name, part of the file name of the plots
        TString     parsName;       ///< name of parameter set inside the workspace
        TString     obsName;        ///< name of observables set inside the workspace
        TString     scanVar1;       ///< name of the can variable
        vector<float>    m_localMinPositions; ///< positions of the local minima in scan steps
        vector<TCanvas*> m_canvases;          ///< Pointers to the canvases of the plots, see selectNewCanvas().
        int              m_padId;             ///< ID of currently selected pad, see selectNewPad().
};

#endif
