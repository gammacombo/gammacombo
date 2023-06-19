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
  xTitle = arg->xtitle;
  yTitle = arg->ytitle;
  ColorBuilder cb;
  m_legend = 0;

  // ==== define style ====
  for ( int i=0; i<9; i++ ){
    // initialize containers holding the plot style
    vector<int> tmp1; linecolor.push_back(tmp1);
    vector<int> tmp2; linestyle.push_back(tmp2);
    vector<int> tmp3; fillcolor.push_back(tmp3);
    vector<int> tmp4; fillstyle.push_back(tmp4);
    vector<int> tmp5; linewidth.push_back(tmp5);
  }

  // fill from the options first
  for ( int ncont=0; ncont<9; ncont++ ) {
    for ( int i=0; i<arg->linecolor.size(); i++ ){
      linecolor[ncont].push_back( arg->linecolor[i] );
    }
    for ( int i=0; i<arg->linestyle.size(); i++ ){
      linestyle[ncont].push_back( arg->linestyle[i] );
    }
    for ( int i=0; i<arg->linewidth.size(); i++ ){
      linewidth[ncont].push_back( arg->linewidth[i] );
    }
    for ( int i=0; i<arg->fillcolor.size(); i++ ){
      fillcolor[ncont].push_back( arg->fillcolor[i] );
    }
    for ( int i=0; i<arg->fillstyle.size(); i++ ){
      fillstyle[ncont].push_back( arg->fillstyle[i] );
    }
  }

  // otherwise use some defaults

  // 1st scanner
  if ( arg->plotsolutions[0]==2 ) {
    markerstyle.push_back(20);
    markersize.push_back(1.1);
  }
  else {
    markerstyle.push_back(3);
    markersize.push_back(1.6);
  }
  // 1 simga
  linecolor[0].push_back(TColor::GetColor(24,170,231));
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(TColor::GetColor("#bee7fd"));
  fillstyle[0].push_back(3005);
  linewidth[0].push_back(2);
  // 2 sigma
  linecolor[1].push_back(9);
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(TColor::GetColor("#d3e9ff"));
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);
  // 3 sigma
  linecolor[2].push_back(cb.lightcolor(linecolor[1][0]));
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(cb.lightcolor(fillcolor[1][0]));
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);
  // 4 sigma
  linecolor[3].push_back(cb.lightcolor(linecolor[2][0]));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.lightcolor(fillcolor[2][0]));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);
  // 5 sigma
  linecolor[4].push_back(cb.lightcolor(linecolor[3][0]));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.lightcolor(fillcolor[3][0]));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);
  // 6 sigma
  linecolor[5].push_back(cb.lightcolor(linecolor[4][0]));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.lightcolor(fillcolor[4][0]));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);
  // 7 sigma
  linecolor[6].push_back(cb.lightcolor(linecolor[5][0]));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.lightcolor(fillcolor[5][0]));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);
  // 8 sigma
  linecolor[7].push_back(cb.lightcolor(linecolor[6][0]));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.lightcolor(fillcolor[6][0]));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);
  // 9 sigma
  linecolor[8].push_back(cb.lightcolor(linecolor[7][0]));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.lightcolor(fillcolor[7][0]));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

  // 2nd scanner
  //markerstyle.push_back(29);
  //markersize.push_back(1.7);
  markerstyle.push_back(20);
  markersize.push_back(1.1);

  linecolor[0].push_back(kOrange-5);
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(TColor::GetColor(229,199,152));
  fillstyle[0].push_back(3013);
  linewidth[0].push_back(2);

  linecolor[1].push_back(kOrange+4);
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(TColor::GetColor(254,222,185));
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);

  linecolor[2].push_back(cb.lightcolor(linecolor[1][1]));
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(cb.lightcolor(fillcolor[1][1]));
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);

  linecolor[3].push_back(cb.lightcolor(linecolor[2][1]));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.lightcolor(fillcolor[2][1]));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);

  linecolor[4].push_back(cb.lightcolor(linecolor[3][1]));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.lightcolor(fillcolor[3][1]));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);

  linecolor[5].push_back(cb.lightcolor(linecolor[4][1]));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.lightcolor(fillcolor[4][1]));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);

  linecolor[6].push_back(cb.lightcolor(linecolor[5][1]));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.lightcolor(fillcolor[5][1]));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);

  linecolor[7].push_back(cb.lightcolor(linecolor[6][1]));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.lightcolor(fillcolor[6][1]));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);

  linecolor[8].push_back(cb.lightcolor(linecolor[7][1]));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.lightcolor(fillcolor[7][1]));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

  // 3rd scanner
  markerstyle.push_back(20);
  markersize.push_back(1.1);

  linecolor[0].push_back(TColor::GetColor(213,0,252));
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(TColor::GetColor(244,123,255));
  fillstyle[0].push_back(3013);
  linewidth[0].push_back(2);

  linecolor[1].push_back(TColor::GetColor(208,11,210));
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(kMagenta-9);
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);

  linecolor[2].push_back(cb.lightcolor(linecolor[1][2]));
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(cb.lightcolor(fillcolor[1][2]));
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);

  linecolor[3].push_back(cb.lightcolor(linecolor[2][2]));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.lightcolor(fillcolor[2][2]));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);

  linecolor[4].push_back(cb.lightcolor(linecolor[3][2]));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.lightcolor(fillcolor[3][2]));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);

  linecolor[5].push_back(cb.lightcolor(linecolor[4][2]));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.lightcolor(fillcolor[4][2]));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);

  linecolor[6].push_back(cb.lightcolor(linecolor[5][2]));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.lightcolor(fillcolor[5][2]));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);

  linecolor[7].push_back(cb.lightcolor(linecolor[6][2]));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.lightcolor(fillcolor[6][2]));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);

  linecolor[8].push_back(cb.lightcolor(linecolor[7][2]));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.lightcolor(fillcolor[7][2]));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

  // 4th scanner
  markerstyle.push_back(20);
  markersize.push_back(1.1);

  linecolor[0].push_back(kGreen+3);
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(kGreen-7);
  fillstyle[0].push_back(3013);
  linewidth[0].push_back(2);

  linecolor[1].push_back(kGreen-1);
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(kGreen-6);
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);

  linecolor[2].push_back(cb.lightcolor(linecolor[1][3]));
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(kGreen-9);
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);

  linecolor[3].push_back(cb.lightcolor(linecolor[2][3]));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.lightcolor(fillcolor[2][3]));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);

  linecolor[4].push_back(cb.lightcolor(linecolor[3][3]));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.lightcolor(fillcolor[3][3]));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);

  linecolor[5].push_back(cb.lightcolor(linecolor[4][3]));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.lightcolor(fillcolor[4][3]));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);

  linecolor[6].push_back(cb.lightcolor(linecolor[5][3]));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.lightcolor(fillcolor[5][3]));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);

  linecolor[7].push_back(cb.lightcolor(linecolor[6][3]));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.lightcolor(fillcolor[6][3]));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);

  linecolor[8].push_back(cb.lightcolor(linecolor[7][3]));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.lightcolor(fillcolor[7][3]));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

  // 5th scanner
  markerstyle.push_back(22);
  markersize.push_back(1.1);

  linecolor[0].push_back(kOrange+3);
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(kOrange-7);
  fillstyle[0].push_back(3013);
  linewidth[0].push_back(2);

  linecolor[1].push_back(kOrange-1);
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(kOrange-6);
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);

  linecolor[2].push_back(kOrange-2);
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(kOrange-9);
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);

  linecolor[3].push_back(cb.lightcolor(linecolor[2][4]));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.lightcolor(fillcolor[2][4]));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);

  linecolor[4].push_back(cb.lightcolor(linecolor[3][4]));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.lightcolor(fillcolor[3][4]));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);

  linecolor[5].push_back(cb.lightcolor(linecolor[4][4]));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.lightcolor(fillcolor[4][4]));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);

  linecolor[6].push_back(cb.lightcolor(linecolor[5][4]));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.lightcolor(fillcolor[5][4]));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);

  linecolor[7].push_back(cb.lightcolor(linecolor[6][4]));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.lightcolor(fillcolor[6][4]));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);

  linecolor[8].push_back(cb.lightcolor(linecolor[7][4]));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.lightcolor(fillcolor[7][4]));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

  // scanners 6-16
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

  // new colors from Alison
  // 16-18
  makeNewPlotStyle("#9e9ac8"); // shades of purple
  makeNewPlotStyle("#54278f");
  makeNewPlotStyle("#3f007d");

  // 19-21
  makeNewPlotStyle("#fb6a4a"); // shades of red
  makeNewPlotStyle("#ef3b2c");
  makeNewPlotStyle("#cb181d");

  //22-24
  makeNewPlotStyle("#a1d99b"); // shades of green
  makeNewPlotStyle("#41ab5d");
  makeNewPlotStyle("#238b45");

  //25-27
  makeNewPlotStyle("#ffeda0"); // shades of orange
  makeNewPlotStyle("#fed976");
  makeNewPlotStyle("#feb24c");

  //28-30
  makeNewPlotStyle("#9ecae1"); // shades of blue
  makeNewPlotStyle("#4292c6");
  makeNewPlotStyle("#2171b5");

  // some other colors (should we just change this to take the color hex string directly?)
	// 31-36
  makeOneColorPlotStyle("#bdbdbd"); // gray
  makeOneColorPlotStyle("#969696"); // dark gray
  makeOneColorPlotStyle("#525252"); // darker gray
  makeOneColorPlotStyle("#252525"); // black
  makeOneColorPlotStyle("#9c1216"); // dark red
  makeOneColorPlotStyle("#9e9ac8"); // blue

	// the color blind safe option from the paper
	// 37-42
	makeNewPlotStyle("#74add1"); // 37 light blue
	makeNewPlotStyle("#f46d43"); // 38 coral
	makeNewPlotStyle("#fdae61"); // 39 orangey
	makeNewPlotStyle("#d73027"); // 40 red
	makeNewPlotStyle("#4575b4"); // 41 dark blue
	makeNewPlotStyle("#fee090"); // 42 yellow

  // any additional scanners
  for ( int i=fillcolor[0].size(); i<arg->combid.size(); i++ ) {
    makeNewPlotStyle("",kBlue-7);
  }

  // if requested, add or remove any fill pattern to make cleaner plots
  for ( int iContour=0; iContour<fillstyle.size(); iContour++ ) {
    for ( int iScanner=0; iScanner<fillstyle[iContour].size(); iScanner++ ) {
      // remove any fill style at all
      if ( arg->isQuickhack(10) ) fillstyle[iContour][iScanner] = 1001;
      if ( iScanner < arg->fillstyle.size() ) fillstyle[iContour][iScanner] = arg->fillstyle[iScanner];
    }
  }
  for ( int iContour=0; iContour<linewidth.size(); iContour++ ){
    for ( int iScanner=0; iScanner<linewidth[iContour].size(); iScanner++ ) {
      if ( iScanner < arg->linewidth.size() ) linewidth[iContour][iScanner] = arg->linewidth[iScanner];
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
  linewidth[0].push_back(2);
  linecolor[1].push_back(cb.darklightcolor(linecolor[0][currentNumberOfStyles],thisMuchDarker));
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(cb.darklightcolor(fillcolor[0][currentNumberOfStyles],thisMuchDarker));
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);
  linecolor[2].push_back(cb.darklightcolor(linecolor[1][currentNumberOfStyles],thisMuchDarker));
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(cb.darklightcolor(fillcolor[1][currentNumberOfStyles],thisMuchDarker));
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);
  linecolor[3].push_back(cb.darklightcolor(linecolor[2][currentNumberOfStyles],thisMuchDarker));
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.darklightcolor(fillcolor[2][currentNumberOfStyles],thisMuchDarker));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);
  linecolor[4].push_back(cb.darklightcolor(linecolor[3][currentNumberOfStyles],thisMuchDarker));
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.darklightcolor(fillcolor[3][currentNumberOfStyles],thisMuchDarker));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);
  linecolor[5].push_back(cb.darklightcolor(linecolor[4][currentNumberOfStyles],thisMuchDarker));
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.darklightcolor(fillcolor[4][currentNumberOfStyles],thisMuchDarker));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);
  linecolor[6].push_back(cb.darklightcolor(linecolor[5][currentNumberOfStyles],thisMuchDarker));
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.darklightcolor(fillcolor[5][currentNumberOfStyles],thisMuchDarker));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);
  linecolor[7].push_back(cb.darklightcolor(linecolor[6][currentNumberOfStyles],thisMuchDarker));
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.darklightcolor(fillcolor[6][currentNumberOfStyles],thisMuchDarker));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);
  linecolor[8].push_back(cb.darklightcolor(linecolor[7][currentNumberOfStyles],thisMuchDarker));
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.darklightcolor(fillcolor[7][currentNumberOfStyles],thisMuchDarker));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

}

void OneMinusClPlot2d::makeOneColorPlotStyle(TString htmlColor, int ROOTColor)
{

  int currentNumberOfStyles = linecolor[0].size();
  // get index of new color. Either use the provided HTML color, or
  // take a predefined ROOT color.
  int newColor;
  if ( htmlColor.EqualTo("ROOT") ) newColor = currentNumberOfStyles;
  else if ( ROOTColor > 0 ) newColor = ROOTColor;
  else newColor = TColor::GetColor(htmlColor);
  ColorBuilder cb;
  float thisMuchLighter = 1.2;
  markerstyle.push_back(20);
  markersize.push_back(1.1);
  linecolor[0].push_back(newColor);
  linestyle[0].push_back(kSolid);
  fillcolor[0].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[0].push_back(3005);
  linewidth[0].push_back(2);
  linecolor[1].push_back(newColor);
  linestyle[1].push_back(kSolid);
  fillcolor[1].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[1].push_back(1001);
  linewidth[1].push_back(2);
  linecolor[2].push_back(newColor);
  linestyle[2].push_back(kSolid);
  fillcolor[2].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[2].push_back(1001);
  linewidth[2].push_back(2);
  linecolor[3].push_back(newColor);
  linestyle[3].push_back(kSolid);
  fillcolor[3].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[3].push_back(1001);
  linewidth[3].push_back(2);
  linecolor[4].push_back(newColor);
  linestyle[4].push_back(kSolid);
  fillcolor[4].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[4].push_back(1001);
  linewidth[4].push_back(2);
  linecolor[5].push_back(newColor);
  linestyle[5].push_back(kSolid);
  fillcolor[5].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[5].push_back(1001);
  linewidth[5].push_back(2);
  linecolor[6].push_back(newColor);
  linestyle[6].push_back(kSolid);
  fillcolor[6].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[6].push_back(1001);
  linewidth[6].push_back(2);
  linecolor[7].push_back(newColor);
  linestyle[7].push_back(kSolid);
  fillcolor[7].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[7].push_back(1001);
  linewidth[7].push_back(2);
  linecolor[8].push_back(newColor);
  linestyle[8].push_back(kSolid);
  fillcolor[8].push_back(cb.darklightcolor(newColor,thisMuchLighter));
  fillstyle[8].push_back(1001);
  linewidth[8].push_back(2);

}

///
/// Add a scanner to the list of things to be plotted.
///
void OneMinusClPlot2d::addScanner(MethodAbsScan* s, int CLsType)
{
  if ( arg->debug ) cout << "OneMinusClPlot2d::addScanner() : adding " << s->getName() << endl;
  if ((CLsType==1 || CLsType==2) && !s->getHCLs2d()){
    cout << "OneMinusClPlot2d::addScanner() : ERROR : No hCLs available. Will not plot." << endl;
    return;
  }
  scanners.push_back(s);
  if ( (s->getMethodName().EqualTo("Prob") || s->getMethodName().EqualTo("DatasetsProb")) && CLsType==0){
    histosType.push_back(kChi2);
    histos.push_back(s->getHchisq2d());
  }
  else {
    histosType.push_back(kPvalue);
    if(CLsType==1 ||CLsType==2) 	histos.push_back(s->getHCLs2d());
    else 		histos.push_back(s->getHCL2d());
  }
  do_CLs.push_back(CLsType);
  if ( arg->smooth2d ) for ( int i=0; i<arg->nsmooth; i++ ) { histos[histos.size()-1]->Smooth(); }
  title = s->getTitle();
  if(CLsType==1 || CLsType==2) title = s->getTitle() + " CLs";
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
  if (arg->square) yLow = 0.11;
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
    m_mainCanvas = newNoWarnTCanvas(name+getUniqueRootName(), title, 800, arg->square ? 800 : 600);
  }
  m_mainCanvas->cd();
  m_mainCanvas->SetMargin(0.1,0.15,0.1,0.1);
  if(histos.size()==0) std::cout << "OneMinusClPlot2d::DrawFull() : No histogram to plot!" << std::endl;
  if(!histos[0]) std::cout << "OneMinusClPlot2d::DrawFull() : Histogram broken!" << std::endl;
  TH2F *hChi2 = histos[0];
  hChi2->SetContour(95);
  hChi2->GetXaxis()->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle());
  hChi2->GetYaxis()->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle());
  float zMin = hChi2->GetMinimum();
  float zMax;
  if ( histosType[0]==kChi2 ){
    zMax = fmin(zMin+81, hChi2->GetMaximum());
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
  vector<int> linecolor { kRed, kRed, kRed, kRed, kRed, kRed, kRed };
  //vector<int> linestyle { kDashed, kDashed, kDashed, kDashed, kDashed };
  vector<int> linestyle { 1, 2, 3, 4, 5, 6, 7 };
  TColor *col = gROOT->GetColor(0);
  col->SetAlpha(1.);
  vector<int> fillcolor { 0, 0, 0, 0, 0, 0, 0 };
  vector<int> fillstyle { 0, 0, 0, 0, 0, 0, 0 };
  vector<int> linewidth { 2, 2, 2, 2, 2, 2, 2 };
  cont->setStyle( linecolor, linestyle, linewidth, fillcolor, fillstyle );
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
  m_legend->SetNColumns( arg->plotlegcols );
  m_legend->SetFillColor(0);
  m_legend->SetFillStyle(0);
  m_legend->SetBorderSize(0);
  if (arg->isQuickhack(35)) {
    m_legend->SetFillColorAlpha(0,0.7);
    m_legend->SetFillStyle(1001);
    m_legend->SetLineStyle(1);
    m_legend->SetLineWidth(1);
    m_legend->SetLineColor(kGray+1);
    m_legend->SetBorderSize(1);
  }
  m_legend->SetTextFont(font);
  m_legend->SetTextSize(legendsize*0.75);
  if ( arg->isQuickhack(26) ) m_legend->SetTextSize(0.9*legendsize);

  // build legend
  for ( int i = 0; i < histos.size(); i++ ){
    TString legTitle = scanners[i]->getTitle();
    if ( legTitle=="default" ) {
      if ( do_CLs[i] ) legTitle = "Prob CLs";
      else legTitle = "Prob";
    }
    else if ( do_CLs[i] ) legTitle += " CLs";

    if ( histos.size()==1 ){
      // no legend symbol if only one scanner to be drawn
      m_legend->AddEntry((TObject*)0, legTitle, "");
    }
    else{
      // construct a dummy TGraph that uses the style of the 1sigma line
      int styleId = i;
      if ( arg->color.size()>i ) styleId = arg->color[i];
      TGraph *g = new TGraph(1);
      if ( arg->isQuickhack(33) ) g->SetFillStyle(1001); // solid
      else g->SetFillStyle(fillstyle[0][i]);
      g->SetFillColor(fillcolor[0][styleId]);
      g->SetLineColor(linecolor[0][styleId]);
      g->SetLineStyle(linestyle[0][i]);
      g->SetLineWidth(linewidth[0][i]);
      if ( styleId < arg->filltransparency.size() ) g->SetFillColorAlpha( fillcolor[0][styleId], 1.-arg->filltransparency[styleId] );
      g->SetMarkerColor(linecolor[1][styleId]);
      g->SetMarkerStyle(markerstyle[styleId]);
      g->SetMarkerSize(markersize[styleId]);
      TString options = "f";
      if ( scanners[i]->getDrawSolution() ) options += "p"; // only plot marker symbol when solutions are plotted
      if ( scanners[i]->getTitle() != "noleg" ) {
        m_legend->AddEntry(g, legTitle, options );
      }
    }
  }
  if ( arg->plotlegbox ) {
    TPaveText *legbox = new TPaveText(legendXmin,legendYmin,legendXmin+arg->plotlegboxx,legendYmin+arg->plotlegboxy,"ndc");
    legbox->SetFillColorAlpha(0,0.7);
    legbox->SetFillStyle(1001);
    legbox->SetLineStyle(1);
    legbox->SetLineWidth(1);
    legbox->SetLineColor(kGray+1);
    legbox->SetBorderSize(1);
    legbox->SetShadowColor(0);
    legbox->Draw();
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
    m_mainCanvas = newNoWarnTCanvas(name+getUniqueRootName(), title, 800, arg->square ? 800 : 600);
    // put this in for exponent xaxes
    if ( !arg->isQuickhack(30) ) m_mainCanvas->SetRightMargin(0.1);
    // put this in for exponent yaxes
    if ( !arg->isQuickhack(30) ) m_mainCanvas->SetTopMargin(0.07);
    cout << m_mainCanvas->GetLeftMargin() << " " << m_mainCanvas->GetBottomMargin() << endl;
    if ( arg->square ) m_mainCanvas->SetBottomMargin(0.14);
  }

  if ( arg->isQuickhack(14) ){
    m_mainCanvas->GetPad(0)->SetLeftMargin(0.16);
  }

  if (arg->grid) m_mainCanvas->SetGrid();

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
  if (arg->square) haxes->GetYaxis()->SetTitleOffset(1.);
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
  // for the grid
  if (arg->grid) {
    haxes->GetXaxis()->SetAxisColor(15);
    haxes->GetYaxis()->SetAxisColor(15);
  }
  haxes->Draw();

  // draw origin if requested
  TLine *lOrig;
  if ( arg->plotoriginx>-99 ) {
    lOrig = new TLine();
    lOrig->DrawLine( arg->plotoriginx, arg->scanrangeyMin, arg->plotoriginx, arg->scanrangeyMax );
  }
  if ( arg->plotoriginy>-99 ) {
    lOrig = new TLine();
    lOrig->DrawLine( arg->scanrangeMin, arg->plotoriginy, arg->scanrangeMax, arg->plotoriginy );
  }

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
		//cout << i << " " << styleId << " " << linecolor[0][styleId] << " " << linestyle[0][i] << endl;
    cont->setStyle(transpose(linecolor)[styleId], transpose(linestyle)[i], transpose(linewidth)[i],transpose(fillcolor)[styleId], transpose(fillstyle)[i]);
    if (i<arg->filltransparency.size()) cont->setTransparency( arg->filltransparency[i] );
    cont->setContoursToPlot( arg->contourlabels[i] );
    m_contours[i] = cont;
    m_contours_computed[i] = true;
  }
  m_mainCanvas->cd(); // ConfidenceContours::computeContours() creates a temporary canvas

  // overwrite transparency if requested
  if ( arg->isQuickhack(12) ){
    for ( int i=0; i < histos.size(); i++ ){
      m_contours[i]->setTransparency(0.2);
      if ( arg->isQuickhack(28)  ) m_contours[i]->setTransparency(0.5);
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
  if (!arg->grid) {
    haxes->GetXaxis()->SetNdivisions(0);  // disable old axis
    haxes->GetYaxis()->SetNdivisions(0);
  }
  else {
    haxes->GetXaxis()->SetTitleSize(0);
    haxes->GetYaxis()->SetTitleSize(0);
    haxes->GetXaxis()->SetLabelSize(0);
    haxes->GetYaxis()->SetLabelSize(0);
    haxes->GetXaxis()->SetTickLength(0);
    haxes->GetYaxis()->SetTickLength(0);

    m_mainCanvas->RedrawAxis();
  }

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
  if ( isAngle(scanners[0]->getScanVar1()) || arg->isQuickhack(36) ){
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
    axisb->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle() + TString(" [#circ]"));
    axisb->SetTitleOffset(0.8);
    axisb->SetTitleSize(titlesize);
    axisb->SetTitleFont(font);
    axisb->Draw();
  }
  else
  {
    // add top axis
		double nmin = xmin;
		double nmax = xmax;
		if (arg->isQuickhack(38)){
		 nmin *= 100;
		 nmax *= 100;
		}
    TGaxis *axist = new TGaxis(xmin, ymax, xmax, ymax, nmin, nmax, xndiv, xtchopt);
    axist->SetName("axist");
    axist->Draw();

    // draw a new bottom axis because the confidence contours can cover the old one
    TGaxis *axisb = new TGaxis(xmin, ymin, xmax, ymin, nmin, nmax, xndiv, xbchopt);
    axisb->SetName("axisb");
    axisb->SetLabelOffset(haxes->GetXaxis()->GetLabelOffset());
    axisb->SetLabelFont(font);
    axisb->SetLabelSize(labelsize);
    axisb->SetTitle(xTitle!="" ? xTitle : (TString)scanners[0]->getScanVar1()->GetTitle());
    axisb->SetTitleOffset(0.8);
    axisb->SetTitleSize(titlesize);
    axisb->SetTitleFont(font);
    axisb->Draw();
  }

  // New Y axis. For angles in units of degrees.
  if ( isAngle(scanners[0]->getScanVar2()) || arg->isQuickhack(37) ){
    haxes->GetYaxis()->SetTitle(haxes->GetYaxis()->GetTitle() + TString(" [#circ]"));

    // new left axis
    TGaxis *axisl = new TGaxis(xmin, ymin, xmin, ymax, RadToDeg(ymin), RadToDeg(ymax), yndiv, ylchopt);
    axisl->SetName("axisl");
    axisl->SetLabelOffset(haxes->GetYaxis()->GetLabelOffset());
    axisl->SetLabelFont(font);
    axisl->SetLabelSize(labelsize);
    axisl->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle() + TString(" [#circ]"));
    axisl->SetTitleOffset(0.9);
    if (arg->square) axisl->SetTitleOffset(1.2);
    axisl->SetTitleSize(titlesize);
    axisl->SetTitleFont(font);
    axisl->Draw();

    // new axis for the right ticks
    TGaxis *axisr = new TGaxis(xmax, ymin, xmax, ymax, RadToDeg(ymin), RadToDeg(ymax), yndiv, yrchopt);
    axisr->SetName("axisr");
    axisr->Draw();
  }
  else
  {
    // new left axis (so that is not covered by the plot)
		double nmin = ymin;
		double nmax = ymax;
		if (arg->isQuickhack(39)){
		 nmin *= 100;
		 nmax *= 100;
		}
    haxes->GetYaxis()->SetNdivisions(0);
    TGaxis *axisl = new TGaxis(xmin, ymin, xmin, ymax, nmin, nmax, yndiv, ylchopt);
    axisl->SetName("axisl");
    axisl->SetLabelOffset(haxes->GetYaxis()->GetLabelOffset());
    axisl->SetLabelFont(font);
    axisl->SetLabelSize(labelsize);
    axisl->SetTitle(yTitle!="" ? yTitle : (TString)scanners[0]->getScanVar2()->GetTitle());
    axisl->SetTitleOffset(0.8);
    if (arg->square) axisl->SetTitleOffset(1.2);
    axisl->SetTitleSize(titlesize);
    axisl->SetTitleFont(font);
    axisl->Draw();

    // new right axis
    TGaxis *axisr = new TGaxis(xmax, ymin, xmax, ymax, nmin, nmax, yndiv, yrchopt);
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
    if ( scanners[i]->getSolutions().size()==0 ){
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

    for ( int iSol=0; iSol<scanners[i]->getSolutions().size(); iSol++ )
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

