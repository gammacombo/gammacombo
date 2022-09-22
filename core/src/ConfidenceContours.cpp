#include "ConfidenceContours.h"

ConfidenceContours::ConfidenceContours(OptParser *arg)
{
    assert(arg);
    m_arg = arg;
    m_transparency = 0.;
    m_nMaxContours = 9;
}

ConfidenceContours::~ConfidenceContours()
{}

///
/// Helper function for computeContours():
/// Constructs a new 2D histogram that contains 2 more bins in each
/// direction that are set to the minimum, so that the contours will always close.
///
/// \param hist - the 2D histogram
/// \return new 2D histogram. Caller assumes ownership.
///
//TH2F* ConfidenceContours::addBoundaryBins(TH2F* hist)
//{
    //float boundary = hist->GetMinimum();
    //TH2F* hBoundaries = new TH2F(getUniqueRootName(),getUniqueRootName(),
            //hist->GetNbinsX()+4,
            //hist->GetXaxis()->GetXmin() - 2*hist->GetXaxis()->GetBinWidth(1),
            //hist->GetXaxis()->GetXmax() + 2*hist->GetXaxis()->GetBinWidth(1),
            //hist->GetNbinsY()+4,
            //hist->GetYaxis()->GetXmin() - 2*hist->GetYaxis()->GetBinWidth(1),
            //hist->GetYaxis()->GetXmax() + 2*hist->GetYaxis()->GetBinWidth(1));
    //for ( int ix=1; ix<=hBoundaries->GetXaxis()->GetNbins(); ix++ ){
        //for ( int iy=1; iy<=hBoundaries->GetYaxis()->GetNbins(); iy++ ){
            //// fill outmost extra bins with the boundary value
            //if ( ix==1 || ix==hBoundaries->GetXaxis()->GetNbins()
              //|| iy==1 || iy==hBoundaries->GetYaxis()->GetNbins() )
                //hBoundaries->SetBinContent(ix,iy,boundary);
            //// fill the outer 4 bins of the inner extra bins with the boundary value
            //else if ( (ix==2                                     && iy==hBoundaries->GetYaxis()->GetNbins()-1)
                    //||(ix==hBoundaries->GetXaxis()->GetNbins()-1 && iy==hBoundaries->GetYaxis()->GetNbins()-1)
                    //||(ix==2                                     && iy==2)
                    //||(ix==hBoundaries->GetXaxis()->GetNbins()-1 && iy==2) ){
                //hBoundaries->SetBinContent(ix,iy,boundary);
            //}
            //// fill the rest of the inner bins with the adjacent values of the original histogram
            //else if ( ix==2 && (iy>=3 || iy<=hBoundaries->GetYaxis()->GetNbins()-2) ){
                //hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-1,iy-2));
            //}
            //else if ( ix==hBoundaries->GetXaxis()->GetNbins()-1 && (iy>=3 || iy<=hBoundaries->GetYaxis()->GetNbins()-2) ){
                //hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-3,iy-2));
            //}
            //else if ( iy==2 && (ix>=3 || ix<=hBoundaries->GetXaxis()->GetNbins()-2) ){
                //hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-2,iy-1));
            //}
            //else if ( iy==hBoundaries->GetYaxis()->GetNbins()-1 && (ix>=3 || ix<=hBoundaries->GetXaxis()->GetNbins()-2) ){
                //hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-2,iy-3));
            //}
            //// copy the inner part from the original histogram
            //else{
                //hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-2,iy-2));
            //}
        //}
    //}
    //return hBoundaries;
//}
TH2F* ConfidenceContours::addBoundaryBins(TH2F* hist)
{
    float boundary = hist->GetMinimum();
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
                hBoundaries->SetBinContent(ix,iy,boundary);
            else{
                hBoundaries->SetBinContent(ix,iy,hist->GetBinContent(ix-1,iy-1));
            }
        }
    }
    return hBoundaries;
}

///
/// Helper function for computeContours():
/// Transforms the chi2 valley into a hill to help ROOTs contour mechanism
/// Caller assumes ownership.
///
/// \param hist - the 2D histogram
/// \param offset - a chi2 offset, usually 30 units
/// \return - the transformed 2D histogram
///
TH2F* ConfidenceContours::transformChi2valleyToHill(TH2F* hist, float offset)
{
    float chi2min = hist->GetMinimum();
    //cout << "ConfidenceContours::transformChi2valleyToHill() : chi2min=" << chi2min << endl;
    TH2F* newHist = histHardCopy(hist, false, true);
    for ( int ix=1; ix<=hist->GetXaxis()->GetNbins(); ix++ ){
        for ( int iy=1; iy<=hist->GetYaxis()->GetNbins(); iy++ ){
            newHist->SetBinContent(ix,iy,-hist->GetBinContent(ix,iy)+offset+chi2min);
        }
    }
    return newHist;
}

///
/// Add a new contour that is just the plotted area.
///
/// \param hist - histogram providing the dimensions of the plotted area
///
void ConfidenceContours::addFilledPlotArea(TH2F* hist)
{
    // get boundaries
    float xmin = hist->GetXaxis()->GetXmin();
    float xmax = hist->GetXaxis()->GetXmax();
    float ymin = hist->GetYaxis()->GetXmin();
    float ymax = hist->GetYaxis()->GetXmax();
    // make new graph covering the plotted area
    TGraph *g = new TGraph(m_nMaxContours);
    g->SetPoint(0, xmin, ymin);
    g->SetPoint(1, xmin, ymax);
    g->SetPoint(2, xmax, ymax);
    g->SetPoint(3, xmax, ymin);
  for ( int i=4; i<m_nMaxContours; i++ ) g->SetPoint(i, xmin, ymin);
    // make a new Contour object from it
    TList *l = new TList();
    l->Add(g);
    Contour *c = new Contour(m_arg, l);
    m_contours.push_back(c);
}

///
/// Compute the raw N sigma confidence contours from a 2D histogram
/// holding either the chi2 or the p-value curve. The resulting
/// contours are stored into the m_contours member.
///
/// \param hist - the 2D histogram
/// \param type - the type of the 2D histogram, either chi2 or p-value
///
void ConfidenceContours::computeContours(TH2F* hist, histogramType type, int id)
{
    if ( m_arg->debug ) {
        cout << "ConfidenceContours::computeContours() : making contours of histogram ";
        cout << hist->GetName();
        cout << ", type " << (type==kChi2?"chi2":"p-value") << endl;
    }
    // clean up contours from a previous call
    m_contours.clear();

    // transform chi2 from valley to hill
    float offset = 100.;
    if ( type==kChi2 ) hist = transformChi2valleyToHill(hist,offset);

    // add boundaries
    TH2F* histb = addBoundaryBins(hist);

    // make contours
    histb->SetContour(m_nMaxContours);
    if ( type==kChi2 ) {
        // chi2 units
        if ( m_arg->plot2dcl[id]>0 ){
            for ( int i=0;i <m_nMaxContours; i++ ) {
              int cLev = m_nMaxContours-1-i;
              // hack for >= 9 when ROOT precision fails
              if (i==8)       histb->SetContourLevel( cLev, offset - 83.9733 ); // 9 sigma
              else if (i==9)  histb->SetContourLevel( cLev, offset - 99.2688 ); // 10 sigma
              else if (i==10) histb->SetContourLevel( cLev, offset - 114.564 ); // 11 sigma
              else            histb->SetContourLevel( cLev, offset - TMath::ChisquareQuantile( 1.-TMath::Prob( (i+1)*(i+1), 1), 2) );
            }
        }
        else{
            for ( int i=0;i <m_nMaxContours; i++ ) {
              int cLev = m_nMaxContours-1-i;
              histb->SetContourLevel( cLev, offset - (i+1)*(i+1) );
            }
        }
    }
    else {
        // p-value units
        if ( m_arg->plot2dcl[id]>0 ){
            for ( int i=0;i <m_nMaxContours; i++ ) {
              int cLev = m_nMaxContours-1-i;
              histb->SetContourLevel( cLev, TMath::Prob( (i+1)*(i+1), 1) );
            }
        }
        else{
            for ( int i=0;i <m_nMaxContours; i++ ) {
              int cLev = m_nMaxContours-1-i;
              histb->SetContourLevel( cLev, TMath::Prob( (i+1)*(i+1), 2) );
            }
        }
    }

    // create and access the contours
    if ( m_arg->interactive ) gROOT->SetBatch(true); // don't display the temporary canvas
    TCanvas *ctmp = newNoWarnTCanvas(getUniqueRootName(), "ctmp");
    histb->Draw("contlist");
    gPad->Update(); // needed to be able to access the contours as TGraphs
    TObjArray *contours = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
    delete ctmp;
    delete histb;
    if ( m_arg->interactive ) gROOT->SetBatch(false); // it's important to only unset batch mode if we're interactive! Else some canvases get screwed up badly resulting in corrupted PDF files.

    // access contours. They get filled in reverse order,
    // and depend on how many are actually present. If all 5
    // are filled, index 0 is 5sigma. If only 2 are filled, index 0
    // is 2 sigma.
    int nEmptyContours = 0;
    for ( int ic=m_nMaxContours-1; ic>=0; ic-- ){
        if (((TList*)contours->At(ic))->IsEmpty()) nEmptyContours++;
    }
    for ( int ic=m_nMaxContours-1; ic>=0; ic-- ){
        if ( !(((TList*)contours->At(ic))->IsEmpty()) ){
            Contour* cont = new Contour(m_arg, (TList*)contours->At(ic));
            cont->setSigma(5-nEmptyContours-ic);
            m_contours.push_back(cont);
        }
    }

    // add the entire plotted area, if one requested contour
    // is empty, i.e. it contains the entire plot range
    if ( nEmptyContours>0 ){
        addFilledPlotArea(hist);
    }

    // magnetic boundaries
    if ( m_arg->plotmagnetic ) {
        for ( int ic=m_nMaxContours-1; ic>=0; ic-- ){
            if ( ic>=m_contours.size() ) continue;
            m_contours[ic]->magneticBoundaries(hist);
        }
    }

    // clean up
    if ( type==kChi2 ) delete hist; // a copy was made earlier in this case
}

///
/// Draw the contours into the currently active Canvas.
///
void ConfidenceContours::Draw()
{
    //cout << "ConfidenceContours::Draw() : drawing ..." << endl;
    if ( m_contstoplots.size()>0 ) {
        for ( int ind=m_contstoplots.size()-1; ind>=0; ind-- ) {
            int i = m_contstoplots[ind]-1;
            if ( i>=m_contours.size() ) continue;
            m_contours[i]->setStyle(m_linecolor[i], m_linestyle[i], m_linewidth[i], m_fillcolor[i], m_fillstyle[i]);
            m_contours[i]->setTransparency(m_transparency);
            m_contours[i]->Draw();
        }
    }
    else {
        for ( int i=m_arg->plotnsigmacont-1; i>=0; i-- ) {
            if ( i>=m_contours.size() ) continue;
            m_contours[i]->setStyle(m_linecolor[i], m_linestyle[i], m_linewidth[i], m_fillcolor[i], m_fillstyle[i]);
            m_contours[i]->setTransparency(m_transparency);
            m_contours[i]->Draw();
        }
    }
}

///
/// Draw the contours into the currently active Canvas.
///
void ConfidenceContours::DrawDashedLine()
{
    //cout << "ConfidenceContours::DrawDashedLine() : drawing ..." << endl;
    if ( m_contstoplots.size()>0 ) {
        for ( int ind=m_contstoplots.size()-1; ind>=0; ind-- ) {
            int i = m_contstoplots[ind]-1;
            if ( i>=m_contours.size() ) continue;
            m_contours[i]->setStyle(m_linecolor[i], m_linestyle[i], m_linewidth[i], m_fillcolor[i], m_fillstyle[i]);
            m_contours[i]->setTransparency(m_transparency);
            m_contours[i]->Draw();
        }
    }
    else {
        for ( int i=m_arg->plotnsigmacont-1; i>=0; i-- ) {
            if ( i>=m_contours.size() ) continue;
            m_contours[i]->setStyle(m_linecolor[i], kDashed, m_linewidth[i], 0, 0);
            m_contours[i]->DrawLine();
        }
    }
}

///
/// Set the contour style.
///
void ConfidenceContours::setStyle(vector<int>& linecolor, vector<int>& linestyle, vector<int>& linewidth, vector<int>& fillcolor, vector<int>& fillstyle)
{
    m_linecolor = linecolor;
    m_linestyle = linestyle;
    m_fillcolor = fillcolor;
    m_fillstyle = fillstyle;
    m_linewidth = linewidth;
    //for ( int i=0; i<m_linestyle.size(); i++ ) m_linewidth.push_back(2);
    // check if enough styles where given for the number of contours to be plotted
    if ( m_arg->plotnsigmacont > m_linestyle.size() ){
        cout << "ConfidenceContours::setStyle() : ERROR : not enough sigma contour styles defined! ";
        cout << "Reusing style of " << m_linestyle.size() << " sigma contour." << endl;
        for ( int i=m_linestyle.size(); i<m_arg->plotnsigmacont; i++ ){
            int laststyle = m_linestyle.size()-1;
            if ( laststyle<0 ){
                cout << "ConfidenceContours::setStyle() : ERROR : linestyle is empty. Exit." << endl;
                exit(1);
            }
            m_linecolor.push_back(m_linecolor[laststyle]);
            m_linestyle.push_back(m_linestyle[laststyle]);
            m_fillcolor.push_back(m_fillcolor[laststyle]);
            m_fillstyle.push_back(m_fillstyle[laststyle]);
            m_linewidth.push_back(m_linewidth[laststyle]);
        }
    }
}
