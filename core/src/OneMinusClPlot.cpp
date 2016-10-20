/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "OneMinusClPlot.h"

	OneMinusClPlot::OneMinusClPlot(OptParser *arg, TString name, TString title)
: OneMinusClPlotAbs(arg, name, title)
{
	plotPluginMarkers = true;
	plotSolution      = true;
}

///
/// Make a plot out of a 1D histogram holding a 1-CL curve.
/// The strategy is to always convert the 1-CL histogram (hCL) into
/// a TGraph. This way we can add known points (solutions, points
/// at end of scan range) and also have a filled area without line
/// smoothing. This is not possible with histograms due to a Root bug.
///
/// The function draws the TGraphs, and returns a pointer to the
/// TGraph object that can be used in the TLegend.
///
/// Markers are plotted if the method name of the scanner is "Plugin" or "BergerBoos" or "GenericPlugin".
/// One can plot a line instead of points even for the Plugin method by
/// using setPluginMarkers().
///
/// For the angle variables, a new axis is painted that is in Deg.
///
/// \param s The scanner to plot.
/// \param first
/// \param last
/// \param filled
///
TGraph* OneMinusClPlot::scan1dPlot(MethodAbsScan* s, bool first, bool last, bool filled)
{
	if ( arg->debug ){
		cout << "OneMinusClPlot::scan1dPlot() : plotting ";
		cout << s->getName() << " (" << s->getMethodName() << ")" << endl;
	}
	m_mainCanvas->cd();
	bool plotPoints = ( s->getMethodName()=="Plugin" || s->getMethodName()=="BergerBoos" || s->getMethodName()=="GenericPlugin" ) && plotPluginMarkers;
	TH1F *hCL = (TH1F*)s->getHCL()->Clone(getUniqueRootName());
	// fix inf and nan entries
	for ( int i=1; i<=s->getHCL()->GetNbinsX(); i++ ){
		if ( s->getHCL()->GetBinContent(i)!=s->getHCL()->GetBinContent(i)
				|| std::isinf(s->getHCL()->GetBinContent(i)) ) s->getHCL()->SetBinContent(i, 0.0);
	}

	// remove errors the hard way, else root ALWAYS plots them
	if ( !plotPoints ) hCL = histHardCopy(hCL, true, true);

	// disable any statistics box
	hCL->SetStats(0);

	// Convert the histogram into a TGraph so we can add the solution.
	// Also, the lf2 drawing option is broken in latest root versions.
	TGraph *g;
	if ( plotPoints ) g = new TGraphErrors(hCL->GetNbinsX());
	else              g = new TGraph(hCL->GetNbinsX());
	g->SetName(getUniqueRootName());
	for ( int i=0; i<hCL->GetNbinsX(); i++ ){
		g->SetPoint(i, hCL->GetBinCenter(i+1), hCL->GetBinContent(i+1));
		if ( plotPoints ) ((TGraphErrors*)g)->SetPointError(i, 0.0, hCL->GetBinError(i+1));
	}

	// add solution
	if ( s->getNSolutions()>0 && !arg->isQuickhack(21) ){
		TGraphTools t;
		TGraph *gNew = t.addPointToGraphAtFirstMatchingX(g, s->getScanVar1Solution(0), 1.0);
		delete g;
		g = gNew;
	}

	// // set last point to the same p-value as first point by hand
	// // some angle plots sometimes don't manage to do it by themselves...
	// if ( arg->isQuickhack(XX) )
	// {
	//   Double_t pointx0, pointy0, err0;
	//   Double_t pointx1, pointy1, err1;
	//   g->GetPoint(0, pointx0, pointy0);
	//   g->GetPoint(g->GetN()-1, pointx1, pointy1);
	//   g->SetPoint(g->GetN()-1, pointx1, pointy0);
	//   if ( plotPoints ) err0 = ((TGraphErrors*)g)->GetErrorY(0);
	//   if ( plotPoints ) ((TGraphErrors*)g)->SetPointError(g->GetN()-1, 0.0, err0);
	// }

	// add end points of scan range
	if ( !plotPoints )
	{
		Double_t pointx0, pointy0;
		TGraph *gNew = new TGraph(g->GetN()+4);
		gNew->SetName(getUniqueRootName());
		for ( int i=0; i<g->GetN(); i++)
		{
			g->GetPoint(i, pointx0, pointy0);
			gNew->SetPoint(i+2, pointx0, pointy0);
		}

		// add origin
		gNew->SetPoint(0, hCL->GetXaxis()->GetXmin(), 0);

		// add a point at first y height but at x=origin.
		g->GetPoint(0, pointx0, pointy0);
		gNew->SetPoint(1, hCL->GetXaxis()->GetXmin(), pointy0);

		// add a point at last y height but at x=xmax.
		g->GetPoint(g->GetN()-1, pointx0, pointy0);
		gNew->SetPoint(gNew->GetN()-2, hCL->GetXaxis()->GetXmax(), pointy0);

		// add a point at xmax, 0
		gNew->SetPoint(gNew->GetN()-1, hCL->GetXaxis()->GetXmax(), 0);
		g = gNew;
	}

	int color = s->getLineColor();
	g->SetLineColor(color);

	if ( filled ){
		g->SetLineWidth(2);
    double alpha = arg->isQuickhack(12) ? 0.4 : 1.;
    if ( arg->isQuickhack(24) ) alpha = 0.;
		g->SetFillColorAlpha(color,alpha);
		g->SetLineStyle(1);
    g->SetFillStyle( s->getFillStyle() );
	}
	else{
		g->SetLineWidth(2);
		g->SetLineStyle(s->getLineStyle());
    if ( last && arg->isQuickhack(25) ) g->SetLineWidth(3);
	}

	if ( plotPoints ){
		g->SetLineWidth(1);
		g->SetMarkerColor(color);
		g->SetMarkerStyle(8);
		g->SetMarkerSize(0.6);
	}

	// build a histogram which holds the axes
	float min = arg->scanrangeMin == arg->scanrangeMax ? hCL->GetXaxis()->GetXmin() : arg->scanrangeMin;
	float max = arg->scanrangeMin == arg->scanrangeMax ? hCL->GetXaxis()->GetXmax() : arg->scanrangeMax;
	TH1F *haxes = new TH1F("haxes"+getUniqueRootName(), "haxes", 100, min, max);
	haxes->SetStats(0);
	haxes->GetXaxis()->SetTitle(s->getScanVar1()->GetTitle());
	haxes->GetYaxis()->SetTitle("1-CL");
	haxes->GetXaxis()->SetLabelFont(font);
	haxes->GetYaxis()->SetLabelFont(font);
	haxes->GetXaxis()->SetTitleFont(font);
	haxes->GetYaxis()->SetTitleFont(font);
	haxes->GetXaxis()->SetTitleOffset(0.9);
	haxes->GetYaxis()->SetTitleOffset(0.85);
	haxes->GetXaxis()->SetLabelSize(labelsize);
	haxes->GetYaxis()->SetLabelSize(labelsize);
	haxes->GetXaxis()->SetTitleSize(titlesize);
	haxes->GetYaxis()->SetTitleSize(titlesize);
	int xndiv = arg->ndiv==-1 ? 407 : abs(arg->ndiv);
	bool optimizeNdiv = arg->ndiv<0 ? true : false;
	haxes->GetXaxis()->SetNdivisions(xndiv, optimizeNdiv);
	haxes->GetYaxis()->SetNdivisions(407, true);

  // plot y range
  float plotYMax;
  float plotYMin;
  if ( plotLegend && !arg->isQuickhack(22) ) {
    if ( arg->plotlog ) { plotYMin = 1.e-3; plotYMax = 10.; }
    else                { plotYMin = 0.0  ; plotYMax = 1.3; }
  }
  else {
    if ( arg->plotlog ) { plotYMin = 1.e-3; plotYMax = 1.0; }
    else                { plotYMin = 0.0  ; plotYMax = 1.0; }
  }
  // change if passed as option
	plotYMin = arg->plotymin > 0. ? arg->plotymin : plotYMin;
  plotYMax = arg->plotymax > 0. ? arg->plotymax : plotYMax;

  haxes->GetYaxis()->SetRangeUser( plotYMin, plotYMax );
	haxes->Draw("axissame");
	g->SetHistogram(haxes);

	TString drawOption = "";
	if ( plotPoints )   drawOption += " pe";
	else if ( filled )  drawOption += " F";
	else                drawOption += " L";
	if ( first )        drawOption += " A";
	g->Draw(drawOption);
  //if ( drawOption.Contains("F") ) ((TGraph*)g->Clone())->Draw("L");

	gPad->Update();
	float ymin = gPad->GetUymin();
	float ymax = gPad->GetUymax();
	float xmin = gPad->GetUxmin();
	float xmax = gPad->GetUxmax();

	// for the angles, draw a new axis in units of degrees
	if ( isAngle(s->getScanVar1()) ){
		haxes->GetXaxis()->SetTitle(s->getScanVar1()->GetTitle() + TString(" [#circ]"));
		haxes->GetXaxis()->SetNdivisions(0);  // disable old axis
		if ( last ){
			// new top axis
			TString chopt = "-U"; // - = downward ticks, U = unlabeled, http://root.cern.ch/root/html534/TGaxis.html
			if ( !optimizeNdiv ) chopt += "N"; // n = no bin optimization
			TGaxis *axist = new TGaxis(xmin, 1, xmax, 1, RadToDeg(xmin), RadToDeg(xmax), xndiv, chopt);
			axist->SetName("axist");
			axist->Draw();

			// new bottom axis
			float axisbMin = RadToDeg(xmin);
			float axisbMax = RadToDeg(xmax);
			if ( arg->isQuickhack(3) ){ ///< see documentation of --qh option in OptParser.cpp
				axisbMin += 180.;
				axisbMax += 180.;
			}
			chopt = ""; // - = downward ticks, U = unlabeled, http://root.cern.ch/root/html534/TGaxis.html
			if ( !optimizeNdiv ) chopt += "N"; // n = no bin optimization
			TGaxis *axisb = new TGaxis(xmin, ymin, xmax, ymin, axisbMin, axisbMax, xndiv, chopt);
			axisb->SetName("axisb");
			axisb->SetLabelFont(font);
			axisb->SetLabelSize(labelsize);
			axisb->Draw();
		}
	}
	else
	{
		if ( last ){
			// add top axis
			TString chopt = "-U"; // - = downward ticks, U = unlabeled, http://root.cern.ch/root/html534/TGaxis.html
			if ( !optimizeNdiv ) chopt += "N"; // n = no bin optimization
			TGaxis *axist = new TGaxis(xmin, 1.0, xmax, 1.0, xmin, xmax, xndiv, chopt);
			axist->SetName("axist");
			axist->SetLineWidth(1);
			axist->Draw();
		}
	}

	if ( last )
	{
		// add right axis
		TGaxis *axisr = 0;
		if ( arg->plotlog ){
			float f3min = 1e-3;
			float f3max = (plotLegend && !arg->isQuickhack(22)) ? 10. : 1.;
			TF1 *f3 = new TF1("f3","log10(x)",f3min,f3max);
			axisr = new TGaxis(xmax, f3min, xmax, f3max, "f3", 510, "G+");
		}
		else{
			axisr = new TGaxis(xmax, ymin, xmax, ymax, 0, (plotLegend && !arg->isQuickhack(22)) ? 1.3 : 1.0, 407, "+");
		}
		axisr->SetLabelSize(0);
		axisr->SetLineWidth(1);
		axisr->SetName("axisr");
		axisr->SetLabelColor(kWhite);
		axisr->SetTitleColor(kWhite);
		axisr->Draw();

		// redraw right axis as well because the 1-CL graph can cover the old one
		haxes->Draw("axissame");
	}

	return g;
}

///
/// Make a plot out of a 1D histogram holding a 1-CL curve.
/// This is a fall back function that does no fancy stuff.
///
/// \param s The scanner to plot.
/// \param first Set this to true for the first plotted scanner.
///
void OneMinusClPlot::scan1dPlotSimple(MethodAbsScan* s, bool first)
{
	if ( arg->debug ){
		cout << "OneMinusClPlot::scan1dPlotSimple() : plotting ";
		cout << s->getName() << " (" << s->getMethodName() << ")" << endl;
	}
	m_mainCanvas->cd();
	// get rit of nan and inf
	for ( int i=1; i<=s->getHCL()->GetNbinsX(); i++ ){
		if ( s->getHCL()->GetBinContent(i)!=s->getHCL()->GetBinContent(i)
				|| std::isinf(s->getHCL()->GetBinContent(i)) ) s->getHCL()->SetBinContent(i, 0.0);
	}

	int color = s->getLineColor();
	s->getHCL()->SetStats(0);
	s->getHCL()->SetLineColor(color);
	s->getHCL()->SetMarkerColor(color);
	s->getHCL()->SetLineWidth(2);
	s->getHCL()->SetLineStyle(s->getLineStyle());
	s->getHCL()->SetMarkerColor(color);
	s->getHCL()->SetMarkerStyle(8);
	s->getHCL()->SetMarkerSize(0.6);
	s->getHCL()->GetYaxis()->SetNdivisions(407, true);
	s->getHCL()->GetXaxis()->SetTitle(s->getScanVar1()->GetTitle());
	s->getHCL()->GetYaxis()->SetTitle("1-CL");
	s->getHCL()->GetXaxis()->SetLabelFont(font);
	s->getHCL()->GetYaxis()->SetLabelFont(font);
	s->getHCL()->GetXaxis()->SetTitleFont(font);
	s->getHCL()->GetYaxis()->SetTitleFont(font);
	s->getHCL()->GetXaxis()->SetTitleOffset(0.9);
	s->getHCL()->GetYaxis()->SetTitleOffset(0.85);
	s->getHCL()->GetXaxis()->SetLabelSize(labelsize);
	s->getHCL()->GetYaxis()->SetLabelSize(labelsize);
	s->getHCL()->GetXaxis()->SetTitleSize(titlesize);
	s->getHCL()->GetYaxis()->SetTitleSize(titlesize);
	if ( plotLegend && !arg->isQuickhack(22) ){
		if ( arg->plotlog ) s->getHCL()->GetYaxis()->SetRangeUser(1e-3,10);
		else                s->getHCL()->GetYaxis()->SetRangeUser(0.0,1.3);
	}
	else{
		if ( arg->plotlog ) s->getHCL()->GetYaxis()->SetRangeUser(1e-3,1);
		else                s->getHCL()->GetYaxis()->SetRangeUser(0.0,1.0);
	}
	s->getHCL()->Draw(first?"":"same");
}

void OneMinusClPlot::drawVerticalLine(float x, int color, int style)
{
	m_mainCanvas->cd();
	TLine* l1 = new TLine(x, 0., x, 1.);
	l1->SetLineWidth(1);
	l1->SetLineColor(color);
	l1->SetLineStyle(style);
	l1->Draw();
}

///
/// Draw a vertical line a the position of
/// the best solution.
/// For the getDrawSolution() code, see OptParser, --ps.
///
void OneMinusClPlot::drawSolutions()
{
	m_mainCanvas->cd();
	m_mainCanvas->Update();
	float ymin = gPad->GetUymin();
	float ymax = gPad->GetUymax();
	float xmin = gPad->GetUxmin();
	float xmax = gPad->GetUxmax();
	int iDrawn = 0;

	for ( int i = 0; i < scanners.size(); i++ )
	{
		if ( scanners[i]->getDrawSolution()==0 ) continue;
		if ( arg->debug ) cout << "OneMinusClPlot::drawSolutions() : adding solution for scanner " << i << " ..." << endl;
		float xCentral = scanners[i]->getScanVar1Solution();
		float xCLmin = scanners[i]->getCLintervalCentral().min;
		float xCLmax = scanners[i]->getCLintervalCentral().max;
		int color = scanners[i]->getTextColor();

		// draw vertical lines at central value and
		// upper/lower errors
    if ( ! arg->isQuickhack(19) ) {
      drawVerticalLine(xCentral, color, kSolid);
      if ( ! arg->isQuickhack(20) ) {
        drawVerticalLine(xCLmin, color, kDashed);
        drawVerticalLine(xCLmax, color, kDashed);
      }
    }

		// draw text box with numerical values after the lines,
		// so that it doesn't get covered

		// compute y position of the printed central value
		float yNumberMin = 0.6 - 0.13*(float)iDrawn;
		float yNumberMax = yNumberMin+0.1;
		if ( arg->plotlog ) {
			float yNumberMinFirst = 0.1;
			if ( arg->isQuickhack(1) ) yNumberMinFirst = 0.175;
			yNumberMin = yNumberMinFirst / pow(3.0, iDrawn); // move down by a constant shift on log scale
			yNumberMax = yNumberMin*2.;
		}
    // if printsoly option then move a bit
    if ( arg->printSolY > 0. ) {
      yNumberMin += arg->printSolY;
      yNumberMax += arg->printSolY;
    }

		// compute x position of the printed central value
		float xNumberMin, xNumberMax;
		if ( scanners[i]->getDrawSolution()==1 ) {
			xNumberMin = xCentral+(xmax-xmin)*0.20; // draw at central value
			xNumberMax = xCentral+(xmax-xmin)*0.0;
		}
		else if ( scanners[i]->getDrawSolution()==2 ) {
			xNumberMin = xCLmin+(xmax-xmin)*0.0; // draw at left CL boundary
			xNumberMax = xCLmin+(xmax-xmin)*0.25;
		}
		else if ( scanners[i]->getDrawSolution()==3 ) {
			xNumberMin = xCLmax+(xmax-xmin)*0.0; // draw at right CL boundary
			xNumberMax = xCLmax+(xmax-xmin)*0.25;
		}
		else if ( scanners[i]->getDrawSolution()==4 ) {
			xNumberMin = xCLmin+(xmax-xmin)*-0.20; // draw a little left of the left CL boundary
			xNumberMax = xCLmin+(xmax-xmin)*0.0;
		}
		else {
			cout << "OneMinusClPlot::drawSolutions() : ERROR : --ps code ";
			cout << scanners[i]->getDrawSolution() << " not found! Use [0,1,2,3]." << endl;
			continue;
		}

		// move number a bit to the left so it doesn't cover the right plot border anymore
		if ( arg->isQuickhack(4) )
		{
			xNumberMin -= (xmax-xmin)*0.225;
			xNumberMax -= (xmax-xmin)*0.225;
		}

    // if print solution argument given then over write
    if ( arg->printSolX > 0. )
    {
      float diff = xNumberMax - xNumberMin;
      xNumberMin = arg->printSolX;
      xNumberMax = arg->printSolX + diff;
    }

		TPaveText *t1 = new TPaveText(xNumberMin, yNumberMin, xNumberMax, yNumberMax, "BR");
		t1->SetBorderSize(0);
		t1->SetFillStyle(0);
		t1->SetTextAlign(13);
		t1->SetTextFont(font);
		t1->SetTextColor(color);
		if ( isAngle(scanners[i]->getScanVar1()) ){
			xCentral = RadToDeg(xCentral);
			xCLmin = RadToDeg(xCLmin);
			xCLmax = RadToDeg(xCLmax);
		}
		Rounder myRounder(arg, xCLmin, xCLmax, xCentral);
		int d = myRounder.getNsubdigits();
		float xCentralRd = myRounder.central();
		if ( arg->isQuickhack(3) ) xCentralRd += 180.; ///< see documentation of --qh option in OptParser.cpp
		t1->AddText(Form("%.*f^{+%.*f}_{#font[122]{-}%.*f}",
					d, xCentralRd,
					d, myRounder.errPos(),
					d, myRounder.errNeg()))->SetTextSize(labelsize);
		t1->Draw();
		iDrawn += 1;
	}
}

///
/// Draw a horizontal line at given p-value, put a
/// label on top of it stating the corresponding CL.
///
void OneMinusClPlot::drawCLguideLine(float pvalue)
{
	m_mainCanvas->cd();
	m_mainCanvas->Update();
	float ymin = gPad->GetUymin();
	float ymax = gPad->GetUymax();
	float xmin = gPad->GetUxmin();
	float xmax = gPad->GetUxmax();

	float labelPos = xmin+(xmax-xmin)*0.10;
	if ( arg->isQuickhack(2) ) labelPos = xmin+(xmax-xmin)*0.55;
  if ( arg->isQuickhack(23) ) labelPos = xmin+(xmax-xmin)*0.8;
	float labelPosYmin = 0;
	float labelPosYmax = 0;

	if ( arg->plotlog ) {
		labelPosYmin = pvalue;
		labelPosYmax = labelPosYmin * 2.;
	}
	else {
		labelPosYmin = pvalue + 0.02;
		labelPosYmax = labelPosYmin + 0.05;
	}

	TPaveText *t = new TPaveText(labelPos, labelPosYmin, labelPos+(xmax-xmin)*0.5, labelPosYmax, "BR");
	t->SetBorderSize(0);
	t->SetFillStyle(0);
	t->SetTextAlign(12);
	t->SetTextFont(font);
	t->SetTextSize(labelsize);
	t->AddText(Form("%.1f%%",(1.-pvalue)*100.));
	t->Draw();

	TLine* l = new TLine(xmin, pvalue, xmax, pvalue);
	l->SetLineWidth(1);
	l->SetLineColor(kBlack);
	l->SetLineStyle(kDotted);
	l->Draw();
}

///
/// Draw 1, 2, and 3 sigma lines.
///
void OneMinusClPlot::drawCLguideLines()
{
	drawCLguideLine(0.3173);
	drawCLguideLine(4.55e-2);
	if ( arg->plotlog ){
		drawCLguideLine(2.7e-3);
		if ( arg->plotymin < 6.3e-5 ) {
      drawCLguideLine(6.3e-5);
    }
	}
}


void OneMinusClPlot::Draw()
{
	bool plotSimple = false;//arg->debug; ///< set to true to use a simpler plot function
	///< which directly plots the 1-CL histograms without beautification
	if ( m_mainCanvas==0 ){
		m_mainCanvas = newNoWarnTCanvas(name+getUniqueRootName(), title, 800, 600);
	}
	if ( arg->plotlog ){
		m_mainCanvas->SetLogy();
		if ( !this->name.EndsWith("_log") ) this->name = this->name + "_log";
	}
	m_mainCanvas->cd();

	// Legend:
	// make the legend short, the text will extend over the boundary, but the symbol will be shorter
  float legendXmin = arg->plotlegx!=-1. ? arg->plotlegx : 0.19 ;
  float legendYmin = arg->plotlegy!=-1. ? arg->plotlegy : 0.78 ;
  float legendXmax = legendXmin + ( arg->plotlegsizex!=-1. ? arg->plotlegsizex : 0.31 ) ;
  float legendYmax = legendYmin + ( arg->plotlegsizey!=-1. ? arg->plotlegsizey : 0.1640559 ) ;
	TLegend* leg = new TLegend(legendXmin,legendYmin,legendXmax,legendYmax);
	leg->SetFillColor(kWhite);
	leg->SetFillStyle(0);
	leg->SetLineColor(kWhite);
	leg->SetBorderSize(0);
	leg->SetTextFont(font);
	leg->SetTextSize(legendsize*0.75);
	for ( int i = 0; i < scanners.size(); i++ )
	{
		TString legDrawOption = "l";
		if ( plotPluginMarkers
				&& ( scanners[i]->getMethodName()=="Plugin"
					|| scanners[i]->getMethodName()=="BergerBoos"
					|| scanners[i]->getMethodName()=="GenericPlugin" ) )
		{
			legDrawOption = "p";
		}
    if ( arg->plotlegstyle != "default" ) legDrawOption = arg->plotlegstyle;

		if ( plotSimple )
		{
			scan1dPlotSimple(scanners[i], i==0);
			leg->AddEntry(scanners[i]->getHCL(), scanners[i]->getTitle() + " (" + scanners[i]->getMethodName() + ")", legDrawOption);
		}
		else
		{
			TGraph* g = scan1dPlot(scanners[i], i==0, false, scanners[i]->getFilled());
			leg->AddEntry(g, scanners[i]->getTitle(), legDrawOption);
		}
	}

	// lines only
	if ( !plotSimple )
		for ( int i = 0; i < scanners.size(); i++ )
		{
			bool last = i==scanners.size()-1;
			scan1dPlot(scanners[i], false, last, false);
		}
	drawSolutions();
	if ( plotLegend ) leg->Draw();
  if ( arg->isQuickhack(22) ) leg->Draw();
	m_mainCanvas->Update();
	drawCLguideLines();

	// draw the logo
	float yGroup = 0.6;
	if ( plotLegend ){
		// we have a legend
		if ( arg->plotlog )   yGroup = 0.775;
		else                  yGroup = 0.60;
	}
	else{
		// no legend
		if ( arg->plotlog )   yGroup = 0.3;
		else                  yGroup = 0.775;
	}
	drawGroup(yGroup);

	m_mainCanvas->Update();
	m_mainCanvas->Show();
}
