#include "PDF_DatasetOldFitting.h"
#include "RooExponential.h"
#include "RooMinimizer.h"
#include "TMath.h"

PDF_DatasetOldFitting::PDF_DatasetOldFitting(RooWorkspace* w): PDF_Datasets(w){};
PDF_DatasetOldFitting::PDF_DatasetOldFitting(RooWorkspace* w, int nObs, OptParser* opt): PDF_Datasets::PDF_Datasets(w,nObs,opt){};
PDF_DatasetOldFitting::~PDF_DatasetOldFitting(){};

RooFitResult* PDF_DatasetOldFitting::fit(RooDataSet* dataToFit) {

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

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);

    // Choose Dataset to fit to
    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdf->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));
    RooAbsReal* nll; 
    if(blindFlag){
       std::cout << "PDF_DatasetOldFitting::fit: Creating negative log likelihood with blinding" <<std::endl;
       nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)),RooFit::Range("lsb,rsb"));
    }
    else{
       nll = pdf->createNLL(*dataToFit, RooFit::Extended(kTRUE), RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)));
    }
    double nll_init_val = nll->getVal(nll->getVariables());
    RooFormulaVar* nll_toFit = new RooFormulaVar("nll_norm", ("@0-"+std::to_string(nll_init_val)).c_str(), RooArgList(*nll));
    std::cout << "Testing S+BG" <<std::endl;

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
        std::cout << i << " | Liquid: Fit Status=" << result->status() << " |  CovQual=" << result->covQual() <<std::endl;
    }while(!(result->status() == 0 && result->covQual()==3));

 
    this->fitStatus = result->status();
    this->minNll = nll->getVal();
    std::cout << "Wifi: Fit Status=" << this->fitStatus << ": Nll=" << this->minNll <<std::endl;

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);

    delete nll;

    return result;
};


RooFitResult* PDF_DatasetOldFitting::fitBkg(RooDataSet* dataToFit) {

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

    // Turn off RooMsg
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooMsgService::instance().setSilentMode(kTRUE);

    // unfortunately Minuit2 does not initialize the status of the roofitresult, if all parameters are constant. Therefore need to stay with standard Minuit fitting.
    // RooFitResult* result  = pdfBkg->fitTo( *dataToFit, RooFit::Save() , RooFit::ExternalConstraints(*this->getWorkspace()->set(constraintName)), RooFit::Minimizer("Minuit2", "Migrad"));
    
    RooAbsReal* nll_bkg; 
    if(blindFlag){
       std::cout << "PDF_DatasetOldFitting::fit: Creating negative log likelihood with blinding" <<std::endl;
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
        std::cout << i << " | Solid: Fit Status=" << result->status() << " |  CovQual=" << result->covQual() <<std::endl;
    }while(!(result->status() == 0 && result->covQual()==3));

 
    this->fitStatus = result->status();
    this->minNllBkg = nll_bkg->getVal();

    RooMsgService::instance().setSilentMode(kFALSE);
    RooMsgService::instance().setGlobalKillBelow(INFO);

    
    if(sanity){
        plotting((plotDir+"BackgroundFit").c_str(), dataToFit, counterBG, 0);
        counterBG=counterBG+1;
    }

    std::cout << "Intranet: Fit Status=" << this->fitStatus << ": Nll=" << this->minNllBkg <<std::endl;
    getWorkspace()->var("BFsig")->setVal(parvalue);
    getWorkspace()->var("BFsig")->setConstant(isconst);    

    delete nll_bkg;
    return result;
};

void   PDF_DatasetOldFitting::generateToys(int SeedShift) {

    initializeRandomGenerator(SeedShift);
    //RooDataSet* toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);
    //RooDataSet* toys = pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);
    RooDataSet* toys;
    if(isToyDataset){
        toys = this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)), wspc->data(dataName)->numEntries(), kFALSE, kFALSE, kFALSE);
    }
    else{
        toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);
    }

    if(sanity){
        plotting((plotDir+"ToyDataSet").c_str(), toys, counter, 1);
        counter=counter+1;
    }

    // Having the delete in here causes a segmentation fault, likely due to a double free
    // related to Root's internal memory management. Therefore we do not delete,
    // which might or might not cause a memory leak.
    // if(this->toyObservables) delete this->toyObservables;
    this->toyObservables  = toys;
    this->isToyDataSet    = kTRUE;
};

void PDF_DatasetOldFitting::generateBkgToys(int SeedShift) {

    initializeRandomGenerator(SeedShift);

    if(isBkgPdfSet){
        double parvalue = getWorkspace()->var("BFsig")->getVal();
        bool isconst = getWorkspace()->var("BFsig")->isConstant();
        getWorkspace()->var("BFsig")->setVal(0.0);
        getWorkspace()->var("BFsig")->setConstant(true);
        // RooDataSet* toys = pdfBkg->generate(*observables, RooFit::NumEvents(wspc->data(dataName)->numEntries()), RooFit::Extended(kTRUE));
        //RooDataSet* toys = pdf->generate(*observables, RooFit::NumEvents(wspc->data(dataName)->numEntries()), RooFit::Extended(kTRUE));
        RooDataSet* toys;
        if(isToyDataset){
            toys = this->pdf->generate(RooArgSet(*observables), *(RooDataSet*)(wspc->data(dataName)), wspc->data(dataName)->numEntries(), kFALSE, kFALSE, kFALSE);
        }
        else{
            toys = this->pdf->generate(RooArgSet(*observables), wspc->data(dataName)->numEntries(), kFALSE, kTRUE, "", kFALSE, kTRUE);
        }
	//RooDataSet* toys = this->pdf->generate(RooArgSet(wspc->var(massVarname.c_str()), wspc->cat("category")),0,kFALSE,kTRUE,"",kFALSE,kTRUE)
    
        if(sanity){
            plotting((plotDir+"ToyBGDataSet").c_str(), toys, counterToy, 1);
            counterToy=counterToy+1;
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

void PDF_DatasetOldFitting::plotting(std::string plotString, RooDataSet* data, int count, bool isToy){
    RooWorkspace* w = this->getWorkspace();

    if(isToy){
        TCanvas* canvas = new TCanvas("cToy","cToy",600,600);
        RooPlot* frame = w->var(massVarName.c_str())->frame();
        data->plotOn(frame);
        pdf->plotOn(frame,RooFit::ProjWData(*data), RooFit::Range("full"), RooFit::NormRange("full"));
        frame->Draw();
        plotString = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_Toy"+to_string(this->nToy)+"_JobID"+to_string(this->jobID)+"_"+this->jobType+".pdf";
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
            std::string plotOutput = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_Toy"+to_string(this->nToy)+"_JobID"+to_string(this->jobID)+"_"+cat+"_"+this->jobType+"_SBNorm.pdf";
            canvas->SaveAs((plotOutput).c_str());
            canvas->Close();
        }

        TCanvas* canvas2 = new TCanvas(Form("c%i",9),Form("c%i",9),600,600);
        RooPlot* frame2 = w->var(massVarName.c_str())->frame();
        data->plotOn(frame2);
        if(blindFlag){ pdf->plotOn(frame2, RooFit::Range("lsb,rsb"), RooFit::NormRange("lsb,rsb")); }
        else{ pdf->plotOn(frame2, RooFit::Range("full"), RooFit::NormRange("full")); }

        frame2->Draw();
        //std::string plotOutput = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_All_SBNorm.pdf";
        std::string plotOutput = plotString+"_"+to_string(count)+"_ScanPoint"+to_string(this->scanPoint)+"_Toy"+to_string(this->nToy)+"_JobID"+to_string(this->jobID)+"_All"+"_"+this->jobType+"_SBNorm.pdf";
        canvas2->SaveAs((plotOutput).c_str());
        canvas2->Close();
    }
};

void PDF_DatasetOldFitting::plotting(std::string plotString, RooAbsReal* nll, int count){
    RooWorkspace* w = this->getWorkspace();

    TCanvas* canvas = new TCanvas("cNll","cNll",600,600);
    RooPlot* frame = w->var(massVarName.c_str())->frame();
    nll->plotOn(frame);
    frame->Draw();
    plotString = plotString+"_"+to_string(count)+".pdf";
    canvas->SaveAs((plotString).c_str());
    canvas->Close();
};
