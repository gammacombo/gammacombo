#include "PullPlotter.h"
#include "TLatex.h"
#include "TFitResult.h"

PullPlotter::PullPlotter(MethodAbsScan *cmb)
{
	assert(cmb);
	this->cmb = cmb;
	this->arg = cmb->getArg();
	nSolution = 0;
	defineOrder();
}


PullPlotter::~PullPlotter()
{}


void PullPlotter::loadParsFromSolution(int n)
{
	cmb->loadSolution(n);
	nSolution = n;
}


void PullPlotter::defineOrder()
{
	obsOrder.push_back("          deltaAcp_obs");
	obsOrder.push_back("                B4_obs");
	obsOrder.push_back("           dD_k3pi_obs");
	obsOrder.push_back("           kD_k3pi_obs");
	obsOrder.push_back("                B3_obs");
	obsOrder.push_back("                B2_obs");
	obsOrder.push_back("            dD_kpi_obs");
	obsOrder.push_back("                B1_obs");
	obsOrder.push_back("                yD_obs");
	obsOrder.push_back("                xD_obs");
	obsOrder.push_back("             yp_dk_obs");
	obsOrder.push_back("             xp_dk_obs");
	obsOrder.push_back("             ym_dk_obs");
	obsOrder.push_back("             xm_dk_obs");
	obsOrder.push_back("       rp_dpi_k3pi_obs");
	obsOrder.push_back("        rp_dk_k3pi_obs");
	obsOrder.push_back("       rm_dpi_k3pi_obs");
	obsOrder.push_back("        rm_dk_k3pi_obs");
	obsOrder.push_back("          rkp_k3pi_obs");
	obsOrder.push_back("     afav_dpi_k3pi_obs");
	obsOrder.push_back("      afav_dk_k3pi_obs");
	obsOrder.push_back("        rp_dpi_kpi_obs");
	obsOrder.push_back("         rp_dk_kpi_obs");
	obsOrder.push_back("        rm_dpi_kpi_obs");
	obsOrder.push_back("         rm_dk_kpi_obs");
	obsOrder.push_back("      rkp_pipi_kpi_obs");
	obsOrder.push_back("           rkp_kpi_obs");
	obsOrder.push_back("        rkp_kk_kpi_obs");
	obsOrder.push_back("      afav_dpi_kpi_obs");
	obsOrder.push_back("       afav_dk_kpi_obs");
	obsOrder.push_back("  acp_dpi_pipi_kpi_obs");
	obsOrder.push_back("    acp_dpi_kk_kpi_obs");
	obsOrder.push_back("   acp_dk_pipi_kpi_obs");
	obsOrder.push_back("     acp_dk_kk_kpi_obs");
	obsOrder.push_back("                B5_obs");
	obsOrder.push_back("                B6_obs");
	obsOrder.push_back("               dD_k2pi");
	obsOrder.push_back("           dD_k2pi_obs");
	obsOrder.push_back("               kD_k2pi");
	obsOrder.push_back("           kD_k2pi_obs");
	obsOrder.push_back("               rD_k2pi");
	obsOrder.push_back("            rD_kpi_obs");
	obsOrder.push_back("           acpp_dk_obs");
	obsOrder.push_back("           rcpp_dk_obs");
	obsOrder.push_back("       rads_dk_kpi_obs");
	obsOrder.push_back("       aads_dk_kpi_obs");
	for ( int i=0; i<obsOrder.size(); i++ ) obsOrder[i].ReplaceAll(" ","");
}

///
/// Plot up to 10 pulls into one canvas. If less pulls are plotted,
/// the canvas is scaled smaller. This is to be called from plotPulls().
///
/// \param observables - vector of observable names, plot pulls for these observables
/// \param currentid - Define the current index for the "(3 of 7)" statement
/// \param maxid - Define the max index for the "(3 of 7)" statement
/// \param nObs - total number of observables
///
void PullPlotter::plotPullsCanvas(vector<TString>& observables, int currentid, int maxid, int nObs)
{
	// pull bar
	float width = 0.45;    // width of pull bar
	float spacing = 0.1;  // space between two lines
	// the canvas
	float pullRange = 3.;
	float xminCoords = -6.;
	float xmaxCoords = 6.;
	float yminCoords = 0.;
	float ymaxCoords = (width+spacing) * (observables.size() + 3.);
	// pull axis
	float xminPullAxis = -pullRange;
	float xmaxPullAxis = +pullRange;
	TCanvas *cPulls = newNoWarnTCanvas("cPulls"+getUniqueRootName(), cmb->getTitle(), 0, 0, 600, 40*observables.size()+120);
	cPulls->Range(xminCoords,yminCoords,xmaxCoords,ymaxCoords);

	// compute (5 of 7) string
	TString counter = "";
	if ( maxid>1 ) counter = Form(" (%i of %i)", currentid, maxid);

	// draw combiner title
	TPaveText *t4 = new TPaveText(xminCoords, ymaxCoords-width, xmaxCoords, ymaxCoords, "br");
	t4->SetBorderSize(0);
	t4->SetFillStyle(0);
	t4->SetTextAlign(22);
	t4->SetTextFont(133);
	t4->AddText(cmb->getTitle()+counter)->SetTextSize(25);
	t4->Draw();

	// add an horizontal pull axis
	TGaxis *axis = new TGaxis(xminPullAxis,yminCoords+2*width+spacing,xmaxPullAxis,yminCoords+2*width+spacing,xminPullAxis,xmaxPullAxis,510,"");
	axis->SetLabelFont(133);
	axis->SetLabelSize(18);
	axis->Draw();

	// draw observables, pulls, and migration
	for ( int iObs=0; iObs<observables.size(); iObs++ ){
		RooRealVar* pObs = (RooRealVar*)cmb->getObservables()->find(observables[iObs]);

		// find associated theory value
		TString pThName = pObs->GetName();
		pThName.ReplaceAll("obs", "th");
		RooRealVar* pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		if ( !pTh ){
			pThName = pObs->GetName();
			pThName.ReplaceAll("_obs", "");
			pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		}

		// compute pull
		float pull = (pTh->getVal()-pObs->getVal())/pObs->getError();

		// compute coordinates needed to plot a line in the pull graph
		float xmin = pull<0 ? pull : 0;
		float xmax = pull<0 ? 0 : pull;
		float ymax = ymaxCoords - (float)(iObs+1)*(width+spacing);
		float ymin = ymax - width;

		// draw the pull bar
		TBox *b = new TBox(xmin,ymin,xmax,ymax);
		b->SetFillColor(kBlue);
		b->Draw();

		// draw the observable name
		TString label = pObs->GetTitle();
		// TString label = pObs->GetName();
		label.ReplaceAll(" obs", "");
		TPaveText *t1 = new TPaveText(xminCoords,ymin,xminPullAxis,ymax, "br");
		t1->AddText(label);
		t1->SetTextAlign(32);
		t1->SetTextFont(133);
		t1->SetTextSize(15);
		t1->SetShadowColor(kWhite);
		t1->SetFillColor(kAzure-9);
		t1->SetLineColor(kWhite);
		t1->Draw();

		// compute precision for subdigits
		float pObsVal = pObs->getVal();
		float pThVal = pTh->getVal();
		if ( isAngle(pObs) ){
			pObsVal = RadToDeg(pObsVal);
			pThVal = RadToDeg(pThVal);
		}
		int p = TMath::Max(calcNsubdigits(pThVal, 3), calcNsubdigits(pObsVal, 3));
		if ( arg->digits>0 ) p = arg->digits;

		// draw the observable migration
		TPaveText *t2 = new TPaveText(xmaxPullAxis,ymin,xmaxCoords,ymax, "br");
		// t2->AddText(Form("%s: %.*f #rightarrow %.*f", label.Data(), p, pObsVal, p, pThVal));
		t2->AddText(Form("%.*f #rightarrow %.*f", p, pObsVal, p, pThVal));
		t2->SetTextAlign(12);
		t2->SetTextFont(133);
		t2->SetTextSize(16);
		t2->SetShadowColor(kWhite);
		t2->SetFillColor(kAzure-9);
		t2->SetLineColor(kWhite);
		t2->Draw();
	}

	// write chi2/ndof into the last plot
	if ( currentid==maxid ){
		float chi2 = cmb->getSolution(nSolution)->minNll();
		int nPar = cmb->getSolution(nSolution)->floatParsFinal().getSize();
		TPaveText *t3 = new TPaveText(xminCoords, yminCoords, xmaxCoords, yminCoords+width, "br");
		t3->SetBorderSize(0);
		t3->SetFillStyle(0);
		t3->SetTextAlign(12);
		t3->SetTextFont(133);
		t3->AddText(Form("#chi^{2}/(nObs-nPar) = %.2f/(%i-%i), P(#chi^{2},%i)=%.3f",
					chi2, nObs, nPar, nObs-nPar, TMath::Prob(chi2, nObs-nPar)))->SetTextSize(25);
		t3->Draw();
	}
	savePlot(cPulls, Form("pulls_"+cmb->getName()+"_sol%i_%iof%i", nSolution,currentid,maxid));
}

///
/// Save the pulls per combination pdf into a file so that we can somewhere else compute
/// the chi2 that each pdf contributes to the total combination
///
void PullPlotter::savePulls()
{
	cout << "saving pulls" << endl;

	ofstream outfile;
	outfile.open("plots/par/pulls_"+cmb->getName()+".dat");
	outfile << "# ObsName  ObsVal  ObsErr  ThVal  Pull  Chi2" << endl;

	double running_chi2 = 0;

	Combiner* combiner = cmb->getCombiner();
	vector<PDF_Abs*>& pdfs = combiner->getPdfs();
	for ( const auto& pdf: pdfs ){
		double pdf_chi2 = 0;
		//cout << pdf->getName() << endl;
		outfile << "pulls: " << pdf->getName() << endl;
		const RooArgList* obs = pdf->getObservables();
		// combiner holds the parameters so get them and set them on each pdf
		const RooArgSet* parsMin = combiner->getParameters();
		RooArgList* pars = pdf->getParameters();
		TIterator *it1 = pars->createIterator();
		while ( RooRealVar* pPar = (RooRealVar*)it1->Next() ){
			pPar->setVal( ((RooAbsReal*)parsMin->find(pPar->GetName()))->getVal() );
		}
		//
		//const RooArgList* th  = pdf->getTheory();
		//RooAbsPdf *probDist = pdf->getPdf();
		RooFormulaVar ll("ll","ll","-2*log(@0)", RooArgSet(*pdf->getPdf()));
		double chi2_contrib = ll.getVal();
		running_chi2 += chi2_contrib;
		// loop observables
		TIterator *it = obs->createIterator();
		while ( RooRealVar* pObs = (RooRealVar*)it->Next() ){
			// find associated theory value
			TString pThName = pObs->GetName();
			pThName.ReplaceAll("obs", "th");
			//RooRealVar* pTh = (RooRealVar*)th->find(pThName);
			RooRealVar* pTh = (RooRealVar*)cmb->getTheory()->find(pThName);

			double pull = (pTh->getVal() - pObs->getVal()) / pObs->getError();
			string name = string(pObs->GetName()).substr(0, string(pObs->GetName()).find(string("_obs")));

			outfile << "  " << name << " " << pObs->getVal() << " " << pObs->getError() << " " << pTh->getVal() << " " << pull << " " << pull*pull << endl;

			//printf("%f %f %f %f", pTh->getVal(), pObs->getVal(), pObs->getError(), pull);
		}
		outfile << "pdf chi2: " << chi2_contrib << endl;
	}
	outfile << "total chi2: " << running_chi2 << endl;

	outfile.close();

}

///
/// Make a pull plot of observables corresponding
/// to the given solution. Also compute the fit probability
/// using chi2, ndof and the prob function.
///
void PullPlotter::plotPulls()
{
	if ( arg->debug ) cout << "PullPlotter::plotPulls() : ";
	cout << "making pull plot  (" << cmb->getTitle() << ", solution " << nSolution << ") ..." << endl;

	// a histogram to store all the pulls
	TH1F *hPulls = new TH1F("hPulls","hPulls",50,-5,5);

	// add any observables that are not in the ordered list defined above,
	// where the order of certain observables is defined manually
	TIterator* it = cmb->getObservables()->createIterator();
	while ( RooRealVar* pObs = (RooRealVar*)it->Next() ){
		bool found=false;
		for ( int i=0; i<obsOrder.size(); i++ ){
			if ( obsOrder[i]==TString(pObs->GetName()) ) found=true;
		}
		if ( !found ) obsOrder.push_back(pObs->GetName());
	}

	// find observables we want to plot the pull of, and print their values
	vector<TString> observables;
	if ( arg->verbose ) cout << endl;
	for ( int i=obsOrder.size()-1; i>=0; i-- ){
		RooRealVar* pObs = (RooRealVar*)cmb->getObservables()->find(obsOrder[i]);
		if ( !pObs ){
			if ( arg->debug ) cout << "obs not found " << obsOrder[i] << endl;
			continue;
		}

		// find associated theory value
		TString pThName = pObs->GetName();
		pThName.ReplaceAll("obs", "th");
		RooRealVar* pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		if ( !pTh ){
			pThName = pObs->GetName();
			pThName.ReplaceAll("_obs", "");
			pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		}
		if ( !pTh ){
			if ( arg->verbose ) cout << "th not found for " << pObs->GetName() << endl;
			continue;
		}

		observables.push_back(pObs->GetName());

		// print out observables and fit result
		if ( arg->verbose ){
			printf("%30s: obs = %9.6f +/- %9.6f, th = %9.6f\n", pObs->GetName(), pObs->getVal(), pObs->getError(), pTh->getVal());
		}

		// fill histogram of pulls
		hPulls->Fill( (pTh->getVal()-pObs->getVal())/pObs->getError() );
	}
	if ( arg->verbose ) cout << endl;

	// now plot the pulls of the observables we found in chunks of 10.
	vector<TString> observablesChunk;
	for ( int i = 0; i<observables.size(); i++ ){
		observablesChunk.push_back(observables[i]);
		if ( observablesChunk.size()%10==0 ){
			plotPullsCanvas(observablesChunk, ceil((float)i/10.), ceil(observables.size()/10.), observables.size());
			observablesChunk.clear();
		}
	}
	if ( observables.size()%10!=0 ){
		plotPullsCanvas(observablesChunk, ceil(observables.size()/10.), ceil(observables.size()/10.), observables.size());
	}

	// and plot all the pulls
	TCanvas *cPull = newNoWarnTCanvas("cPull"+getUniqueRootName(), cmb->getTitle(), 0, 0, 600, 400);
	cPull->cd();
	hPulls->GetXaxis()->SetTitle("Pull [#sigma]");
	TFitResultPtr r = hPulls->Fit("gaus","SQ");
	hPulls->Draw("LEP");
	TLatex l;
	l.SetNDC();
	l.DrawLatex(0.7 ,0.84,"Histogram:");
	l.DrawLatex(0.72,0.78,Form("#mu = (%4.2f #pm %4.2f)",hPulls->GetMean(),hPulls->GetMeanError()));
	l.DrawLatex(0.72,0.72,Form("#sigma = (%4.2f #pm %4.2f)",hPulls->GetRMS(),hPulls->GetRMSError()));
	l.DrawLatex(0.7 ,0.66,"Fit:");
	l.DrawLatex(0.72,0.60,Form("#mu = (%4.2f #pm %4.2f)",r->Parameter(1),r->ParError(1)));
	l.DrawLatex(0.72,0.54,Form("#sigma = (%4.2f #pm %4.2f)",r->Parameter(2),r->ParError(2)));
	savePlot(cPull, "pull_tot_"+cmb->getName());

}

///
/// Check pulls using the current values of the parameters.
///
/// \param nsigma - threshold value
/// \return - True, if one pull is above N sigma.
///
bool PullPlotter::hasPullsAboveNsigma(float nsigma)
{
	TIterator* it = cmb->getObservables()->createIterator();
	while ( RooRealVar* pObs = (RooRealVar*)it->Next() ){
		// find associated theory value
		TString pThName = pObs->GetName();
		pThName.ReplaceAll("obs", "th");
		RooRealVar* pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		if ( !pTh ){
			if ( arg->verbose ) cout << "hasPullsAboveNsigma() : WARNING : th not found for " << pObs->GetName() << endl;
			continue;
		}
		// compute pull
		float pull = (pTh->getVal()-pObs->getVal())/pObs->getError();
		if ( fabs(pull)>nsigma ) return true;
	}
	return false;
}

///
/// Print pulls using the current values of the parameters.
/// \param aboveNsigma Only print pulls above (or equal) N sigma.
///
void PullPlotter::printPulls(float aboveNsigma)
{
	TIterator* it = cmb->getObservables()->createIterator();
	while ( RooRealVar* pObs = (RooRealVar*)it->Next() ){
		// find associated theory value
		TString pThName = pObs->GetName();
		pThName.ReplaceAll("obs", "th");
		RooRealVar* pTh = (RooRealVar*)cmb->getTheory()->find(pThName);
		if ( !pTh ){
			if ( arg->verbose ) cout << "printPulls() : WARNING : th not found for " << pObs->GetName() << endl;
			continue;
		}
		// compute pull
		float pull = (pTh->getVal()-pObs->getVal())/pObs->getError();
		if ( fabs(pull)>=aboveNsigma ){
			printf("%30s: obs = %9.6f +/- %8.6f, th = %9.6f, pull = %5.2f\n",
					pObs->GetName(), pObs->getVal(), pObs->getError(), pTh->getVal(), pull);
		}
	}
}
