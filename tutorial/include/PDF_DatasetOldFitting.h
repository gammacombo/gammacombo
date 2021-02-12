#ifndef PDF_DatasetOldFitting_h
#define PDF_DatasetOldFitting_h

#include "PDF_Datasets.h"


class PDF_DatasetOldFitting : public PDF_Datasets
{
    public:
        PDF_DatasetOldFitting(RooWorkspace* w);
        PDF_DatasetOldFitting(RooWorkspace* w, int nObs, OptParser* opt);
        RooFitResult* fit(RooDataSet* dataToFit);
        RooFitResult* fitBkg(RooDataSet* dataToFit);
        void  generateToys(int SeedShift = 0) override;
        void  generateBkgToys(int SeedShift = 0) override;
        void  plotting(std::string plotString, RooDataSet* data, int count=0, bool isToy=0);
        void  plotting(std::string plotString, RooAbsReal* nll, int count);
        ~PDF_DatasetOldFitting();

        std::string massVarName =""; 
        std::string plotDir ="plots/pdf/";
        bool sanity = false; 
        bool blindFlag = false;
        bool isToyDataset = false;
        double blindWindow[2] = {999,-999};

    private:
        int counter =0;
        int counterToy =0;
        int counterBG =0;
        int counterSB =0;
};

#endif
