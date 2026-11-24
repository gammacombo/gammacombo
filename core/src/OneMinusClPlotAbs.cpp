/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: August 2012
 *
 **/

#include <OneMinusClPlotAbs.h>

#include <MethodAbsScan.h>
#include <OptParser.h>

#include <TPaveText.h>
#include <TString.h>
#include <TStyle.h>

#include <iostream>

OneMinusClPlotAbs::OneMinusClPlotAbs(const OptParser* arg, TString name, TString title) {
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
}

OneMinusClPlotAbs::~OneMinusClPlotAbs() {
  if (m_mainCanvas) delete m_mainCanvas;
}

///
/// Add a scanner to this plot.
///
void OneMinusClPlotAbs::addScanner(MethodAbsScan* s, int CLsType) {
  if (arg->debug) std::cout << "OneMinusClPlotAbs::addScanner() : adding " << s->getName() << std::endl;
  if (CLsType == 0 || (CLsType == 1 && s->getHCLs()) || (CLsType == 2 && s->getHCLsFreq())) {
    scanners.push_back(s);
    do_CLs.push_back(CLsType);
  } else {
    std::cout << "OneMinusClPlotAbs::addScanner() : WARNING : no CLs type " << CLsType << " histogram for "
              << s->getName() << " (" << s->getMethodName() << "), not plotting its CLs curve." << std::endl;
  }
}

///
/// Save the plot.
///
void OneMinusClPlotAbs::save() {
  if (m_mainCanvas == 0) {
    std::cout << "OneMinusClPlotAbs::save() : ERROR : Empty canvas. Call Draw() or DrawFull() before saving!"
              << std::endl;
    return;
  }
  Utils::savePlot(m_mainCanvas, name + arg->plotext);
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
void OneMinusClPlotAbs::drawGroup(double yPos) {
  if (arg->group == TString("off")) return;
  m_mainCanvas->cd();
  double xPos = 0.65;
  double xLow, yLow;
  if (arg->plotgroupx == -1)
    xLow = xPos;
  else
    xLow = arg->plotgroupx;
  if (arg->plotgroupy == -1)
    yLow = yPos;
  else
    yLow = arg->plotgroupy;
  TPaveText* t1 = new TPaveText(xLow, yLow, xLow + 0.225, yLow + 0.08, "BRNDC");
  t1->SetBorderSize(0);
  t1->SetFillStyle(0);
  t1->SetTextAlign(32);
  t1->SetTextFont(font);
  t1->SetTextSize(titlesize * 1.0);
  t1->AddText(arg->group);
  t1->Draw();
  if (arg->plotprelim || arg->plotunoff) {
    TPaveText* t2 = new TPaveText(xLow, yLow - 0.025, xLow + 0.225, yLow, "BRNDC");
    t2->SetBorderSize(0);
    t2->SetFillStyle(0);
    t2->SetTextAlign(32);
    t2->SetTextFont(font);
    t2->SetTextSize(titlesize * 0.525);
    if (arg->plotprelim) t2->AddText("Preliminary");
    if (arg->plotunoff) t2->AddText("Unofficial");
    t2->Draw();
  }
  if (arg->plotdate != "") {
    double yExt = 0.;
    if (arg->plotprelim || arg->plotunoff) yExt += 0.035;
    TPaveText* t3 = new TPaveText(xLow, yLow - yExt - 0.025, xLow + 0.225, yLow - yExt, "BRNDC");
    t3->SetBorderSize(0);
    t3->SetFillStyle(0);
    t3->SetTextAlign(32);
    t3->SetTextFont(font);
    t3->SetTextSize(titlesize * 0.46);
    t3->AddText(arg->plotdate);
    t3->Draw();
  }
}

void OneMinusClPlotAbs::drawSolutions() {
  // not implemented
}

void OneMinusClPlotAbs::Draw() {
  // not implemented
}
