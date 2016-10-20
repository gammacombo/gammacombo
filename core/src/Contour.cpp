#include "Contour.h"

///
/// Constructor. The class stores copies of the TGraphs provided in listOfGraphs.
///
/// \param arg - command line options
/// \param listOfGraphs - list of TGraphs that make up the subcontours
///
Contour::Contour(OptParser *arg, TList *listOfGraphs)
{
	assert(arg);
	m_arg = arg;
	TIterator* it = listOfGraphs->MakeIterator();
	while ( TGraph *g = (TGraph*)it->Next() ){
		m_contours.push_back((TGraph*)g->Clone());
	}
	delete it;
	m_linecolor = 2;
	m_linestyle = kSolid;
	m_fillcolor = 2;
	m_fillstyle = 1001;
	m_linewidth = 1;
	m_alpha = 1.;

	// compute holes in the contours
	m_contoursHoles = makeHoles(m_contours);
}

Contour::~Contour()
{
	for ( int i=0; i<m_contours.size(); i++ ){
		delete m_contours[i];
	}
	for ( int i=0; i<m_contoursHoles.size(); i++ ){
		delete m_contoursHoles[i];
	}
}

///
/// Draw the contours into the currently active Canvas.
///
void Contour::Draw()
{
	//cout << "Contour::Draw() : drawing contour (sigma=" << m_sigma << ") ..." << endl;
	DrawFilled();
	DrawLine();
}

///
/// Draw filled contours into the currently active Canvas.
/// This plots the contours in m_contoursHoles.
///
void Contour::DrawFilled()
{
	for ( int i=0; i<m_contoursHoles.size(); i++ ) {
		TGraph* g = (TGraph*)m_contoursHoles[i]->Clone();
		g->SetFillStyle(1001); // solid
		//g->SetFillColor(m_fillcolor);
		g->SetFillColorAlpha(m_fillcolor,m_alpha); // transparency!
		if (!m_arg->isQuickhack(27)) g->Draw("F");
		if ( m_fillstyle!=1001 ){ // if not solid, add the pattern in the line color
			g = (TGraph*)m_contoursHoles[i]->Clone();
			g->SetFillStyle(m_fillstyle); // hatched on top
			g->SetFillColor(m_linecolor);
			g->Draw("F");
		}
	}
}

///
/// Draw line contours into the currently active Canvas.
/// This plots the contours in m_contours.
///
void Contour::DrawLine()
{
	for ( int i=0; i<m_contours.size(); i++ ) {
		TGraph* g = (TGraph*)m_contours[i]->Clone();
		g->SetLineWidth(m_linewidth);
		g->SetLineColor(m_linecolor);
		g->SetLineStyle(m_linestyle);
		g->SetFillStyle(0); // hollow
		g->Draw("L");
	}
}

///
/// Set the contour style.
///
void Contour::setStyle(int linecolor, int linestyle, int linewidth, int fillcolor, int fillstyle)
{
	m_linecolor = linecolor;
	m_linestyle = linestyle;
	m_fillcolor = fillcolor;
	m_fillstyle = fillstyle;
	m_linewidth = linewidth;
}

///
/// Make contours that have holes cut away.
/// If a, say, 1sigma contour looks like a ring, it is initially
/// stored as two independent circles in the TList. Here we join
/// them, if one lies inside the other, such that when the resulting
/// contour is plotted, the plot will actually have a hole.
///
/// Fills m_contoursHoles. The reason this is kept separate from
/// the m_contours container, that holds the contours without holes,
/// is the plotting: one can't plot the just-lines version from the
/// contours with holes, else one sees where the contour is artificially
/// closed to be able to fill the inside.
///
/// \param contours - vector containing contours without holes
/// \return - vector with contours with holes
///
vector<TGraph*> Contour::makeHoles(vector<TGraph*>& contours)
{
	int n = contours.size();
	bool joined = false;
	int iJoined1;
	int iJoined2;
	TGraph *gJoined = 0;
	for ( int i1=0; i1<n; i1++ ){
		if ( joined ) break;
		TGraph *g1 = contours[i1];
		for ( int i2=i1+1; i2<n; i2++ ){
			TGraph *g2 = contours[i2];
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
		vector<TGraph*> newContours;
		newContours.push_back(gJoined);
		for ( int i=0; i<n; i++ ){
			if ( i!=iJoined1 && i!=iJoined2 ) newContours.push_back(contours[i]);
		}
		return makeHoles(newContours);
	}
	return contours;
}

///
/// Helper function for makeHoles().
///
TGraph* Contour::joinIfInside(TGraph *g1, TGraph *g2)
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

///
/// Helper function for joinIfInside().
///
TGraph* Contour::changePointOrder(TGraph *g, int pointId)
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

///
/// Helper function for makeHoles().
///
void Contour::findClosestPoints(TGraph *g1, TGraph *g2, int &i1, int &i2)
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
	// printf("Contour::findClosestPoints(): point 1 = [%f,%f] (id %2i), point 2 = [%f,%f] (id %2i)\n",
	//   x1, y1, i1, x2, y2, i2);
}


//float Contour::getXBoundary(float p1x, float p1y, float p2x, float p2y, float ymax)
//{
	//return p1x + (ymax-p1y)/(p2y-p1y)*(p2x-p1x);
//}

//float Contour::getYBoundary(float p1x, float p1y, float p2x, float p2y, float xmax)
//{
	//return p1y + (xmax-p1x)/(p2x-p1x)*(p2y-p1y);
//}

///
/// Magnetic boundaries. If a contour is closer than half a binwidth
/// to a boundary, adjust it to be actually the boundary.
///
/// \param contour - input contours
/// \param hCL - a histogram defining the boundaries
///
void Contour::magneticBoundaries(vector<TGraph*>& contours, const TH2F* hCL)
{
	float magneticRange = 0.75;
	float xmin = hCL->GetXaxis()->GetXmin();
	float xmax = hCL->GetXaxis()->GetXmax();
	float ymin = hCL->GetYaxis()->GetXmin();
	float ymax = hCL->GetYaxis()->GetXmax();
	float xbinwidth = hCL->GetXaxis()->GetBinWidth(1);
	float ybinwidth = hCL->GetYaxis()->GetBinWidth(1);
	Double_t pointx, pointy;
	for ( int j=0; j<contours.size(); j++ ) {
		TGraph* g = (TGraph*)contours[j];
		for ( int i=0; i<g->GetN(); i++) {
			g->GetPoint(i, pointx, pointy);
			if ( abs(pointx-xmin) < xbinwidth*magneticRange ) g->SetPoint(i, xmin, pointy);
			g->GetPoint(i, pointx, pointy);
			if ( abs(pointx-xmax) < xbinwidth*magneticRange ) g->SetPoint(i, xmax, pointy);
			g->GetPoint(i, pointx, pointy);
			if ( abs(pointy-ymin) < ybinwidth*magneticRange ) g->SetPoint(i, pointx, ymin);
			g->GetPoint(i, pointx, pointy);
			if ( abs(pointy-ymax) < ybinwidth*magneticRange ) g->SetPoint(i, pointx, ymax);
		}
	}
}

void Contour::magneticBoundaries(const TH2F* hCL)
{
	magneticBoundaries(m_contours, hCL);
	magneticBoundaries(m_contoursHoles, hCL);
}

///
/// Set transparency.
///
/// \param percent - 0% means intransparent
///
void Contour::setTransparency(float percent)
{
	if ( ! ( 0. <= percent && percent <= 1. ) ){
		cout << "Contour::setTransparency() : ERROR : percent not in [0,1]. Skipping." << endl;
		return;
	}
	m_alpha = 1.-percent;
}
