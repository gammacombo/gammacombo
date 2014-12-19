#include "ParameterEvolutionPlotter.h"

ParameterEvolutionPlotter::ParameterEvolutionPlotter(MethodProbScan *scanner):
	allResults(scanner->getAllResults()), // provide a link to the scan curve results
	curveResults(scanner->getCurveResults())
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

	// get the chi2 values at the local minima
	localChi2 = getLocalMinimaY(scanner->getHchisq());
}


ParameterEvolutionPlotter::~ParameterEvolutionPlotter()
{
	delete w;
}


///
/// Find the Y coordinates of all local minima of a histogram.
/// We use it to quickly find the chi2 values at the local minima.
/// \return vector of the Y values of all local minima.
///
vector<float> ParameterEvolutionPlotter::getLocalMinimaY(TH1F* h)
{
	vector<float> yvalues;
	for ( int i=2; i<h->GetNbinsX()-1; i++ )
	{
		if ( h->GetBinContent(i-1) > h->GetBinContent(i)
		  && h->GetBinContent(i) < h->GetBinContent(i+1) )
		{
			yvalues.push_back(h->GetBinContent(i));
		}
	}
	return yvalues;
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
	// vector<RooSlimFitResult*> results = allResults;
	vector<RooSlimFitResult*> results = curveResults;

	cout << "ParameterEvolutionPlotter::plotParEvolution() : plotting ..." << endl;
	TCanvas *c2 = new TCanvas("plotParEvolution"+getUniqueRootName(), title, 2000,1600);
	c2->Divide(7,5);
	int iPad = 1;

	// get all parameters, loop over them
	TIterator* it = w->set(parsName)->createIterator();
	while ( RooRealVar* p = (RooRealVar*)it->Next() )
	{
		if ( p->isConstant() && p->GetName()!=scanVar1 ) continue;
		if ( arg->verbose ) cout << "ParameterEvolutionPlotter::plotParEvolution() : var = " << p->GetName() << endl;
		TGraph *g = new TGraph(results.size());
		int iGraph = 0;

		for ( int i=0; i<results.size(); i++ ){
			assert(results[i]);
			g->SetPoint(iGraph, iGraph, results[i]->getParVal(p->GetName()));
			iGraph++;
		}

		TPad *pad = (TPad*)c2->cd(iPad);
		pad->SetLeftMargin(0.25);
		g->SetTitle(p->GetName());
		g->GetXaxis()->SetTitle("scan step");
		g->GetYaxis()->SetTitleSize(0.09);
		g->GetYaxis()->SetLabelSize(0.07);
		g->GetYaxis()->SetTitleOffset(1.5);
		g->GetYaxis()->SetTitle(p->GetName());
		g->Draw("al");
		c2->Update();
		iPad += 1;
	}

	// plot the chi2 to the last pad
	TPad *pad = (TPad*)c2->cd(iPad++);
	pad->SetLeftMargin(0.25);
	TGraph *g = new TGraph(results.size()-1);
	int iGraph = 0;
	for ( int i=0; i<results.size(); i++ )
	{
		if ( !results[i] ) continue;
		g->SetPoint(iGraph, iGraph, results[i]->minNll());
		iGraph++;
	}
	g->SetTitle("chi2");
	g->GetXaxis()->SetTitle("scan step");
	g->GetYaxis()->SetTitleSize(0.09);
	g->GetYaxis()->SetLabelSize(0.07);
	g->GetYaxis()->SetTitleOffset(1.5);
	g->GetYaxis()->SetTitle("chi2");
	g->Draw("al");
	c2->Update();

	// print a red line at the position of the local solutions
	for ( int i=0; i<localChi2.size(); i++ )
	{
		iGraph = 0;
		for ( int j=0; j<results.size(); j++ )
		{
			if ( !results[j] ) continue;
			iGraph++;
			if ( !(fabs(results[j]->minNll() - localChi2[i])<0.001) ) continue;

			for ( int p=1; p<iPad; p++ )
			{
				TPad *pad = (TPad*)c2->cd(p);
				float ymin = pad->GetUymin();
				float ymax = pad->GetUymax();
				float xmin = pad->GetUxmin();
				float xmax = pad->GetUxmax();

				TLine* l1 = new TLine(iGraph, ymin, iGraph, ymax);
				l1->SetLineWidth(1);
				l1->SetLineColor(kRed);
				l1->SetLineStyle(kDashed);
				l1->Draw();
			}
		}
	}

	savePlot(c2, "parEvolution_"+name+"_"+scanVar1);
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
	TCanvas *c2 = new TCanvas("plotObsScanCheck"+getUniqueRootName(), title, 800,600);
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
