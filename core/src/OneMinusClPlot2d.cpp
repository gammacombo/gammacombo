/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "OneMinusClPlot2d.h"

	OneMinusClPlot2d::OneMinusClPlot2d(OptParser *arg, TString name, TString title)
: OneMinusClPlotAbs(arg,name,title)
{
	contoursOnly = false;
	xTitle = "";
	yTitle = "";
	ColorBuilder cb;
	m_legend = 0;

	// ==== define style ====
	for ( int i=0; i<5; i++ ){
		// initialize containers holding the plot style
		vector<int> tmp1; linecolor.push_back(tmp1);
		vector<int> tmp2; linestyle.push_back(tmp2);
		vector<int> tmp3; fillcolor.push_back(tmp3);
		vector<int> tmp4; fillstyle.push_back(tmp4);
	}
	// 1st scanner
	markerstyle.push_back(3);
	markersize.push_back(1.6);
	// 1 simga
	linecolor[0].push_back(TColor::GetColor(24,170,231));
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(TColor::GetColor("#bee7fd"));
	fillstyle[0].push_back(3005);
	// 2 sigma
	linecolor[1].push_back(9);
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(TColor::GetColor("#d3e9ff"));
	fillstyle[1].push_back(1001);
	// 3 sigma
	linecolor[2].push_back(cb.lightcolor(linecolor[1][0]));
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(cb.lightcolor(fillcolor[1][0]));
	fillstyle[2].push_back(1001);
	// 4 sigma
	linecolor[3].push_back(cb.lightcolor(linecolor[2][0]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][0]));
	fillstyle[3].push_back(1001);
	// 5 sigma
	linecolor[4].push_back(cb.lightcolor(linecolor[3][0]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][0]));
	fillstyle[4].push_back(1001);

	// 2nd scanner
	markerstyle.push_back(29);
	markersize.push_back(1.7);

	linecolor[0].push_back(kOrange-5);
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(TColor::GetColor(229,199,152));
	fillstyle[0].push_back(3013);

	linecolor[1].push_back(kOrange+4);
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(TColor::GetColor(254,222,185));
	fillstyle[1].push_back(1001);

	linecolor[2].push_back(cb.lightcolor(linecolor[1][1]));
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(cb.lightcolor(fillcolor[1][1]));
	fillstyle[2].push_back(1001);

	linecolor[3].push_back(cb.lightcolor(linecolor[2][1]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][1]));
	fillstyle[3].push_back(1001);

	linecolor[4].push_back(cb.lightcolor(linecolor[3][1]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][1]));
	fillstyle[4].push_back(1001);

	// 3rd scanner
	markerstyle.push_back(20);
	markersize.push_back(1.1);

	linecolor[0].push_back(TColor::GetColor(213,0,252));
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(TColor::GetColor(244,123,255));
	fillstyle[0].push_back(3013);

	linecolor[1].push_back(TColor::GetColor(208,11,210));
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(kMagenta-9);
	fillstyle[1].push_back(1001);

	linecolor[2].push_back(cb.lightcolor(linecolor[1][2]));
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(cb.lightcolor(fillcolor[1][2]));
	fillstyle[2].push_back(1001);

	linecolor[3].push_back(cb.lightcolor(linecolor[2][2]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][2]));
	fillstyle[3].push_back(1001);

	linecolor[4].push_back(cb.lightcolor(linecolor[3][2]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][2]));
	fillstyle[4].push_back(1001);

	// 4th scanner
	markerstyle.push_back(22);
	markersize.push_back(1.1);

	linecolor[0].push_back(kGreen+3);
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(kGreen-7);
	fillstyle[0].push_back(3013);

	linecolor[1].push_back(kGreen-1);
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(kGreen-6);
	fillstyle[1].push_back(1001);

	linecolor[2].push_back(cb.lightcolor(linecolor[1][3]));
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(kGreen-9);
	fillstyle[2].push_back(1001);

	linecolor[3].push_back(cb.lightcolor(linecolor[2][3]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][3]));
	fillstyle[3].push_back(1001);

	linecolor[4].push_back(cb.lightcolor(linecolor[3][3]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][3]));
	fillstyle[4].push_back(1001);

	// 5th scanner
	markerstyle.push_back(22);
	markersize.push_back(1.1);

	linecolor[0].push_back(kOrange+3);
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(kOrange-7);
	fillstyle[0].push_back(3013);

	linecolor[1].push_back(kOrange-1);
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(kOrange-6);
	fillstyle[1].push_back(1001);

	linecolor[2].push_back(kOrange-2);
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(kOrange-9);
	fillstyle[2].push_back(1001);

	linecolor[3].push_back(cb.lightcolor(linecolor[2][4]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][4]));
	fillstyle[3].push_back(1001);

	linecolor[4].push_back(cb.lightcolor(linecolor[3][4]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][4]));
	fillstyle[4].push_back(1001);

	// scanners 6-15
	// colors based on http://colorbrewer2.org/, six classes, qualitative, second scheme
	makeNewPlotStyle("#1b9e77"); // sea green
	makeNewPlotStyle("#d95f02"); // dark orange
	makeNewPlotStyle("#7570b3"); // medium purple
	makeNewPlotStyle("#e7298a"); // medium violet red
	makeNewPlotStyle("#66a61e"); // forest green
	makeNewPlotStyle("#e6ab02"); // goldenrod
	makeNewPlotStyle("#a6761d"); // chocolate
	makeNewPlotStyle("#e31a1c"); // red
  makeNewPlotStyle("#984ea3"); // darkish purple
	makeNewPlotStyle("",kBlue-5); // same as 1D scan 1
	makeNewPlotStyle("",kGreen-8); // same as 1D scan 2

	// if requested, add or remove any fill pattern to make cleaner plots
  for ( int iContour=0; iContour<fillstyle.size(); iContour++ ) {
    for ( int iScanner=0; iScanner<fillstyle[iContour].size(); iScanner++ ) {
      // remove any fill style at all
      if ( arg->isQuickhack(10) ) fillstyle[iContour][iScanner] = 1001;
      if ( iScanner < arg->fillstyle.size() ) fillstyle[iContour][iScanner] = arg->fillstyle[iScanner];
      //cout << iContour << " " << iScanner << " " << linecolor[iContour][iScanner] << " " << linestyle[iContour][iScanner] << " " << fillcolor[iContour][iScanner] << " " << fillstyle[iContour][iScanner] << endl;
    }
  }

}

///
/// The plot style of the first N scanners is defined in the constructor.
/// To be able to plot more scanners, we here define a generic new style
/// based on the provided HTML color.
///
/// \param htmlColor - an HTML color, e.g. "#e6ab02". If ROOT is provided,
/// the new scanner will be based on a predefined ROOT color.
///
void OneMinusClPlot2d::makeNewPlotStyle(TString htmlColor, int ROOTColor)
{
	int currentNumberOfStyles = linecolor[0].size();
	// get index of new color. Either use the provided HTML color, or
	// take a predefined ROOT color.
	int newColor;
	if ( htmlColor.EqualTo("ROOT") ) newColor = currentNumberOfStyles;
	else if ( ROOTColor > 0 ) newColor = ROOTColor;
	else newColor = TColor::GetColor(htmlColor);
	markerstyle.push_back(20);
	markersize.push_back(1.1);
	ColorBuilder cb;
	float thisMuchDarker = 1.1;
	linecolor[0].push_back(cb.darklightcolor(newColor,0.7));
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(newColor);
	fillstyle[0].push_back(3005);
	linecolor[1].push_back(cb.darklightcolor(linecolor[0][currentNumberOfStyles],thisMuchDarker));
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(cb.darklightcolor(fillcolor[0][currentNumberOfStyles],thisMuchDarker));
	fillstyle[1].push_back(1001);
	linecolor[2].push_back(cb.darklightcolor(linecolor[1][currentNumberOfStyles],thisMuchDarker));
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(cb.darklightcolor(fillcolor[1][currentNumberOfStyles],thisMuchDarker));
	fillstyle[2].push_back(1001);
	linecolor[3].push_back(cb.darklightcolor(linecolor[2][currentNumberOfStyles],thisMuchDarker));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.darklightcolor(fillcolor[2][currentNumberOfStyles],thisMuchDarker));
	fillstyle[3].push_back(1001);
	linecolor[4].push_back(cb.darklightcolor(linecolor[3][currentNumberOfStyles],thisMuchDarker));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.darklightcolor(fillcolor[3][currentNumberOfStyles],thisMuchDarker));
	fillstyle[4].push_back(1001);

}

///
/// Add a scanner to the list of things to be plotted.
///
void OneMinusClPlot2d::addScanner(MethodAbsScan* s)
{
	if ( arg->debug ) cout << "OneMinusClPlot2d::addScanner() : adding " << s->getName() << endl;
	scanners.push_back(s);
	if ( s->getMethodName().EqualTo("Prob") ){
		histosType.push_back(kChi2);
		histos.push_back(s->getHchisq2d());
	}
	else {
		histosType.push_back(kPvalue);
		histos.push_back(s->getHCL2d());
	}
	if ( arg->smooth2d ) for ( int i=0; i<arg->nsmooth; i++ ) { histos[histos.size()-1]->Smooth(); }
	title = s->getTitle();
	m_contours.push_back(0);
	m_contours_computed.push_back(false);
}


void OneMinusClPlot2d::addFile(TString fName)
{
	if ( arg->debug ) cout << "OneMinusClPlot2d::addFile() : Opening " << fName << endl;
	TFile *f = TFile::Open(fName, "ro");
	TH2F *hCL = (TH2F*)f->Get("hCL");
	if ( !hCL ){
		cout << "OneMinusClPlot2d::addFile() : ERROR : File doesn't contain hCL." << endl;
		return;
	}
	histos.push_back(hCL);
	if ( arg->smooth2d ) for ( int i=0; i<arg->nsmooth; i++ ) { histos[histos.size()-1]->Smooth(); }
	histosType.push_back(kPvalue);
	m_contours.push_back(0);
	m_contours_computed.push_back(false);
}

///
/// Draw the group label on a 2d plot at a higher position.
///
void OneMinusClPlot2d::drawGroup()
{
	OneMinusClPlotAbs::drawGroup(0.775);
}

///
/// Checks if one of the scanners to be plotted provided
/// a histogram of a certain type
///
/// t - histogram type: kChi2 or kPvalue
///
bool OneMinusClPlot2d::hasHistoType(histogramType t)
{
	for ( int i=0; i<histosType.size(); i++ ){
		if ( histosType[i]==t ) return true;
	}
	return false;
}

///
/// Draw a line stating the CL content of the contours.
///
void OneMinusClPlot2d::drawCLcontent(bool isFull)
{
	float xLow, yLow;
	xLow = 0.17;
	yLow = 0.15;
  if ( isFull ) {
    xLow = 0.11;
    yLow = 0.11;
  }
	TPaveText *t1 = new TPaveText(xLow, yLow, xLow+0.20, yLow+0.125, "BRNDC");
	t1->SetBorderSize(0);
	t1->SetFillStyle(0);
	t1->SetTextAlign(12);
	t1->SetTextFont(font);
	t1->SetTextSize(labelsize*0.6);
	t1->SetTextColor(kGray+1);
  if ( isFull ) t1->SetTextColor( kRed );
	TString text;
	text = "contours hold ";
	if ( arg->plot2dcl[0]>0 || (histos.size()==1 && hasHistoType(kPvalue)) ){
		if ( arg->plotnsigmacont>0 ) text += "68%";
		if ( arg->plotnsigmacont>1 ) text += ", 95%";
	}
	else{
		if ( arg->plotnsigmacont>0 ) text += "39%";
		if ( arg->plotnsigmacont>1 ) text += ", 87%";
	}
	text += " CL";
	if ( arg->plotnsigmacont>2 ) text += " (etc.)";
	t1->AddText(text);
	t1->Draw();
}

///
/// Draw the full DeltaChi2 histogram of the first scanner.
///
void OneMinusClPlot2d::DrawFull()
{
	if ( arg->debug ){
		cout << "OneMinusClPlot2d::DrawFull() : drawing ..." << endl;
	}
	if ( histos.size()>1 ){
		cout << "OneMinusClPlot2d::DrawFull() : WARNING : can only draw the full histogram of the first" << endl;
		cout << "                                         scanner." << endl;
	}
	if ( m_mainCanvas==0 ){
		m_mainCanvas = newNoWarnTCanvas(name+getUniqueRootName(), title, 800, 600);
	}
	m_mainCanvas->cd();
	m_mainCanvas->SetMargin(0.1,0.15,0.1,0.1);
	TH2F *hChi2 = histos[0];
	hChi2->SetContour(95);
	hChi2->GetXaxis()->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle());
	hChi2->GetYaxis()->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle());
	float zMin = hChi2->GetMinimum();
	float zMax;
	if ( histosType[0]==kChi2 ){
		zMax = fmin(zMin+25, hChi2->GetMaximum());
	}
	else{
		zMax = 1;
	}
	hChi2->GetZaxis()->SetRangeUser(zMin,zMax);
	hChi2->GetZaxis()->SetTitle(histosType[0]==kChi2?"#Delta#chi^{2}":"p-value");
	hChi2->Draw("colz");

  // draw contours if requested
  ConfidenceContours* cont = new ConfidenceContours(arg);
  cont->computeContours(histos[0], histosType[0], 0);
  vector<int> linecolor { kRed, kRed, kRed, kRed, kRed };
  //vector<int> linestyle { kDashed, kDashed, kDashed, kDashed, kDashed };
  vector<int> linestyle { 1, 2, 3, 4, 5 };
  TColor *col = gROOT->GetColor(0);
  col->SetAlpha(1.);
  vector<int> fillcolor { 0, 0, 0, 0, 0 };
  vector<int> fillstyle { 0, 0, 0, 0, 0 };
  cont->setStyle( linecolor, linestyle, fillcolor, fillstyle );
  cont->setTransparency( 1. );
  cont->Draw();

	if ( !arg->isQuickhack(15) ) drawCLcontent(true);

	TPaveText *title = new TPaveText(.10,.92,.90,.99,"BRNDC");
	title->AddText(Form("%s for %s",histosType[0]==kChi2?"#Delta#chi^{2}":"p-value",scanners[0]->getTitle().Data()));
	title->SetBorderSize(0);
	title->SetFillStyle(0);
	title->SetTextFont(133);
	title->SetTextSize(35);
	title->Draw();
	m_mainCanvas->Update();
}

///
/// Draw the legend.
///
void OneMinusClPlot2d::drawLegend()
{
	if ( arg->debug ){
		cout << "OneMinusClPlot2d::drawLegend() : drawing legend ..." << endl;
	}
	// set up the legend
	float legendXmin = arg->plotlegx!=-1. ? arg->plotlegx : 0.17;
	float legendYmin = arg->plotlegy!=-1. ? arg->plotlegy : 0.75;
	float legendXmax = legendXmin + (arg->plotlegsizex!=-1. ? arg->plotlegsizex : 0.38) ;
	float legendYmax = legendYmin + (arg->plotlegsizey!=-1. ? arg->plotlegsizey : 0.15) ;
	if ( m_legend ) delete m_legend;
	m_legend = new TLegend(legendXmin,legendYmin,legendXmax,legendYmax);
	m_legend->SetFillColor(0);
	m_legend->SetFillStyle(0);
	m_legend->SetBorderSize(0);
	m_legend->SetTextFont(font);
	m_legend->SetTextSize(legendsize);
  if ( arg->isQuickhack(26) ) m_legend->SetTextSize(0.9*legendsize);

	// build legend
	for ( int i = 0; i < histos.size(); i++ ){
		if ( histos.size()==1 ){
			// no legend symbol if only one scanner to be drawn
			m_legend->AddEntry((TObject*)0, scanners[i]->getTitle(), "");
		}
		else{
			// construct a dummy TGraph that uses the style of the 1sigma line
			int styleId = i;
			if ( arg->color.size()>i ) styleId = arg->color[i];
			TGraph *g = new TGraph(1);
			g->SetFillStyle(fillstyle[0][i]); // solid
			g->SetFillColor(fillcolor[0][styleId]);
			g->SetLineWidth(2);
			g->SetLineColor(linecolor[0][styleId]);
			g->SetLineStyle(linestyle[0][styleId]);
			g->SetMarkerColor(linecolor[1][styleId]);
			g->SetMarkerStyle(markerstyle[styleId]);
			g->SetMarkerSize(markersize[styleId]);
			TString options = "f";
			if ( scanners[i]->getDrawSolution() ) options += "p"; // only plot marker symbol when solutions are plotted
			if ( scanners[i]->getTitle() != "noleg" ) m_legend->AddEntry(g, scanners[i]->getTitle(), options);
		}
	}
	m_legend->Draw();
}


void OneMinusClPlot2d::Draw()
{
	if ( arg->debug ){
		cout << "OneMinusClPlot2d::Draw() : drawing ..." << endl;
	}
	if ( scanners.size()==0 ){
		cout << "OneMinusClPlot2d::Draw() : ERROR : cannot draw " << name << " : No plots were added!" << endl;
		return;
	}
	if ( m_mainCanvas==0 ){
		m_mainCanvas = newNoWarnTCanvas(name+getUniqueRootName(), title, 800, 600);
	}

	if ( arg->isQuickhack(14) ){
		m_mainCanvas->GetPad(0)->SetLeftMargin(0.14);
	}

	TH2F *hCL = histos[0];
	float min1 = arg->scanrangeMin  == arg->scanrangeMax  ? hCL->GetXaxis()->GetXmin() : arg->scanrangeMin;
	float max1 = arg->scanrangeMin  == arg->scanrangeMax  ? hCL->GetXaxis()->GetXmax() : arg->scanrangeMax;
	float min2 = arg->scanrangeyMin == arg->scanrangeyMax ? hCL->GetYaxis()->GetXmin() : arg->scanrangeyMin;
	float max2 = arg->scanrangeyMin == arg->scanrangeyMax ? hCL->GetYaxis()->GetXmax() : arg->scanrangeyMax;

	// build a histogram which holds the axes
	TH1 *haxes = new TH2F("haxes"+getUniqueRootName(), "haxes", 100, min1, max1, 100, min2, max2);
	haxes->GetXaxis()->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle());
	haxes->GetYaxis()->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle());
	haxes->GetXaxis()->SetLabelFont(font);
	haxes->GetYaxis()->SetLabelFont(font);
	haxes->GetXaxis()->SetTitleFont(font);
	haxes->GetYaxis()->SetTitleFont(font);
	haxes->GetXaxis()->SetTitleOffset(0.85);
	if ( arg->isQuickhack(14) ){
		haxes->GetYaxis()->SetTitleOffset(0.65);
	}
	else {
		haxes->GetYaxis()->SetTitleOffset(0.95);
	}
	haxes->GetXaxis()->SetLabelSize(labelsize);
	haxes->GetYaxis()->SetLabelSize(labelsize);
	haxes->GetXaxis()->SetTitleSize(titlesize);
	haxes->GetYaxis()->SetTitleSize(titlesize);
	int xndiv = arg->ndiv==-1 ? 407 : abs(arg->ndiv);
	int yndiv = arg->ndivy==-1 ? 407 : abs(arg->ndivy);
	bool optimizeNdivx = arg->ndiv<0 ? true : false;
	bool optimizeNdivy = arg->ndivy<0 ? true : false;
	haxes->GetXaxis()->SetNdivisions(xndiv, optimizeNdivx);
	haxes->GetYaxis()->SetNdivisions(yndiv, optimizeNdivy);
	haxes->Draw();

	// make new scanner styles if we're plotting more scanners
	// than styles defined in the constructor.
	if ( linecolor[0].size()<histos.size() ){
		cout << "OneMinusClPlot2d::Draw() : WARNING : making a new plot style" << endl;
		cout << "OneMinusClPlot2d::Draw() :   for a new scanner that doesn't have" << endl;
		cout << "OneMinusClPlot2d::Draw() :   a style defined in the constructor." << endl;
	}
	for ( int i=linecolor[0].size(); i<histos.size(); i++ ){
		makeNewPlotStyle("ROOT");
	}

	// make contours
	for ( int i=0; i < histos.size(); i++ ){
		if ( m_contours_computed[i] ) continue;
		ConfidenceContours* cont = new ConfidenceContours(arg);
		cont->computeContours(histos[i], histosType[i], i);
		int styleId = i;
		if ( arg->color.size()>i ) styleId = arg->color[i];
		cont->setStyle(transpose(linecolor)[styleId], transpose(linestyle)[styleId], transpose(fillcolor)[styleId], transpose(fillstyle)[i]);
		m_contours[i] = cont;
		m_contours_computed[i] = true;
	}
	m_mainCanvas->cd(); // ConfidenceContours::computeContours() creates a temporary canvas

	// set transparency
	if ( arg->isQuickhack(12) ){
		for ( int i=0; i < histos.size(); i++ ){
			m_contours[i]->setTransparency(0.2);
			// don't use transparency for the last scanner
			if ( arg->isQuickhack(13) && i==histos.size()-1 ) m_contours[i]->setTransparency(0.);
		}
	}

	// draw filled contours first
	if ( ! contoursOnly ){
		for ( int i=0; i < m_contours.size(); i++ ){
			m_contours[i]->Draw();
		}
	}

	// draw a second time, this time only the lines
	if ( ! arg->isQuickhack(11) ){
		for ( int i=0; i < m_contours.size(); i++ ){
			m_contours[i]->DrawDashedLine();
		}
	}

	gPad->Update();
	float ymin = gPad->GetUymin();
	float ymax = gPad->GetUymax();
	float xmin = gPad->GetUxmin();
	float xmax = gPad->GetUxmax();

	// Draw new axes.
	haxes->GetXaxis()->SetNdivisions(0);  // disable old axis
	haxes->GetYaxis()->SetNdivisions(0);
	// configure axis draw options
	TString xtchopt = "-U"; // - = downward ticks, U = unlabeled, http://root.cern.ch/root/html534/TGaxis.html
	TString xbchopt = "";
	TString ylchopt = "";
	TString yrchopt = "+U"; // + = put ticks on the other side
	if ( !optimizeNdivx ){
		xtchopt += "N"; // N = no bin optimization
		xbchopt += "N";
	}
	if ( !optimizeNdivy ){
		ylchopt += "N";
		yrchopt += "N";
	}
	// New X axis. For angles in units of degrees.
	if ( isAngle(scanners[0]->getScanVar1()) ){
		haxes->GetXaxis()->SetTitle(haxes->GetXaxis()->GetTitle() + TString(" [#circ]"));

		// new axis for the top ticks
		TGaxis *axist = new TGaxis(xmin, ymax, xmax, ymax, RadToDeg(xmin), RadToDeg(xmax), xndiv, xtchopt);
		axist->SetName("axist");
		axist->Draw();

		// new bottom axis
		TGaxis *axisb = new TGaxis(xmin, ymin, xmax, ymin, RadToDeg(xmin), RadToDeg(xmax), xndiv, xbchopt);
		axisb->SetName("axisb");
		axisb->SetLabelOffset(haxes->GetXaxis()->GetLabelOffset());
		axisb->SetLabelFont(font);
		axisb->SetLabelSize(labelsize);
		axisb->Draw();
	}
	else
	{
		// add top axis
		TGaxis *axist = new TGaxis(xmin, ymax, xmax, ymax, xmin, xmax, xndiv, xtchopt);
		axist->SetName("axist");
		axist->Draw();

		// draw a new bottom axis because the confidence contours can cover the old one
		TGaxis *axisb = new TGaxis(xmin, ymin, xmax, ymin, xmin, xmax, xndiv, xbchopt);
		axisb->SetName("axisb");
		axisb->SetLabelOffset(haxes->GetXaxis()->GetLabelOffset());
		axisb->SetLabelFont(font);
		axisb->SetLabelSize(labelsize);
		axisb->Draw();
	}

	// New Y axis. For angles in units of degrees.
	if ( isAngle(scanners[0]->getScanVar2()) ){
		haxes->GetYaxis()->SetTitle(haxes->GetYaxis()->GetTitle() + TString(" [#circ]"));

		// new left axis
		TGaxis *axisl = new TGaxis(xmin, ymin, xmin, ymax, RadToDeg(ymin), RadToDeg(ymax), yndiv, ylchopt);
		axisl->SetName("axisl");
		axisl->SetLabelOffset(haxes->GetYaxis()->GetLabelOffset());
		axisl->SetLabelFont(font);
		axisl->SetLabelSize(labelsize);
		axisl->Draw();

		// new axis for the right ticks
		TGaxis *axisr = new TGaxis(xmax, ymin, xmax, ymax, RadToDeg(ymin), RadToDeg(ymax), yndiv, yrchopt);
		axisr->SetName("axisr");
		axisr->Draw();
	}
	else
	{
		// new left axis (so that is not covered by the plot)
		haxes->GetYaxis()->SetNdivisions(0);
		TGaxis *axisl = new TGaxis(xmin, ymin, xmin, ymax, ymin, ymax, yndiv, ylchopt);
		axisl->SetName("axisl");
		axisl->SetLabelOffset(haxes->GetYaxis()->GetLabelOffset());
		axisl->SetLabelFont(font);
		axisl->SetLabelSize(labelsize);
		axisl->Draw();

		// new right axis
		TGaxis *axisr = new TGaxis(xmax, ymin, xmax, ymax, ymin, ymax, yndiv, yrchopt);
		axisr->SetName("axisr");
		axisr->Draw();
	}

	drawSolutions();
	drawLegend();
	drawGroup();
	if ( !arg->isQuickhack(15) ) drawCLcontent();
	m_mainCanvas->Update();
	m_mainCanvas->Show();
}


void OneMinusClPlot2d::drawMarker(float x, float y, int color, int style, float size)
{
	TMarker *m = new TMarker(x, y, style);
	m->SetMarkerSize(size);
	m->SetMarkerColor(color);
	m->Draw();
}

///
/// Draw a marker at the position of
/// each local minimum, as provided by MethodAbsScan::getScanVar1Solution()
/// and MethodAbsScan::getScanVar2Solution().
///
void OneMinusClPlot2d::drawSolutions()
{
	m_mainCanvas->cd();
	m_mainCanvas->Update();
	float ymin = gPad->GetUymin();
	float ymax = gPad->GetUymax();
	float xmin = gPad->GetUxmin();
	float xmax = gPad->GetUxmax();

	for ( int i=0; i<scanners.size(); i++ )
	{
		if ( scanners[i]->getDrawSolution()==0 ) continue;
		if ( arg->debug ) cout << "OneMinusClPlot2d::drawSolutions() : adding solutions for scanner " << i << " ..." << endl;
		if ( scanners[i]->getNSolutions()==0 ){
			cout << "OneMinusClPlot2d::drawSolutions() : WARNING : \n"
														 "        Plot solutions requested but no solutions found!\n"
																						  "        Perform a 1d scan first or use MethodAbsScan::setSolutions()." << endl;
		}
		int markerColor = 0;
		int styleId = i;
		if ( arg->color.size()>i ) styleId = arg->color[i];
		if ( i<linecolor[1].size() ) markerColor = linecolor[1][styleId];
		else cout << "OneMinusClPlot2d::drawSolutions() : ERROR : not enough marker colors" << endl;
		int markerStyle = 3;
		if ( i<markerstyle.size() ) markerStyle = markerstyle[styleId];
		else cout << "OneMinusClPlot2d::drawSolutions() : ERROR : not enough marker styles" << endl;
		float markerSize = 2.0;
		if ( i<markersize.size() ) markerSize = markersize[styleId];
		else cout << "OneMinusClPlot2d::drawSolutions() : ERROR : not enough marker sizes" << endl;

		for ( int iSol=0; iSol<scanners[i]->getNSolutions(); iSol++ )
		{
			/// if option 2 is given, only plot best fit points and equivalent ones
			if ( scanners[i]->getDrawSolution()==2
					&& iSol>0
					&& scanners[i]->getSolution(iSol)->minNll() - scanners[i]->getSolution(0)->minNll() > 0.01 ) continue;
			float xSol = scanners[i]->getScanVar1Solution(iSol);
			float ySol = scanners[i]->getScanVar2Solution(iSol);
			if ( arg->debug ) cout << "OneMinusClPlot2d::drawSolutions() : " << xSol << " " << ySol << endl;
			drawMarker(xSol, ySol, markerColor, markerStyle, markerSize);
		}
	}
}

