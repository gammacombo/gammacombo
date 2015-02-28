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

	linecolor[0].push_back(kGray+3);
	linestyle[0].push_back(kSolid);
	fillcolor[0].push_back(kGray-7);
	fillstyle[0].push_back(3013);

	linecolor[1].push_back(kGray-1);
	linestyle[1].push_back(kSolid);
	fillcolor[1].push_back(kGray-6);
	fillstyle[1].push_back(1001);

	linecolor[2].push_back(kGray-2);
	linestyle[2].push_back(kSolid);
	fillcolor[2].push_back(kGray-9);
	fillstyle[2].push_back(1001);

	linecolor[3].push_back(cb.lightcolor(linecolor[2][4]));
	linestyle[3].push_back(kSolid);
	fillcolor[3].push_back(cb.lightcolor(fillcolor[2][4]));
	fillstyle[3].push_back(1001);

	linecolor[4].push_back(cb.lightcolor(linecolor[3][4]));
	linestyle[4].push_back(kSolid);
	fillcolor[4].push_back(cb.lightcolor(fillcolor[3][4]));
	fillstyle[4].push_back(1001);

	// scanners 6-10
	// colors based on http://colorbrewer2.org/, six classes, qualitative, second scheme
	makeNewPlotStyle("#1b9e77");
	makeNewPlotStyle("#d95f02");
	makeNewPlotStyle("#7570b3");
	makeNewPlotStyle("#e7298a");
	makeNewPlotStyle("#66a61e");
	makeNewPlotStyle("#e6ab02");

	// if requested, remove any fill pattern to make cleaner plots
	if ( arg->isQuickhack(10) ){
		for ( int iScanners=0; iScanners<fillstyle.size(); iScanners++ ){
			for ( int iContours=0; iContours<fillstyle[iScanners].size(); iContours++ ){
				fillstyle[iScanners][iContours] = 1001;
			}
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
void OneMinusClPlot2d::makeNewPlotStyle(TString htmlColor)
{
	int currentNumberOfStyles = linecolor[0].size();
	// get index of new color. Either use the provided HTML color, or
	// take a predefined ROOT color.
	int newColor;
	if ( htmlColor.EqualTo("ROOT") ) newColor = currentNumberOfStyles;
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
  if ( arg->smooth2d ) histos[histos.size()-1]->Smooth();
	title = s->getTitle();
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
  if ( arg->smooth2d ) histos[histos.size()-1]->Smooth();
	histosType.push_back(kPvalue);
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
void OneMinusClPlot2d::drawCLcontent()
{
	float xLow, yLow;
	xLow = 0.17;
	yLow = 0.15;
	TPaveText *t1 = new TPaveText(xLow, yLow, xLow+0.20, yLow+0.125, "BRNDC");
	t1->SetBorderSize(0);
	t1->SetFillStyle(0);
	t1->SetTextAlign(12);
	t1->SetTextFont(font);
	t1->SetTextSize(labelsize*0.6);
	t1->SetTextColor(kGray+1);
	TString text;
	if ( histos.size()>1 && hasHistoType(kPvalue) && !arg->plot2dcl ){
		cout << endl;
		cout << "OneMinusClPlot2d::drawCLcontent() : WARNING : plotting contours of inconsistent CL content." << endl;
		cout << "                                              Use --2dcl to restore consitency." << endl;
		cout << endl;
		text = "contours have inconsistent CL content";
	}
	else {
		text = "contours hold ";
		if ( arg->plot2dcl || (histos.size()==1 && hasHistoType(kPvalue)) ){
			if ( arg->plotnsigmacont>0 ) text += "68%";
			if ( arg->plotnsigmacont>1 ) text += ", 95%";
		}
		else{
			if ( arg->plotnsigmacont>0 ) text += "39%";
			if ( arg->plotnsigmacont>1 ) text += ", 87%";
		}
		text += " CL";
		if ( arg->plotnsigmacont>2 ) text += " (etc.)";
	}
	t1->AddText(text);
	t1->Draw();
}


TGraph* OneMinusClPlot2d::joinIfInside(TGraph *g1, TGraph *g2)
{
	// First determine which graph lies inside which, if they include each
	// other at all.
	// Get number of points of g1 that lie inside g2:
	Double_t pointx, pointy;
	int nG1InsideG2 = 0;
	for ( int i=0; i<g1->GetN(); i++){
		g1->GetPoint(i, pointx, pointy);
		if ( g2->IsInside(pointx,pointy) ) nG1InsideG2++;
	}
	// reversed: Get number of points of g2 that lie inside g1:
	int nG2InsideG1 = 0;
	for ( int i=0; i<g2->GetN(); i++){
		g2->GetPoint(i, pointx, pointy);
		if ( g1->IsInside(pointx,pointy) ) nG2InsideG1++;
	}
	// they don't contain each other
	if ( nG1InsideG2==0 && nG2InsideG1==0 ) return 0;
	// they do contain each other: merge them into g1 so that the line
	// will form a hole! We'll merge at the points that are closest.
	int i1, i2;
	findClosestPoints(g1, g2, i1, i2);
	// cout << "g1 ===========" << endl;
	// g1->Print();
	// cout << "g2 ===========" << endl;
	// g2->Print();
	// change graph order such that it stars and ends with the nearest point
	g1 = changePointOrder(g1, i1);
	g2 = changePointOrder(g2, i2);
	// cout << "g1 ===========" << endl;
	// g1->Print();
	// cout << "g2 ===========" << endl;
	// g2->Print();
	// merge them
	TGraph *gNew = new TGraph(g1->GetN()+g2->GetN());
	for ( int i=0; i<g1->GetN(); i++){
		g1->GetPoint(i, pointx, pointy);
		gNew->SetPoint(i, pointx,pointy);
	}
	for ( int i=0; i<g2->GetN(); i++){
		g2->GetPoint(i, pointx, pointy);
		gNew->SetPoint(i+g1->GetN(),pointx,pointy);
	}
	// cout << "gNew ===========" << endl;
	// gNew->Print();
	return gNew;
}


TGraph* OneMinusClPlot2d::changePointOrder(TGraph *g, int pointId)
{
	Double_t pointx, pointy;
	TGraph *gNew = new TGraph(g->GetN()+1);
	for ( int i=pointId; i<g->GetN()+pointId; i++){
		g->GetPoint(i<g->GetN() ? i : i-g->GetN(), pointx, pointy);
		gNew->SetPoint(i-pointId,pointx,pointy);
	}
	gNew->GetPoint(0, pointx, pointy);
	gNew->SetPoint(gNew->GetN()-1,pointx,pointy);
	return gNew;
}


void OneMinusClPlot2d::findClosestPoints(TGraph *g1, TGraph *g2, int &i1, int &i2)
{
	Double_t x1, y1, x2, y2;
	double distance = 1e6;
	for ( int ii1=0; ii1<g1->GetN(); ii1++){
		for ( int ii2=0; ii2<g2->GetN(); ii2++){
			g1->GetPoint(ii1, x1, y1);
			g2->GetPoint(ii2, x2, y2);
			double d = sqrt(sq(x1-x2)+sq(y1-y2));
			if ( d<distance ){
				i1 = ii1;
				i2 = ii2;
				distance = d;
			}
		}
	}
	g1->GetPoint(i1, x1, y1);
	g2->GetPoint(i2, x2, y2);
	// printf("OneMinusClPlot2d::findClosestPoints(): point 1 = [%f,%f] (id %2i), point 2 = [%f,%f] (id %2i)\n",
	//   x1, y1, i1, x2, y2, i2);
}

///
/// Make contours that have holes cut away.
/// If a, say, 1sigma contour looks like a ring, it is initially
/// stored as two independent circles in the TList. Here we join
/// them, if one lies inside the other, such that when the resulting
/// contour is plotted, the plot will actually have a hole.
/// \param contour list of contours as generated by Roots contour plot
///         mechanism, where each contour is a TGraph, possibly containing
///         each other
/// \return A new contour list, that now contains contours with holes,
///         and the disjoint contours
///
TList* OneMinusClPlot2d::makeHoles(TList *contour)
{
	if ( !contour ){
		cout << "OneMinusClPlot2d::makeHoles() : ERROR : contour is empty." << endl;
		assert(0);
	}
	int n = contour->GetSize();
	bool joined = false;
	int iJoined1;
	int iJoined2;
	TGraph *gJoined = 0;
	for ( int i1=0; i1<n; i1++ ){
		if ( joined ) break;
		TGraph *g1 = (TGraph*)contour->At(i1);
		for ( int i2=i1+1; i2<n; i2++ ){
			TGraph *g2 = (TGraph*)contour->At(i2);
			gJoined = joinIfInside(g1,g2);
			if ( gJoined ){
				joined = true;
				iJoined1 = i1;
				iJoined2 = i2;
				break;
			}
		}
	}

	if ( joined ){
		TList *newContour = new TList();
		newContour->Add(gJoined);
		for ( int i=0; i<n; i++ ){
			TGraph *g = (TGraph*)contour->At(i);
			if ( i!=iJoined1 && i!=iJoined2 ) newContour->Add(g);
		}
		return makeHoles(newContour);
	}

	return contour;
}

///
/// make a contour plot out of a 2D histogram holding
/// a p-value or a chi2 histogram.
///
/// \param hCLid - index of the histogram in the histos member
/// \param nContours - number of contours to be made
/// \param plotFilled - make filled contours
/// \param last - ?
///
TMultiGraph* OneMinusClPlot2d::makeContours(int hCLid, int nContours, bool plotFilled, bool last)
{
	if ( arg->debug ) {
		cout << "OneMinusClPlot2d::makeContours() : making contours of histogram id " << hCLid
		<< ", type " << (histosType[hCLid]==kChi2?"kChi2":"kPvalue") << endl;
	}
	TH2F* h = histos[hCLid];

	// get coordinates of plotted area
	float xmin = h->GetXaxis()->GetXmin();
	float xmax = h->GetXaxis()->GetXmax();
	float ymin = h->GetYaxis()->GetXmin();
	float ymax = h->GetYaxis()->GetXmax();

	// construct a new 2d histogram that contains 2 more bins in each
	// direction that are set to 0, so that the contours always close
	TH2F* hBoundaries = new TH2F(getUniqueRootName(),getUniqueRootName(),
			h->GetNbinsX()+2,
			h->GetXaxis()->GetXmin() - h->GetXaxis()->GetBinWidth(1),
			h->GetXaxis()->GetXmax() + h->GetXaxis()->GetBinWidth(1),
			h->GetNbinsY()+2,
			h->GetYaxis()->GetXmin() - h->GetYaxis()->GetBinWidth(1),
			h->GetYaxis()->GetXmax() + h->GetYaxis()->GetBinWidth(1));
	for ( int ix=1; ix<=hBoundaries->GetXaxis()->GetNbins(); ix++ ){
		for ( int iy=1; iy<=hBoundaries->GetYaxis()->GetNbins(); iy++ ){
			if ( ix==1 || ix==hBoundaries->GetXaxis()->GetNbins()
					|| iy==1 || iy==hBoundaries->GetYaxis()->GetNbins() )
				hBoundaries->SetBinContent(ix,iy,0);
			else{
				double chi2min = h->GetMinimum();
				if ( histosType[hCLid]==kChi2 ) {
					// transform the chi2 valley into a mountain to help ROOTs contour mechanism
					hBoundaries->SetBinContent(ix,iy,-h->GetBinContent(ix-1,iy-1)+30.+chi2min);
				}
				else {
					hBoundaries->SetBinContent(ix,iy,h->GetBinContent(ix-1,iy-1));
				}
			}
		}
	}

	// make contours
	// hBoundaries is 30-chi2. We add 30 to make a positive hill to help Root with the contours.
	TCanvas *ctmp = new TCanvas(getUniqueRootName(), "ctmp");
	hBoundaries->SetContour(5);
	if ( histosType[hCLid]==kChi2 ) {
		// chi2 units
		if ( arg->plot2dcl ){
			hBoundaries->SetContourLevel(4, 30.- 2.30);
			hBoundaries->SetContourLevel(3, 30.- 6.18);
			hBoundaries->SetContourLevel(2, 30.-11.83);
			hBoundaries->SetContourLevel(1, 30.-19.34);
			hBoundaries->SetContourLevel(0, 30.-28.76);
		}
		else{
			hBoundaries->SetContourLevel(4, 30.-1.);
			hBoundaries->SetContourLevel(3, 30.-4.);
			hBoundaries->SetContourLevel(2, 30.-9.);
			hBoundaries->SetContourLevel(1, 30.-16.);
			hBoundaries->SetContourLevel(0, 30.-25.);
		}
	}
	else {
		// p-value units
		hBoundaries->SetContourLevel(4, 0.3173);
		hBoundaries->SetContourLevel(3, 4.55e-2);
		hBoundaries->SetContourLevel(2, 2.7e-3);
		hBoundaries->SetContourLevel(1, 6.3e-5);
		hBoundaries->SetContourLevel(0, 5.7e-7);
	}
	hBoundaries->Draw("contlist");
	gPad->Update();	// access contours as TGraphs
	TObjArray *contours = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
	delete ctmp;
	c1->cd();

	// access contours. They get filled in reverse order,
	// and depend on how many are actually present. If all 5
	// are filled, index 0 is 5sigma. If only 2 are filled, index 0
	// is 2 sigma.
	TList *contour[5] = {0};
	int nEmptyContours = 0;
	for ( int ic=4; ic>=0; ic-- ){
		if (((TList*)contours->At(ic))->IsEmpty()) nEmptyContours++;
	}
	for ( int ic=4; ic>=0; ic-- ){
		if ( !(((TList*)contours->At(ic))->IsEmpty()) ){
			contour[4-nEmptyContours-ic] = (TList*)contours->At(ic);
		}
	}

	// fill out the first empty contour so that it becomes just the plotted area
	for ( int ic=0; ic<5; ic++ ){
		if ( contour[ic] ) continue;
		TGraph *g = new TGraph(5);
		g->SetPoint(0, xmin, ymin);
		g->SetPoint(1, xmin, ymax);
		g->SetPoint(2, xmax, ymax);
		g->SetPoint(3, xmax, ymin);
		g->SetPoint(4, xmin, ymin);
		contour[ic] = new TList();
		contour[ic]->Add(g);
		break;
	}

	// magnetic boundaries
	if ( arg->plotmagnetic ) {
		for ( int ic=4; ic>=0; ic-- ){
			TIterator* it = contour[ic]->MakeIterator();
			while ( TGraph *g = (TGraph*)it->Next() ){
				magneticBoundaries(g, h);
			}
		}
	}

	// draw the contours
	int styleId = hCLid;
	if ( arg->color.size()>hCLid ) styleId = arg->color[hCLid];
	TMultiGraph *mg = new TMultiGraph(getUniqueRootName(), "graph");
	for ( int ic=nContours-1; ic>=0; ic-- ){
		drawContour(mg, contour[ic], 2, linecolor[ic][styleId], linestyle[ic][styleId],
				fillcolor[ic][styleId], fillstyle[ic][styleId], kDashed,
				last, plotFilled);
	}

	return mg;
}

///
/// Magnetic boundaries. If a contour is closer than half a binwidth
/// to a boundary, adjust it to be actually the boundary.
///
void OneMinusClPlot2d::magneticBoundaries(TGraph *g, const TH2F* hCL)
{
	float xmin = hCL->GetXaxis()->GetXmin();
	float xmax = hCL->GetXaxis()->GetXmax();
	float ymin = hCL->GetYaxis()->GetXmin();
	float ymax = hCL->GetYaxis()->GetXmax();
	Double_t pointx, pointy;
	for ( int i=0; i<g->GetN(); i++)
	{
		g->GetPoint(i, pointx, pointy);
		float magneticRange = 0.75;
		if ( abs(pointx-xmin)<hCL->GetXaxis()->GetBinWidth(1)*magneticRange ) g->SetPoint(i, xmin, pointy);
		g->GetPoint(i, pointx, pointy);
		if ( abs(pointx-xmax)<hCL->GetXaxis()->GetBinWidth(1)*magneticRange ) g->SetPoint(i, xmax, pointy);
		g->GetPoint(i, pointx, pointy);
		if ( abs(pointy-ymin)<hCL->GetYaxis()->GetBinWidth(1)*magneticRange ) g->SetPoint(i, pointx, ymin);
		g->GetPoint(i, pointx, pointy);
		if ( abs(pointy-ymax)<hCL->GetYaxis()->GetBinWidth(1)*magneticRange ) g->SetPoint(i, pointx, ymax);
	}
}

///
/// Add the contours from the supplied list to the multigraph,
/// setting their plot style while doing so.
///
void OneMinusClPlot2d::drawContour(TMultiGraph *mg, TList* contour, int linewidth, int linecolor, int linestyle,
		int fillcolor, int fillstyle, int linestylelast, bool last, bool plotFilled)
{
	assert(mg);
	assert(contour);
	// draw filled area of the contour
	if ( plotFilled ){
		TList* contourf = makeHoles(contour);
		TIterator* it = contourf->MakeIterator();
		while ( TGraph *g = (TGraph*)it->Next() ){
			g = (TGraph*)g->Clone();
			g->SetFillStyle(1001); // solid
			g->SetFillColor(fillcolor);
			mg->Add(g, "F");
			if ( fillstyle!=1001 ){ // if not solid, add the pattern in the line color
				g = (TGraph*)g->Clone();
				g->SetFillStyle(fillstyle); // hatched on top
				g->SetFillColor(linecolor);
				mg->Add(g, "F");
			}
		}
	}
	// draw the lines of the contour
	TIterator* it = contour->MakeIterator();
	while ( TGraph *g = (TGraph*)it->Next() ){
		g->SetLineWidth(linewidth);
		g->SetLineColor(linecolor);
		g->SetLineStyle(linestyle);
		g->SetFillStyle(0); // hollow
		if ( last ) g->SetLineStyle(linestylelast);
		g = (TGraph*)g->Clone();
		mg->Add(g, "L");
	}
}

///
/// Draw the full DeltaChi2 histogram of the first scanner.
///
void OneMinusClPlot2d::DrawFull()
{
	if ( histos.size()>1 ){
		cout << "OneMinusClPlot2d::DrawFull() : WARNING : can only draw the full histogram of the first" << endl;
		cout << "                                         scanner." << endl;
	}
	c1->cd();
	c1->SetMargin(0.1,0.15,0.1,0.1);
	TH2F *hChi2 = histos[0];
	hChi2->SetContour(95);
	hChi2->GetXaxis()->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle());
	hChi2->GetYaxis()->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle());
	hChi2->GetZaxis()->SetRangeUser(hChi2->GetMinimum(),hChi2->GetMinimum()+(histosType[0]==kChi2?25:1));
	hChi2->GetZaxis()->SetTitle(histosType[0]==kChi2?"#Delta#chi^{2}":"p-value");
	hChi2->Draw("colz");
	TPaveText *title = new TPaveText(.10,.92,.90,.99,"BRNDC");
	title->AddText(Form("%s for %s",histosType[0]==kChi2?"#Delta#chi^{2}":"p-value",scanners[0]->getTitle().Data()));
	title->SetBorderSize(0);
	title->SetFillStyle(0);
	title->SetTextFont(133);
	title->SetTextSize(35);
	title->Draw();
	c1->Update();
}

///
/// Draw the legend.
///
void OneMinusClPlot2d::drawLegend()
{
	// set up the legend
	float legendXmin = arg->plotlegx!=-1. ? arg->plotlegx : 0.17;
	float legendYmin = arg->plotlegy!=-1. ? arg->plotlegy : 0.75;
	float legendXmax = legendXmin + (arg->plotlegsizex!=-1. ? arg->plotlegsizex : 0.38) ;
	float legendYmax = legendYmin + (arg->plotlegsizey!=-1. ? arg->plotlegsizey : 0.15) ;
	TLegend* leg = new TLegend(legendXmin,legendYmin,legendXmax,legendYmax);
	leg->SetFillColor(0);
	leg->SetFillStyle(0);
	leg->SetBorderSize(0);
	leg->SetTextFont(font);
	leg->SetTextSize(legendsize);

	// build legend
	for ( int i = 0; i < histos.size(); i++ ){
		if ( histos.size()==1 ){
			// no legend symbol if only one scanner to be drawn
			leg->AddEntry((TObject*)0, scanners[i]->getTitle(), "");
		}
		else{
			// construct a dummy TGraph that uses the style of the 1sigma line
			int styleId = i;
			if ( arg->color.size()>i ) styleId = arg->color[i];
			TGraph *g = new TGraph(1);
			g->SetFillStyle(1001); // solid
			g->SetFillColor(fillcolor[0][styleId]);
			g->SetLineWidth(2);
			g->SetLineColor(linecolor[0][styleId]);
			g->SetLineStyle(linestyle[0][styleId]);
			g->SetMarkerColor(linecolor[1][styleId]);
			g->SetMarkerStyle(markerstyle[styleId]);
			g->SetMarkerSize(markersize[styleId]);
			TString options = "f";
			if ( scanners[i]->getDrawSolution() ) options += "p"; // only plot marker symbol when solutions are plotted
			leg->AddEntry(g, scanners[i]->getTitle(), options);
		}
	}
	leg->Draw();
}


void OneMinusClPlot2d::Draw()
{
	if ( scanners.size()==0 ){
		cout << "OneMinusClPlot2d::Draw() : ERROR : cannot draw " << name << " : No plots were added!" << endl;
		return;
	}
	c1->cd();
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
	haxes->GetYaxis()->SetTitleOffset(0.95);
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

	// draw filled contours first
	vector<TMultiGraph*> gFirst;
	if ( ! contoursOnly )
		for ( int i = 0; i < histos.size(); i++ ){
			gFirst.push_back(makeContours(i, arg->plotnsigmacont));
			gFirst[i]->Draw();
		}

	// draw a second time, this time only the lines
	vector<TMultiGraph*> gSecond;
	for ( int i = 0; i < histos.size(); i++ ){
		gSecond.push_back(makeContours(i, arg->plotnsigmacont, false, true));
		gSecond[i]->Draw();
	}

	// draw a third time, one sigma only, to get objects for the legend
	vector<TMultiGraph*> gLegend;
	for ( int i = 0; i < histos.size(); i++ ){
		gLegend.push_back(makeContours(i, 1));
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
	drawCLcontent();
	c1->Update();
	c1->Show();
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
	c1->cd();
	c1->Update();
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

///
/// Save the plot.
///
void OneMinusClPlot2d::save()
{
	savePlot(c1, name);
}
