/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maximilian.schlupp@cern.ch
 * Date: January 2013 
 */

#ifndef MethodBergerBoosScan_h
#define MethodBergerBoosScan_h

#include "MethodPluginScan.h"
#include "TLeaf.h"

class MethodBergerBoosScan : public MethodPluginScan
{
public:
    MethodBergerBoosScan(MethodProbScan* s, TString d="XX");
    ~MethodBergerBoosScan();
    TH2F*           calcPValues(TH2F better, TH2F all, TH2F bg);
    void            getBestPValue(TH1F* hCL,TH2F* pValues); 
    int             getNBergerBoosPointsPerScanpoint(){return nBBPoints;}; ///< Return number of BB points per scan point
    void            readScan1dTrees(int runMin=1, int runMax=1); 
    int             scan1d(int nRun=1); 
    inline void     setNBergerBoosPointsPerScanpoint(int n){nBBPoints=n;}; ///< Set number of BB points per scan point
                                            ///< Set number of Berger Boos points drawn from 
                                            ///< the BergerBoos CL intervals defined in the workspace
    void            setNewBergerBoosPoint(int m);  ///< Samples one Berger Boos point
                                        ///< Draws ALL parameters randomly according to their 
                                        ///< BergerBoos ranges defined in the workspace
                                        ///< The parameter of interest (scan parameter) has to be set 
                                        ///< constant (and to its scan point value) after the function call.
    void            drawBBPoints(TString varX, TString varY, int runMin=1, int runMax=1, bool save=true); ///< Draws 2D Histogram showing the BB points in varX-varY space
                                        ///< The boolian 'save' specifies if a copy of the plot will
                                        ///< be saved in the plots folder. 
    TFile* file;
    TTree* BBtree;
    TString dir;
    
protected:

    int             nBBPoints;          ///< number of sampled Berger Boos points per scan point
};

#endif
