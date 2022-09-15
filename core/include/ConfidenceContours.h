/**
 * Gamma Combination
 * Author: Till Moritz Karbach, moritz.karbach@cern.ch
 * Date: Feb 2015
 *
 * Class holding the 1-N sigma confidence contours.
 *
 **/

#ifndef ConfidenceContours_h
#define ConfidenceContours_h

#include "TROOT.h"
#include "OptParser.h"
#include "Contour.h"
#include "Utils.h"

using namespace std;
using namespace Utils;

class ConfidenceContours
{
    public:

        ConfidenceContours(OptParser *arg);
        ~ConfidenceContours();
        void        computeContours(TH2F* hist, histogramType type, int id=0);
        void        Draw();
        void        DrawDashedLine();
        void        setStyle(vector<int>& linecolor, vector<int>& linestyle, vector<int>& linewidth, vector<int>& fillcolor, vector<int>& fillstyle);
        inline void setTransparency(float percent){m_transparency = percent;};
        inline void setContoursToPlot( vector<int>& contstoplot ){ m_contstoplots = contstoplot;};

    private:

        TH2F*            addBoundaryBins(TH2F* hist);
        void             addFilledPlotArea(TH2F* hist);
        TH2F*            transformChi2valleyToHill(TH2F* hist,float offset);
        OptParser*       m_arg;       ///< command line arguments
        vector<Contour*> m_contours;  ///< container for the 1,...,N sigma contours
        vector<int>      m_linecolor; ///< style for the 1,...,N sigma contours
        vector<int>      m_linestyle;
        vector<int>      m_fillcolor;
        vector<int>      m_fillstyle;
        vector<int>      m_linewidth;
        float            m_transparency;
        vector<int>      m_contstoplots; ///< container for which contours to actually draw
        int              m_nMaxContours;
};

#endif
