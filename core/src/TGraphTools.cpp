#include "TGraphTools.h"

TGraph* TGraphTools::addPointToGraphAtFirstMatchingX(TGraph* g, float xNew, float yNew)
{
	// get x and y coordinates as vectors- the TGraph interface is just not suited to
	// what we want to do
	vector<float> xVec;
	vector<float> yVec;
	Double_t xOld, yOld;
	for ( int i=0; i<g->GetN(); i++ ){
		g->GetPoint(i, xOld, yOld);
		xVec.push_back(xOld);
		yVec.push_back(yOld);
	}

	// find position to insert: before the first x value that is larger than xNew
	int iPos = xVec.size();
	for ( int i=0; i<xVec.size(); i++ ){
		if ( xNew <= xVec[i] ){
			iPos = i;
			break;
		}
	}

	// insert
	xVec.insert(xVec.begin()+iPos, xNew);
	yVec.insert(yVec.begin()+iPos, yNew);

	// create a new graph of the right kind
	bool isTGraphErrors = TString(g->ClassName()).EqualTo("TGraphErrors");
	TGraph *gNew;
	if ( isTGraphErrors ) gNew = new TGraphErrors(g->GetN()+1);
	else                  gNew = new TGraph(g->GetN()+1);
	gNew->SetName(getUniqueRootName());

	// set the points
	for ( int i=0; i<xVec.size(); i++ ){
		gNew->SetPoint(i, xVec[i], yVec[i]);
	}

	// set the errors, if necessary
	if ( isTGraphErrors ){
		for ( int i=0; i<xVec.size(); i++ ){
			if ( i < iPos ){
				((TGraphErrors*)gNew)->SetPointError(i, 0.0, g->GetErrorY(i));
			}
			else if ( i == iPos ){
				((TGraphErrors*)gNew)->SetPointError(i, 0.0, 0.0);
			}
			else{
				((TGraphErrors*)gNew)->SetPointError(i, 0.0, g->GetErrorY(i-1));
			}
		}
	}
	return gNew;
}
