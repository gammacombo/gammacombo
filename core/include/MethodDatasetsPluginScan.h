/*
 * Gamma Combination
 * Author: Maximilian Schlupp, maxschlupp@gmail.com
 * Author: Konstantin Schubert, schubert.konstantin@gmail.com
 * Date: October 2016
 *
 */

#ifndef MethodDatasetsPluginScan_h
#define MethodDatasetsPluginScan_h

#include "MethodPluginScan.h"
#include "MethodDatasetsProbScan.h"
#include "ProgressBar.h"
#include "PDF_Datasets.h"
#include "RooSlimFitResult.h"
#include "TLeaf.h"
#include "TBranch.h"

class MethodDatasetsPluginScan : public MethodPluginScan
{
public:
    MethodDatasetsPluginScan(MethodProbScan* probScan, PDF_Datasets* PDF, OptParser* opt);
    void                drawDebugPlots(int runMin, int runMax, TString fileNameBaseIn = "default");
    float               getParValAtIndex(int index, TString parName);
    MethodDatasetsProbScan*     getProfileLH() {return dynamic_cast<MethodDatasetsProbScan*>(this->profileLH);};
    virtual void        initScan();
    void                loadParameterLimits();
    void                performBootstrapTest(int nSamples = 1000, const TString& ext = "");
    virtual void        print();
    void                printDebug(const RooFitResult& r);
    TChain*             readFiles(int runMin, int runMax, int &nFilesRead, int &nFilesMissing, TString fileNameBaseIn = "default");
    virtual void        readScan1dTrees(int runMin, int runMax, TString fileNameBaseIn = "default");
    virtual int         scan1d(int nRun = 1);
    inline  void        setInputFile(TString name) {inputFiles.push_back(name); explicitInputFile = true;};
    inline  void        addFile(TString name) {inputFiles.push_back(name);};

    PDF_Datasets*           pdf;
    bool                    explicitInputFile;
    std::vector<TString>    inputFiles;
    std::vector<double>     bootstrapPVals;
    TChain*                 chain;
    // RooFitResult*           dataFreeFitResult;
    RooFitResult*           dataBkgFitResult;

protected:
    RooSlimFitResult*   getParevolPoint(float scanpoint);
    void                setParevolPointByIndex(int index);
    double              bestfitpoint;

private:
    RooFitResult*       loadAndFit(PDF_Datasets* pdf); // in this Plugin class, this fits to toy!!
    RooFitResult*       loadAndFitBkg(PDF_Datasets* pdf); // in this Plugin class, this fits to bkg-only toy!!
    double              getPValueTTestStatistic(double test_statistic_value);
    void                setAndPrintFitStatusConstrainedToys(const ToyTree& ToyTree);
    void                setAndPrintFitStatusFreeToys(const ToyTree& ToyTree);
    void                checkExtProfileLH();
    void                makeControlPlots( std::map<int, std::vector<double> > bVals, std::map<int, std::vector<double> > sbVals );
    void                makeControlPlotsBias( std::map<int, std::vector<double> > biasVals);
};

#endif
