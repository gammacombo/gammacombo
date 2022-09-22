#include "FitResultDump.h"
#include "Utils.h"

using namespace std;

FitResultDump::FitResultDump(){}
FitResultDump::~FitResultDump(){}

void FitResultDump::dumpResult(string ofname, MethodAbsScan *scanner){

    ofname = "plots/par/"+ofname+".dat";
    system("mkdir -p plots/par");

    cout << "FitResultDump::dumpResult() : saving " << ofname << endl;

    ofstream outf;
    outf.open(ofname.c_str());
    outf << "# Fit Result Summary" << endl;
    outf << "nSolutions=" << scanner->getSolutions().size() << endl;
    outf << "# pvalue central min max" << endl;

    bool angle=false;
    if (isAngle(scanner->getWorkspace()->var(scanner->getScanVar1Name()))) angle=true;

    for (vector<CLInterval>::iterator cl=scanner->clintervals1sigma.begin(); cl!=scanner->clintervals1sigma.end(); cl++){

        float central = cl->central;
        float min = cl->min;
        float max = cl->max;
        if (angle){
            central = RadToDeg(central);
            min = RadToDeg(min);
            max = RadToDeg(max);
        }

        outf << cl->pvalue << " " << central << " " << min << " " << max << endl;
    }
    for (vector<CLInterval>::iterator cl=scanner->clintervals2sigma.begin(); cl!=scanner->clintervals2sigma.end(); cl++){

        float central = cl->central;
        float min = cl->min;
        float max = cl->max;
        if (angle){
            central = RadToDeg(central);
            min = RadToDeg(min);
            max = RadToDeg(max);
        }

        outf << cl->pvalue << " " << central << " " << min << " " << max << endl;
    }

    outf.close();
}
