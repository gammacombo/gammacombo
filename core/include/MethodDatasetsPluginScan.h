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
#include "ProgressBar.h"
#include "PDF_Datasets.h"
#include "RooSlimFitResult.h"
#include "TLeaf.h"
#include "TBranch.h"

class MethodDatasetsPluginScan : public MethodPluginScan
{
public:
    MethodDatasetsPluginScan(PDF_Datasets* PDF, OptParser* opt);
    void                drawDebugPlots(int runMin, int runMax, TString fileNameBaseIn = "default");
    float               getParValAtIndex(int index, TString parName);
    MethodProbScan*     getProfileLH(){return this->profileLH;};
    virtual void        initScan();
    void                loadParameterLimits();
    inline  void        performProbScanOnly(bool yesNo=true){doProbScanOnly = yesNo;};
    void                performBootstrapTest(int nSamples=1000, const TString& ext ="");
    virtual void        print();
    void                printDebug(const RooFitResult& r);
    TChain*             readFiles(int runMin, int runMax, int &nFilesRead, int &nFilesMissing, TString fileNameBaseIn = "default");
    void                readScan1dTrees_prob();
    void                readScan1dTrees(int runMin, int runMax, TString fileNameBaseIn = "default");
    virtual int         scan1d(int nRun=1);
    inline  void        setInputFile(TString name){inputFiles.push_back(name); explicitInputFile=true;};
    inline  void        addFile(TString name){inputFiles.push_back(name);};

    PDF_Datasets*        pdf;
    TH1F*                   probPValues;
    TTree*                  probScanTree;
    bool                    drawPlots;
    bool                    explicitInputFile;
    bool                    doProbScanOnly; // for information on this switch, read the comments on top of the .cpp file.
    bool                    externalProfileLH;
    std::vector<TString>    inputFiles;
    std::vector<double>     bootstrapPVals;
    TChain*                 chain;
    RooFitResult*           dataFreeFitResult;

protected:
    RooSlimFitResult*   getParevolPoint(float scanpoint);
    void                setParevolPointByIndex(int index);

private:
    RooFitResult*       loadAndFit(bool fitToys, PDF_Datasets* pdf);
    void                setExtProfileLH(TTree* tree);
    void                scan1d_plugin(int nRun);
    void                scan1d_prob();
    double              getPValueTTestStatistic(double test_statistic_value);
    void                setAndPrintFitStatusConstrainedToys(const ToyTree& ToyTree);
    void                setAndPrintFitStatusFreeToys(const ToyTree& ToyTree);
};

#endif
