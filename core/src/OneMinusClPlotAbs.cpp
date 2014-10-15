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
  titlesize  = 45;  ///< axis titles, "LHCb", "Preliminary" is x0.75
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
	c1 = new TCanvas(name+getUniqueRootName(), title, 800, 600);
	
  if ( arg->plotlog ){
    c1->SetLogy();
    this->name = this->name + "_log";
  }
  if ( arg->plotprelim ){
    this->name = this->name + "_prelim";
  }
  plotLegend    = true;
  plotSolution  = true;
  plotLogYMin   = 1.e-3;
  plotLogYMax   = 1;

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
	TString saveName = name;
	if ( arg->isQuickhack(7) ){
		saveName.ReplaceAll("_addedPdf","+");
		saveName.ReplaceAll("_delPdf","-");
	}
  savePlot(c1, saveName);
}

///
/// Draw the LHCb (preliminary) label on plots.
/// It is possible to configure the position of the label through
/// the --lhcb command line argument.
///
void OneMinusClPlotAbs::drawLHCb(float yPos)
{
  if ( arg->lhcb==TString("off") ) return;
  float xLow, yLow;
  if ( arg->plotlhcbx==-1 ) xLow = 0.75; else xLow = arg->plotlhcbx;
  if ( arg->plotlhcby==-1 ) yLow = yPos; else yLow = arg->plotlhcby;
  TPaveText *t1 = new TPaveText(xLow, yLow, xLow+0.20, yLow+0.125, "BRNDC");
  t1->SetBorderSize(0);
  t1->SetFillStyle(0);
  t1->SetTextAlign(12);
  t1->SetTextFont(font);
  t1->SetTextSize(titlesize*1.0);
  t1->AddText("LHCb");
  if ( arg->plotprelim ) t1->AddText("Preliminary")->SetTextSize(titlesize*0.525);
  else if ( arg->plotunoff ) t1->AddText("Unofficial")->SetTextSize(titlesize*0.525);
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
