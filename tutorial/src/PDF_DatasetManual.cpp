#include "PDF_DatasetManual.h"
#include "RooExponential.h"
#include "RooMinimizer.h"
#include "TMath.h"

PDF_DatasetManual::PDF_DatasetManual(RooWorkspace* w): PDF_Datasets(w){};
PDF_DatasetManual::PDF_DatasetManual(RooWorkspace* w, int nObs, OptParser* opt): PDF_Datasets::PDF_Datasets(w,nObs,opt){};
PDF_DatasetManual::~PDF_DatasetManual(){};

RooFitResult* PDF_DatasetManual::fit(RooDataSet* dataToFit) {

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

    RooAbsReal* nll; 
    if(blindFlag){ nll=pdf->createNLL(*dataToFit,RooFit::Extended(kTRUE),RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)),RooFit::Range("lsb,rsb"));}
    else{ nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));}

    double nll_init_val = nll->getVal(nll->getVariables());
    RooFormulaVar* nll_toFit = new RooFormulaVar("nll_norm", ("@0-"+std::to_string(nll_init_val)).c_str(), RooArgList(*nll));

    RooFitResult* result;
    //RooMinuit* min = new RooMinuit(*nll);
    //RooMinimizer* min = new RooMinimizer(*nll);
    RooMinimizer* min = new RooMinimizer(*nll_toFit);
    min->setStrategy(2);
    int i = 0;

    //min->simplex();
    do{
        min->migrad();
        min->hesse();
        min->minos();
        result = min->save();
        i=i+1;
        
        if (i>30){ break;}
        std::cout << i << " | PDF_DatasetManual::Fit: Status=" << result->status() << " |  CovQual=" << result->covQual() <<std::endl;
    }while(!(result->status() == 0 && result->covQual()==3));

 
    this->fitStatus = result->status();
    this->minNll = nll->getVal();

    if(sanity){
        plotting((plotDir+"SignalBGDataSet").c_str(), suffix, dataToFit , counterSB, 1);
        counterBGToy+=1;
    }

    //std::cout << "Sig+BG: Fit Status=" << this->fitStatus << ": Nll=" << this->minNllBkg <<std::endl;
    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);

    delete nll;

    return result;
};


RooFitResult* PDF_DatasetManual::fitBkg(RooDataSet* dataToFit) {

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

    if (!pdfBkg)
    {
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
    
    RooAbsReal* nll_bkg; 
    if(blindFlag){
       std::cout << "PDF_DatasetManual::fit: Creating negative log likelihood with blinding" <<std::endl;
       nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)),RooFit::Range("lsb,rsb"));
    }
    else{
       nll_bkg = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    }
    double nll_init_valbg = nll_bkg->getVal(nll_bkg->getVariables());
    RooFormulaVar* nllbg_toFit = new RooFormulaVar("nllbg_norm", ("@0-"+std::to_string(nll_init_valbg)).c_str(), RooArgList(*nll_bkg));

    RooFitResult* result;
    //RooMinuit* min = new RooMinuit(*nll_bkg);
    //RooMinimizer* min = new RooMinimizer(*nll_bkg);
    RooMinimizer* min = new RooMinimizer(*nllbg_toFit);
    min->setStrategy(2);
    int i = 0;

    //min->simplex();
    do{
        min->migrad();
        min->hesse();
        min->minos();
        result = min->save();
        i=i+1;
        
        if (i>30){ break;}
        std::cout << i << " | PDF_DatasetManual::FitBG: Status=" << result->status() << " |  CovQual=" << result->covQual() <<std::endl;
    }while(!(result->status() == 0 && result->covQual()==3));

 
    this->fitStatus = result->status();
    this->minNllBkg = nll_bkg->getVal();

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);

    
    if(sanity){
        plotting((plotDir+"BackgroundFit").c_str(), suffix, dataToFit, counterBG, 0);
        counterBG+=1;
    }

    //std::cout << "BG: Fit Status=" << this->fitStatus << ": Nll=" << this->minNllBkg <<std::endl;
    getWorkspace()->var("BFsig")->setVal(parvalue);
    getWorkspace()->var("BFsig")->setConstant(isconst);    

    delete nll_bkg;
    return result;
};

void   PDF_DatasetManual::generateToys(int SeedShift) {
    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    initializeRandomGenerator(SeedShift);

    RooDataSet* toys;
    if(isToyDataset){ toys = this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)), wspc->data(dataName)->numEntries(),kFALSE,kFALSE,kFALSE);}
    else{ toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);}

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

void PDF_DatasetManual::generateBkgToys(int SeedShift) {
    std::string suffix = Form("_Scanpoint%i_Toy%i_JobID%i",this->scanPoint,this->nToy,this->jobID);

    initializeRandomGenerator(SeedShift);

    if(isBkgPdfSet){
        double parvalue = getWorkspace()->var("BFsig")->getVal();
        bool isconst = getWorkspace()->var("BFsig")->isConstant();
        getWorkspace()->var("BFsig")->setVal(0.0);
        getWorkspace()->var("BFsig")->setConstant(true);

        RooDataSet* toys;
        if(isToyDataset){ toys=this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)), wspc->data(dataName)->numEntries(),kFALSE,kFALSE,kFALSE);}
        else{ toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);    }
    
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

void PDF_DatasetManual::plotting(std::string plotString, std::string plotSuffix, RooDataSet* data, int count, bool isToy){
    RooWorkspace* w = this->getWorkspace();

    if(isToy){
        TCanvas* canvas = new TCanvas("cToy","cToy",600,600);
        RooPlot* frame = w->var(massVarName.c_str())->frame();
        data->plotOn(frame);
        pdf->plotOn(frame,RooFit::ProjWData(*data), RooFit::Range("full"), RooFit::NormRange("full"));
        frame->Draw();
        plotString = plotString+"_"+to_string(count)+plotSuffix+"_"+this->jobType+".pdf";
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
            //std::string plotOutput = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_"+cat+"_SBNorm.pdf";
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
        //std::string plotOutput = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_All_SBNorm.pdf";
        std::string plotOutput = plotString+"_"+to_string(count)+plotSuffix+"_All"+"_"+this->jobType+"_SBNorm.pdf";
        canvas2->SaveAs((plotOutput).c_str());
        canvas2->Close();
    }
};

