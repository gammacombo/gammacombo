/*
 * Gamma Combination
 * Date: October 2016
 */

#ifndef MethodDatasetsProbScan_h
#define MethodDatasetsProbScan_h

#include "MethodProbScan.h"
#include "ProgressBar.h"
#include "PDF_Datasets.h"
#include "RooSlimFitResult.h"
#include "TLeaf.h"
#include "TBranch.h"
#include "ToyTree.h"

class MethodDatasetsProbScan : public MethodProbScan
{
public:
    MethodDatasetsProbScan(PDF_Datasets* PDF, OptParser* opt);

    virtual void        initScan();
    void                loadScanFromFile(TString fileNameBaseIn = "default");
    void                loadParameterLimits();
    virtual void        print();
    virtual int         scan1d();
    inline  void        setInputFile(TString name){inputFiles.push_back(name); explicitInputFile=true;};
    inline  void        addFile(TString name){inputFiles.push_back(name);};

    PDF_Datasets*           pdf;
    TH1F*                   probPValues;
    bool                    drawPlots;
    bool                    explicitInputFile;
    std::vector<TString>    inputFiles;
    std::vector<double>     bootstrapPVals;
    TChain*                 chain;
    RooFitResult*           dataFreeFitResult;
    ToyTree*                probScanTree;

protected:

private:
    TChain*             readFiles(TString fileNameBaseIn = "default");
    void                readScan1dTrees(TString fileNameBaseIn = "default");
    RooFitResult*       loadAndFit(PDF_Datasets* pdf);
    double              getPValueTTestStatistic(double test_statistic_value);
    void                setAndPrintFitStatusConstrainedToys(const ToyTree& toyTree);
    void                setAndPrintFitStatusFreeToys(const ToyTree& toyTree);
    void                sethCLFromProbScanTree();
};

#endif
