#include "ParameterEvolutionPlotter.h"

ParameterEvolutionPlotter::ParameterEvolutionPlotter(MethodProbScan *scanner)
{
    // copy over the command line arguments
    arg = scanner->getArg();

    // clone the workspace so we don't mess with the original one
    w = (RooWorkspace*)scanner->getWorkspace()->Clone();

    // copy over names
    title = scanner->getTitle();
    name = scanner->getName();
    parsName = scanner->getParsName();
    obsName = scanner->getObsName();
    scanVar1 = scanner->getScanVar1()->GetName();

    // copy over non-empty curve results
    for ( int i=0; i<scanner->getAllResults().size(); i++ ){
        if ( scanner->getAllResults()[i] ) allResults.push_back(scanner->getAllResults()[i]);
    }
    for ( int i=0; i<scanner->getCurveResults().size(); i++ ){
        if ( scanner->getCurveResults()[i] ) curveResults.push_back(scanner->getCurveResults()[i]);
    }

    // get the chi2 values at the local minima
    getLocalMinPositions();

    // canvas handling
    m_padId = 0;
}


ParameterEvolutionPlotter::~ParameterEvolutionPlotter()
{
    delete w;
}

///
/// Compute the positions of the local minima, in terms of scan steps, store it in
/// m_localMinPositions. This will then be used to plot a red line
/// at the position of the local minima.
///
void ParameterEvolutionPlotter::getLocalMinPositions()
{
    m_localMinPositions.clear();
    for ( int i=1; i<curveResults.size()-1; i++ ){
        if ( curveResults[i-1]==0 || curveResults[i]==0 ) continue;
        if ( curveResults[i-1]->minNll() > curveResults[i]->minNll()
          && curveResults[i]->minNll() < curveResults[i+1]->minNll() ){
            m_localMinPositions.push_back(curveResults[i]->getParVal(scanVar1));
        }
    }
}

void ParameterEvolutionPlotter::drawLinesAtMinima(TVirtualPad *pad)
{
    for ( int i=0; i<m_localMinPositions.size(); i++ ){
        drawVerticalRedLine(pad, m_localMinPositions[i]);
    }
}

///
/// Draw a vertical red line into the current pad at position i.
///
void ParameterEvolutionPlotter::drawVerticalRedLine(TVirtualPad *pad, float xpos)
{
    pad->cd();
    pad->Update();
    float ymin = pad->GetUymin();
    float ymax = pad->GetUymax();
    float xmin = pad->GetUxmin();
    float xmax = pad->GetUxmax();
    TLine* l1 = new TLine(xpos, ymin, xpos, ymax);
    l1->SetLineWidth(1);
    l1->SetLineColor(kRed);
    l1->SetLineStyle(kSolid);
    l1->Draw();
}

///
/// Make an evolution graph for one parameter.
///
TGraphErrors* ParameterEvolutionPlotter::makeEvolutionGraphErrors(vector<RooSlimFitResult*> results, TString parName)
{
    TGraphErrors *g = new TGraphErrors(results.size());
    int iGraph = 0;
    for ( int i=0; i<results.size(); i++ ){
        if ( results[i] ){
            g->SetPoint(iGraph, results[i]->getParVal(scanVar1), results[i]->getParVal(parName));
            g->SetPointError(iGraph, 0, results[i]->getParErr(parName));
            iGraph++;
        }
    }
    return g;
}

///
/// Make an evolution graph for one parameter.
///
TGraph* ParameterEvolutionPlotter::makeEvolutionGraph(vector<RooSlimFitResult*> results, TString parName)
{
    TGraph *g = new TGraph(results.size());
    int iGraph = 0;
    for ( int i=0; i<results.size(); i++ ){
        if ( results[i] ){
            g->SetPoint(iGraph, results[i]->getParVal(scanVar1), results[i]->getParVal(parName));
            iGraph++;
        }
    }
    return g;
}

///
/// Make a chi2 graph.
///
TGraph* ParameterEvolutionPlotter::makeChi2Graph(vector<RooSlimFitResult*> results)
{
    TGraph *g = new TGraph(results.size());
    int iGraph = 0;
    for ( int i=0; i<results.size(); i++ ){
        if ( results[i] ){
            g->SetPoint(iGraph, results[i]->getParVal(scanVar1), results[i]->minNll());
            iGraph++;
        }
    }
    return g;
}

///
/// Plot the evolution of best fit nuisance paramters
/// along the 1-CL curve.
///
/// By changing the code at the beginning of the function
/// one can chose whether all scan results are plotted, or
/// only those comprising the 1-CL curve.
///
void ParameterEvolutionPlotter::plotParEvolution()
{
    vector<RooSlimFitResult*> results = allResults;
    // vector<RooSlimFitResult*> results = curveResults;

    cout << "ParameterEvolutionPlotter::plotParEvolution() : plotting ..." << endl;
    selectNewCanvas(title+" 1");

    // get all parameters, loop over them
    TIterator* it = w->set(parsName)->createIterator();
    while ( RooRealVar* p = (RooRealVar*)it->Next() )
    {
        if ( p->isConstant() && p->GetName()!=scanVar1 ) continue;
        if ( arg->debug ) cout << "ParameterEvolutionPlotter::plotParEvolution() : var = " << p->GetName() << endl;
        TVirtualPad *pad = selectNewPad();
        pad->SetLeftMargin(0.25);
        pad->SetTopMargin(0.10);
        // create a graph of the nominal evolution of one parameter
        TGraph *g = makeEvolutionGraph(curveResults, p->GetName());
        g->SetTitle(p->GetTitle());
        g->GetXaxis()->SetTitle(scanVar1);
        g->GetXaxis()->SetTitleSize(0.08);
        g->GetXaxis()->SetLabelSize(0.06);
        g->GetXaxis()->SetNdivisions(-406);
        g->GetYaxis()->SetTitleSize(0.08);
        g->GetYaxis()->SetLabelSize(0.06);
        g->GetYaxis()->SetTitleOffset(1.4);
        g->GetYaxis()->SetTitle(p->GetName());
        g->SetLineColor(kBlue);
        g->SetLineWidth(2);
        TGaxis::SetMaxDigits(3); // forces scienfific notation
        g->Draw("al");
        // add error bands
        TGraphErrors *g2 = makeEvolutionGraphErrors(curveResults, p->GetName());
        g2->SetFillColorAlpha(kBlue,0.15);
        g2->Draw("3");
        // create a graph of the full evolution of one parameter
        if ( arg->isQuickhack(16) ){
            TGraph *g3 = makeEvolutionGraph(allResults, p->GetName());
            g3->Draw("p");
        }
        updateCurrentCanvas();
        // plot a red line at minimum
        drawLinesAtMinima(pad);
    }

    // plot the chi2 to the last pad
    TVirtualPad *pad = selectNewPad();
    pad->SetLeftMargin(0.25);
    TGraph *g = makeChi2Graph(curveResults);
    g->SetLineWidth(2);
    g->SetTitle("chi2");
    g->GetXaxis()->SetTitle(scanVar1);
    g->GetXaxis()->SetTitleSize(0.08);
    g->GetXaxis()->SetLabelSize(0.06);
    g->GetXaxis()->SetNdivisions(-406);
    g->GetYaxis()->SetTitleSize(0.08);
    g->GetYaxis()->SetLabelSize(0.06);
    g->GetYaxis()->SetTitleOffset(1.4);
    g->GetYaxis()->SetTitle("#chi^{2}");
    g->Draw("al");
    drawLinesAtMinima(pad);
    updateCurrentCanvas();
    // save plots
    saveEvolutionPlots();
}

///
/// Save all parameter evolution plots that were created so far.
///
void ParameterEvolutionPlotter::saveEvolutionPlots()
{
    for ( int i=0; i<m_canvases.size(); i++ ) {
        TString fName = "parEvolution_"+name+"_"+scanVar1;
        fName += Form("_%i",i+1);
        savePlot(m_canvases[i], fName);
    }
}

///
/// Plot the discrepancy between the observable and the predicted
/// observable when making predictions about observables by scanning
/// them. This checks if the chi2 term of the observable is tight enough.
/// This only works for 1D scans for now.
///
void ParameterEvolutionPlotter::plotObsScanCheck()
{
    vector<RooSlimFitResult*> results = curveResults;

    cout << "ParameterEvolutionPlotter::plotObsScanCheck() : plotting ..." << endl;
    TCanvas *c2 = newNoWarnTCanvas("plotObsScanCheck"+getUniqueRootName(), title, 800,600);
    c2->SetLeftMargin(0.2);

    // get observable
    TGraphErrors *g = new TGraphErrors(results.size());
    int iGraph = 0;

    for ( int i=0; i<results.size(); i++ ){
        assert(results[i]);
        // get value of observable
        float obsValue = results[i]->getParVal(scanVar1);
        float obsError = w->var(scanVar1)->getError();

        // get value of theory prediction
        setParameters(w,parsName,results[i]);
        TString thName = scanVar1;
        thName.ReplaceAll("_obs","_th");
        if ( !w->function(thName) ){
            cout << "ParameterEvolutionPlotter::plotObsScanCheck() : ERROR : theory value not found: " << thName << endl;
            continue;
        }
        float thValue = w->function(thName)->getVal();
        g->SetPoint(iGraph, iGraph, obsValue-thValue);
        g->SetPointError(iGraph, 0., obsError);
        iGraph++;
    }

    g->SetTitle(scanVar1);
    g->GetXaxis()->SetTitle("scan step");
    g->GetYaxis()->SetTitleSize(0.06);
    g->GetYaxis()->SetLabelSize(0.04);
    g->GetYaxis()->SetTitleOffset(1.5);
    g->GetYaxis()->SetTitle(scanVar1);
    Int_t ci = 927;
    TColor *col = new TColor(ci, 0, 0, 1, " ", 0.5);
    g->SetFillColor(ci);
    g->SetFillStyle(1001);
    g->Draw("a3");
    g->Draw("lxsame");
    c2->Update();

    savePlot(c2, "parEvolutionObsSanCheck_"+name+"_"+scanVar1);
}

///
/// Create a new canvas and add it to the list of canvases.
///
/// \return pointer to the new canvas. Caller assumes ownership
///
TCanvas* ParameterEvolutionPlotter::selectNewCanvas(TString title)
{
    title.ReplaceAll(name+" ","");
    TCanvas* c1 = newNoWarnTCanvas(getUniqueRootName(), name+" "+title, 1200, 900);
    c1->Divide(3,2);
    m_canvases.push_back(c1);
    m_padId = 0;
    return c1;
}


TVirtualPad* ParameterEvolutionPlotter::selectNewPad()
{
    TCanvas* c1 = m_canvases[m_canvases.size()-1];
    if ( m_padId>=6 ){
        // Create a new canvas that has the old title "foo 3" but with
        // incremented number: "foo 4". Only one-digit numbers are supported.
        TString title = c1->GetTitle();
        int oldNumber = TString(title[title.Sizeof()-2]).Atoi(); // get last character and turn into integer
        title.Replace(title.Sizeof()-2,1,Form("%i",++oldNumber)); // replace last character with incremented integer
        c1 = selectNewCanvas(title);
    }
    m_padId+=1;
    return c1->cd(m_padId);
}

///
/// Update the current control plot canvas.
///
void ParameterEvolutionPlotter::updateCurrentCanvas()
{
    TCanvas* c1 = m_canvases[m_canvases.size()-1];
    c1->Update();
}

