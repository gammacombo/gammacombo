#include "ConfidenceContours.h"

ConfidenceContours::ConfidenceContours(OptParser *arg)
{
  assert(arg);
  m_arg = arg;
}

ConfidenceContours::~ConfidenceContours()
{}

///
/// Helper function for computeContours():
/// Constructs a new 2D histogram that contains 2 more bins in each
/// direction that are set to 0, so that the contours will always close.
///
/// \param hist - the 2D histogram
/// \return new 2D histogram. Caller assumes ownership.
///
TH2F* ConfidenceContours::addBoundaryBins(TH2F* hist)
{
	TH2F* hBoundaries = new TH2F(getUniqueRootName(),getUniqueRootName(),
			hist->GetNbinsX()+2,
			hist->GetXaxis()->GetXmin() - hist->GetXaxis()->GetBinWidth(1),
			hist->GetXaxis()->GetXmax() + hist->GetXaxis()->GetBinWidth(1),
			hist->GetNbinsY()+2,
			hist->GetYaxis()->GetXmin() - hist->GetYaxis()->GetBinWidth(1),
			hist->GetYaxis()->GetXmax() + hist->GetYaxis()->GetBinWidth(1));
	for ( int ix=1; ix<=hBoundaries->GetXaxis()->GetNbins(); ix++ ){
		for ( int iy=1; iy<=hBoundaries->GetYaxis()->GetNbins(); iy++ ){
			if ( ix==1 || ix==hBoundaries->GetXaxis()->GetNbins()
					|| iy==1 || iy==hBoundaries->GetYaxis()->GetNbins() )
				hBoundaries->SetBinContent(ix,iy,0);
			else{
				hBoundaries->SetBinContent(ix,iy,hBoundaries->GetBinContent(ix-1,iy-1));
			}
		}
	}
	return hBoundaries;
}

///
/// Helper function for computeContours():
/// Transforms the chi2 valley into a hill to help ROOTs contour mechanism
///
/// \param hist - the 2D histogram
///
void ConfidenceContours::transformChi2valleyToHill(TH2F* hist, float offset)
{
	float chi2min = hist->GetMinimum();
	for ( int ix=1; ix<=hist->GetXaxis()->GetNbins(); ix++ ){
		for ( int iy=1; iy<=hist->GetYaxis()->GetNbins(); iy++ ){
			hist->SetBinContent(ix,iy,-hist->GetBinContent(ix,iy)+offset+chi2min);
		}
	}
}

///
/// Compute the raw N sigma confidence contours from a 2D histogram
/// holding either the chi2 or the p-value curve. The resulting
/// contours are stored into the m_contours member.
///
/// \param hist - the 2D histogram
/// \param type - the type of the 2D histogram, either chi2 or p-value
///
void ConfidenceContours::computeContours(TH2F* hist, histogramType type)
{
	if ( m_arg->debug ) {
		cout << "ConfidenceContours::computeContours() : making contours of histogram ";
		cout << ", type " << (type==kChi2?"chi2":"p-value") << endl;
	}
	// clean up contours from a previous call
	m_contours.clear();

	// preprocessing of the histogram
	hist = addBoundaryBins(hist);
	float offset = 30.;
	if ( type==kChi2 ) transformChi2valleyToHill(hist,offset);

	// make contours
	const int nMaxContours = 5;
	hist->SetContour(nMaxContours);
	if ( type==kChi2 ) {
		// chi2 units
		if ( m_arg->plot2dcl ){
			hist->SetContourLevel(4, offset- 2.30);
			hist->SetContourLevel(3, offset- 6.18);
			hist->SetContourLevel(2, offset-11.83);
			hist->SetContourLevel(1, offset-19.34);
			hist->SetContourLevel(0, offset-28.76);
		}
		else{
			hist->SetContourLevel(4, offset-1.);
			hist->SetContourLevel(3, offset-4.);
			hist->SetContourLevel(2, offset-9.);
			hist->SetContourLevel(1, offset-16.);
			hist->SetContourLevel(0, offset-25.);
		}
	}
	else {
		// p-value units
		hist->SetContourLevel(4, 0.3173);
		hist->SetContourLevel(3, 4.55e-2);
		hist->SetContourLevel(2, 2.7e-3);
		hist->SetContourLevel(1, 6.3e-5);
		hist->SetContourLevel(0, 5.7e-7);
	}

	// draw and access the contours
	TCanvas *ctmp = new TCanvas(getUniqueRootName(), "ctmp");
	hist->Draw("contlist");
	gPad->Update();	// needed to be able to access the contours as TGraphs
	TObjArray *contours = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
	delete ctmp;
	delete hist;

	// access contours. They get filled in reverse order,
	// and depend on how many are actually present. If all 5
	// are filled, index 0 is 5sigma. If only 2 are filled, index 0
	// is 2 sigma.
	int nEmptyContours = 0;
	for ( int ic=4; ic>=0; ic-- ){
		if (((TList*)contours->At(ic))->IsEmpty()) nEmptyContours++;
	}
	for ( int ic=4; ic>=0; ic-- ){
		if ( !(((TList*)contours->At(ic))->IsEmpty()) ){
			Contour* cont = new Contour(m_arg, (TList*)contours->At(ic));
			cont->setSigma(5-nEmptyContours-ic);
			m_contours.push_back(cont);
		}
	}
}

///
/// Draw the contours into the currenlty active Canvas.
///
void ConfidenceContours::Draw()
{
	for ( int i=0; i<m_contours.size(); i++ )
	{
		m_contours[i]->Draw();
	}
}

