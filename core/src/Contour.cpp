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
}

Contour::~Contour()
{
	for ( int i=0; i<m_contours.size(); i++ ){
		delete m_contours[i];
	}
}

///
/// Draw the contours into the currenlty active Canvas.
///
void Contour::Draw()
{
	for ( int i=0; i<m_contours.size(); i++ )
	{
		m_contours[i]->Draw();
	}
}
