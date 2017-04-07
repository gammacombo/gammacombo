/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#ifndef OneMinusClPlotAbs_h
#define OneMinusClPlotAbs_h

#include "TLegend.h"
#include "TPaveText.h"
#include "TPaveLabel.h"
#include "TGraphErrors.h"
#include "TColor.h"

#include "Utils.h"
#include "MethodAbsScan.h"
#include "OptParser.h"

using namespace Utils;
using namespace RooFit;
using namespace std;

class MethodAbsScan;

class OneMinusClPlotAbs
{
public:
    OneMinusClPlotAbs(OptParser *arg, TString name="c1", TString title="c1");
	~OneMinusClPlotAbs();

    virtual void    addScanner(MethodAbsScan* s, int CLsType = 0);
    inline void     disableLegend(bool yesNo=false){plotLegend = yesNo;};
    inline void     disableSolution(bool yesNo=false){plotSolution = yesNo;};
    virtual void    drawSolutions();
    virtual void    drawLabel(float yPos=0.6){cout << "nothing yet" << endl;};
    virtual void    drawGroup(float yPos=0.6);
    inline TString  getName(){return name;};
    void            save();
    void            setYLogRange(double min=1.e-3, double max=1){ plotLogYMin=min; plotLogYMax=max;};
    inline  void    setFont(int fnum){font = fnum;};
    inline  void    setLabelSize(int lnum){labelsize = lnum;};
    inline  void    setPlotLabel(TString &lname){label = lname;};
	inline void     Show(){m_mainCanvas->Show();};
    virtual void    Draw();

    int font;		///< font code. The last digit disables scaling with the canvas size.
    int labelsize;	///< text size of axis labels in pixels
    int titlesize;	///< text size of axis titles in pixels
    int legendsize;	///< text size of legend entries in pixels

    vector<MethodAbsScan*> scanners;
    vector<int> do_CLs;    ///< vector, which stores the cls method type to be plotted
    OptParser*  arg; ///< command line options
    TCanvas*    m_mainCanvas;
    TString     name;
    TString     title;
    TString     label;
    bool        plotLegend;
    bool        plotSolution;
    double      plotLogYMin;
    double      plotLogYMax;
};

#endif
