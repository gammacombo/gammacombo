/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include "OneMinusClPlotAbs.h"

OneMinusClPlotAbs::OneMinusClPlotAbs(OptParser *arg, TString name, TString title)
{
	font       = 133;
	labelsize  = 35;  ///< axis labels, numeric solutions, CL guide lines
	titlesize  = 45;  ///< axis titles, group label, "Preliminary" is x0.75
	legendsize = 29;  ///< legends in 1d and 2d plots

	gStyle->SetOptTitle(0);
	gStyle->SetOptStat(0);
	gStyle->SetPadTopMargin(0.05);
	gStyle->SetPadRightMargin(0.05);
	gStyle->SetPadBottomMargin(0.17);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetLabelOffset(0.005, "X");
	gStyle->SetLabelOffset(0.010, "Y");

	this->arg = arg;
	this->name = name;
	this->title = title;
	m_mainCanvas = 0;

	plotLegend    = true;
	plotSolution  = true;
	plotLogYMin   = 1.e-3;
	plotLogYMax   = 1;
}

OneMinusClPlotAbs::~OneMinusClPlotAbs()
{
	if ( m_mainCanvas!=0 ) delete m_mainCanvas;
}

///
/// Add a scanner to this plot.
///
void OneMinusClPlotAbs::addScanner(MethodAbsScan* s)
{
	if ( arg->debug ) cout << "OneMinusClPlotAbs::addScanner() : adding " << s->getName() << endl;
	scanners.push_back(s);
}

///
/// Save the plot.
///
void OneMinusClPlotAbs::save()
{
	if ( m_mainCanvas==0 ){
		cout << "OneMinusClPlotAbs::save() : ERROR : Empty canvas. Call Draw() or DrawFull() before saving!" << endl;
		return;
	}
	savePlot(m_mainCanvas, name + arg->plotext);
}

///
/// Draw the group label on plots.
/// The label can be configured through the --group command line
/// argument.
/// It is possible to configure the position of the label through
/// the --groupPos command line argument.
/// The command line arguments --prelim and --unoff add "Preliminary"
/// and "Unofficial" strings, respectively.
///
void OneMinusClPlotAbs::drawGroup(float yPos)
{
	if ( arg->group==TString("off") ) return;
	m_mainCanvas->cd();
	float xPos = 0.7;
	float xLow, yLow;
	if ( arg->plotgroupx==-1 ) xLow = xPos; else xLow = arg->plotgroupx;
	if ( arg->plotgroupy==-1 ) yLow = yPos; else yLow = arg->plotgroupy;
	TPaveText *t1 = new TPaveText(xLow, yLow, xLow+0.225, yLow+0.125, "BRNDC");
	t1->SetBorderSize(0);
	t1->SetFillStyle(0);
	t1->SetTextAlign(32);
	t1->SetTextFont(font);
	t1->SetTextSize(titlesize*1.0);
	t1->AddText(arg->group);
	if ( arg->plotprelim ) t1->AddText("Preliminary")->SetTextSize(titlesize*0.525);
	if ( arg->plotunoff ) t1->AddText("Unofficial")->SetTextSize(titlesize*0.525);
	t1->Draw();
}


void OneMinusClPlotAbs::drawSolutions()
{
	// not implemented
}


void OneMinusClPlotAbs::Draw()
{
	// not implemented
}
