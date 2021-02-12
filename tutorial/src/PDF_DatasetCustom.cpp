#include "PDF_DatasetCustom.h"
#include "RooExponential.h"
#include "RooMinimizer.h"
#include "TMath.h"

PDF_DatasetCustom::PDF_DatasetCustom(RooWorkspace* w): PDF_Datasets(w){};
PDF_DatasetCustom::PDF_DatasetCustom(RooWorkspace* w, int nObs, OptParser* opt): PDF_Datasets::PDF_Datasets(w,nObs,opt){};
PDF_DatasetCustom::~PDF_DatasetCustom(){};

RooFitResult* PDF_DatasetCustom::fit(RooDataSet* dataToFit) {

    if (this->getWorkspace()->set(constraintName) == NULL) {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "ERROR: No RooArgSet with constraints found." << std::endl;
        std::cout << "The Workspace must contain a RooArgSet with constraint PDFs." << std::endl;
        std::cout << "These are usually Gaussians that constrain parameters via global observables." << std::endl;
        std::cout << "This set can be empty." << std::endl;
        std::cout << "By default its name should be 'default_internal_constraint_set_name'." << std::endl;
        std::cout << "Other names can be passed via PDF_Datasets::initConstraints" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);

    // Choose Dataset to fit to
    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));
    
    RooFitResult* result;
    int counter =0;
    int fitStrategy = 2;
    do{
        if(counter ==30) break;

        if(blindFlag){ result  = pdf->fitTo( *dataToFit, RooFit::Save()
                                             ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                             ,RooFit::Extended(kTRUE) 
                                             //,RooFit::Range("lsb,rsb")
                                             ,RooFit::Minos(true)
                                             ,RooFit::Strategy(fitStrategy)
        );}
        else{ result  = pdf->fitTo( *dataToFit, RooFit::Save()
                                   ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                   ,RooFit::Extended(kTRUE) 
                                   //,RooFit::Minimizer("Minuit", "migrad") 
                                   ,RooFit::Minos(true)
                                   ,RooFit::Strategy(fitStrategy)
        ); }

        std::cout << counter << " | PDF_DatasetCustom::Fit::Status = " << result->status() << " | covQual= " << result->covQual() <<std::endl;
        counter+=1;
    }while( !(result->status() == 0 && result->covQual()==3) );
 
    this->fitStatus = result->status();

    //Output FitResult to text file
    //std::string outstring = this->plotDir+Form("FitResultFor_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID)+"_"+this->jobType+".txt";
    std::string outstring = this->plotDir+"FitResult"+suffix+".txt";
    std::ofstream outfile(outstring, std::ofstream::out);
    result->printMultiline(outfile, 1111, kTRUE);
    
    RooAbsReal* nll; 
    if(blindFlag){ nll=pdf->createNLL(*dataToFit,RooFit::Extended(kTRUE),RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)),RooFit::Range("lsb,rsb"));}
    //if(blindFlag){ nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));}
    else{ nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));}

    this->minNll = nll->getVal();
    //std::cout << "S+BG: Fit Status=" << this->fitStatus << ": Nll=" << this->minNll <<std::endl;

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);

    if(sanity){
        plotting((plotDir+"SignalBGFit").c_str(), suffix, dataToFit, counterSB, 0);
        counterSB+=1;
    }
    delete nll;

    return result;
};


RooFitResult* PDF_DatasetCustom::fitBkg(RooDataSet* dataToFit) {

    if (this->getWorkspace()->set(constraintName) == NULL) {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "ERROR: No RooArgSet with constraints found." << std::endl;
        std::cout << "The Workspace must contain a RooArgSet with constraint PDFs." << std::endl;
        std::cout << "These are usually Gaussians that constrain parameters via global observables." << std::endl;
        std::cout << "This set can be empty." << std::endl;
        std::cout << "By default its name should be 'default_internal_constraint_set_name'." << std::endl;
        std::cout << "Other names can be passed via PDF_Datasets::initConstraints" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!pdfBkg){
        std::cout << "ERROR in PDF_Datasets::fitBkg -- No background PDF given!" << std::endl;
        exit(EXIT_FAILURE);
    }
    double parvalue = getWorkspace()->var("BFsig")->getVal();
    bool isconst = getWorkspace()->var("BFsig")->isConstant();
    getWorkspace()->var("BFsig")->setVal(0.0);
    getWorkspace()->var("BFsig")->setConstant(true);

    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);

    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdfBkg->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));
    
    RooFitResult* result;
    int counter =0;
    int fitStrategy = 2;
    do{
        if(counter ==30) break;

        if(blindFlag){
           std::cout << "PDF_DatasetCustom::fitBkg: Fitting with blinding" <<std::endl;
           result = pdf->fitTo( *dataToFit, RooFit::Save() 
                                ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                ,RooFit::Extended(kTRUE)
                                //,RooFit::Range("lsb,rsb")
                                ,RooFit::Minos(true)
                                ,RooFit::Strategy(fitStrategy)
                                );
        }
        else{
            result  = pdf->fitTo( *dataToFit, RooFit::Save() 
                                ,RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))
                                ,RooFit::Extended(kTRUE)
                                //,RooFit::Minimizer("Minuit", "minimize") //Used in Test 2
                                //,RooFit::Minimizer("Minuit", "migrad") //Used in Test 3
                                ,RooFit::Minos(true)
                                ,RooFit::Strategy(fitStrategy)
                                );
        }
        std::cout << counter << " | PDF_DatasetCustom::FitBkg::Status = " << result->status() << " | covQual= " << result->covQual() <<std::endl;
        counter+=1;
    }while( !(result->status() == 0 && result->covQual()==3) );

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);
    this->fitStatus = result->status();

    //Output FitResult to text file
    //std::string outstring = this->plotDir+Form("BGFitResultFor_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID)+"_"+this->jobType+".txt";
    std::string outstring = this->plotDir+"BGFitResult"+suffix+".txt";
    std::ofstream outfile(outstring, std::ofstream::out);
    std::ofstream outfileBG(outstring, std::ofstream::out);
    result->printMultiline(outfileBG, 1111, kTRUE);
    

    RooAbsReal* nll_bkg; 
    if(blindFlag){ 
        nll_bkg=pdf->createNLL(*dataToFit,RooFit::Extended(kTRUE),RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)),RooFit::Range("lsb,rsb"));
       //nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    }
    else{ nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName))); }

    this->minNllBkg = nll_bkg->getVal();
    //std::cout << "BG: Fit Status=" << this->fitStatus << ": Nll=" << this->minNllBkg <<std::endl;

    if(sanity){
        plotting((plotDir+"BackgroundFit").c_str(), suffix, dataToFit, counterBG, 0);
        counterBG+=1;
    }
    getWorkspace()->var("BFsig")->setVal(parvalue);
    getWorkspace()->var("BFsig")->setConstant(isconst);    
    delete nll_bkg;

    return result;
};

void   PDF_DatasetCustom::generateToys(int SeedShift) {
    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    initializeRandomGenerator(SeedShift);
    RooDataSet* toys;
    if(isToyDataset){ toys=this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)), wspc->data(dataName)->numEntries(), kFALSE, kFALSE, kFALSE);}
    else{ toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE); }

    if(sanity){
        plotting((plotDir+"ToyDataSet").c_str(), suffix, toys, counterToy, 1);
        counterToy+=1;
    }

    // Having the delete in here causes a segmentation fault, likely due to a double free
    // related to Root's internal memory management. Therefore we do not delete,
    // which might or might not cause a memory leak.
    // if(this->toyObservables) delete this->toyObservables;
    this->toyObservables  = toys;
    this->isToyDataSet    = kTRUE;
};

void PDF_DatasetCustom::generateBkgToys(int SeedShift) {
    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    initializeRandomGenerator(SeedShift);

    if(isBkgPdfSet){
        double parvalue = getWorkspace()->var("BFsig")->getVal();
        bool isconst = getWorkspace()->var("BFsig")->isConstant();
        getWorkspace()->var("BFsig")->setVal(0.0);
        getWorkspace()->var("BFsig")->setConstant(true);

        RooDataSet* toys;
        //TODO: Try with pdfBkg.
        if(isToyDataset){ toys=this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)),wspc->data(dataName)->numEntries(),kFALSE,kFALSE,kFALSE);}
        else{ toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);}
    
        if(sanity){
            plotting((plotDir+"ToyBGDataSet").c_str(), suffix, toys, counterBGToy, 1);
            counterBGToy+=1;
        }

        getWorkspace()->var("BFsig")->setVal(parvalue);
        getWorkspace()->var("BFsig")->setConstant(isconst);    
        this->toyBkgObservables  = toys;
    }
    else{
        std::cerr << "Error in PDF_Datasets::generateBkgToys: No bkg pdf given." << std::endl;
        exit(EXIT_FAILURE);
    }
};

void PDF_DatasetCustom::plotting(std::string plotString, std::string plotSuffix, RooDataSet* data, int count, bool isToy){
    RooWorkspace* w = this->getWorkspace();

    if(isToy){
        TCanvas* canvas = new TCanvas("cToy","cToy",600,600);
        RooPlot* frame = w->var(massVarName.c_str())->frame();
        data->plotOn(frame);
        pdf->plotOn(frame,RooFit::ProjWData(*data), RooFit::Range("full"), RooFit::NormRange("full"));
        frame->Draw();
        plotString = plotString+"_"+to_string(count)+plotSuffix+".pdf";
        canvas->SaveAs((plotString).c_str());
        canvas->Close();
    }
    else{
        std::string categories[8] = {"Brem_DD_Run1","Brem_LL_Run1","NoBrem_DD_Run1","NoBrem_LL_Run1","Brem_DD_Run2","Brem_LL_Run2","NoBrem_DD_Run2","NoBrem_LL_Run2"};

        for(int i=0; i<8; i++){
            std::string cat = categories[i];
            TCanvas* canvas = new TCanvas(Form("c%i",i),Form("c%i",i),600,600);
            RooPlot* frame = w->var(massVarName.c_str())->frame();
            std::string cutting = "category==category::"+cat;
            data->plotOn(frame,RooFit::Cut(cutting.c_str()));
            RooCategory* slicedCategory = w->cat("category");
            if(blindFlag){ pdf->plotOn(frame,RooFit::Slice(*slicedCategory, cat.c_str()), RooFit::ProjWData(*data), RooFit::Range("lsb,rsb"),RooFit::NormRange("lsb,rsb"));}
            else{ pdf->plotOn(frame,RooFit::Slice(*slicedCategory, cat.c_str()), RooFit::ProjWData(*data), RooFit::Range("full"),RooFit::NormRange("full"));}

            frame->Draw();
            std::string plotOutput = plotString+"_"+to_string(count)+plotSuffix+"_"+cat+"_"+this->jobType+"_SBNorm.pdf";
            canvas->SaveAs((plotOutput).c_str());
            canvas->Close();
        }

        TCanvas* canvas2 = new TCanvas("c9","c9",600,600);
        RooPlot* frame2 = w->var(massVarName.c_str())->frame();
        data->plotOn(frame2);
        if(blindFlag){ pdf->plotOn(frame2, RooFit::Range("lsb,rsb"), RooFit::NormRange("lsb,rsb")); }
        else{ pdf->plotOn(frame2, RooFit::Range("full"), RooFit::NormRange("full")); }

        frame2->Draw();
        std::string plotOutput = plotString+"_"+to_string(count)+plotSuffix+"_All_"+this->jobType+"_SBNorm.pdf";
        canvas2->SaveAs((plotOutput).c_str());
        canvas2->Close();
    }
};

