#ifndef PDF_DatasetManual_h
#define PDF_DatasetManual_h

#include "PDF_Datasets.h"


class PDF_DatasetManual : public PDF_Datasets
{
    public:
        PDF_DatasetManual(RooWorkspace* w);
        PDF_DatasetManual(RooWorkspace* w, int nObs, OptParser* opt);
        RooFitResult* fit(RooDataSet* dataToFit);
        RooFitResult* fitBkg(RooDataSet* dataToFit);
        void  generateToys(int SeedShift = 0) override;
        void  generateBkgToys(int SeedShift = 0) override;
        void  plotting(std::string plotString, std::string plotSuffix, RooDataSet* data, int count=0, bool isToy=0);
        ~PDF_DatasetManual();

        std::string massVarName =""; 
        std::string plotDir ="plots/pdf/";
        bool sanity = false; 
        bool blindFlag = false;
        bool isToyDataset = false;

    private:
        int counterToy =0;
        int counterBGToy =0;
        int counterBG =0;
        int counterSB =0;
};

#endif
