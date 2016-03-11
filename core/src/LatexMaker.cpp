#include "LatexMaker.h"

using namespace std;

LatexMaker::LatexMaker( TString cName, PDF_Abs *_pdf ):
  pdf(_pdf)
{
  system("mkdir -p plots/latex");
  outfname = "plots/latex/" + cName + "_" + pdf->getName();
}

LatexMaker::~LatexMaker(){}

void LatexMaker::writeFile()
{

  // central values and errors
  ofstream outfile;
  outfile.open(outfname + ".tex");

  RooArgList *observables = pdf->getObservables();
  vector<TString> labels  = pdf->getLatexObservables();

  outfile << "\\begin{alignat}{3}" << endl;
  for ( int i=0; i<pdf->getNobs(); i++ ) {

    RooRealVar *var = (RooRealVar*)observables->at(i);

    TString title = var->GetTitle();
    title.ReplaceAll("#","\\");

    if ( i < labels.size() ) title = labels[i];

    if ( var->getVal() < 0. ) {
      outfile << Form("& %-20s  & \\;=\\;            %7.5f  \\;\\pm\\; & %7.5f \\;\\pm\\; & %7.5f \\nonumber", title.Data(), var->getVal(), pdf->StatErr[i], pdf->SystErr[i]);
    }
    else {
      outfile << Form("& %-20s  & \\;=\\;  \\phantom{-}%7.5f  \\;\\pm\\; & %7.5f \\;\\pm\\; & %7.5f \\nonumber", title.Data(), var->getVal(), pdf->StatErr[i], pdf->SystErr[i]);
    }
    if ( i<pdf->getNobs()-1 ) {
      outfile << " \\\\" << endl;
    }
    else {
      outfile << endl;
    }

  }
  outfile << "\\end{alignat}" << endl;

  outfile.close();

  // stat correlations
  ofstream outfile_stat;
  outfile_stat.open(outfname + "_stat.tex");
  writeCorrMatrix( outfile_stat, pdf->corStatMatrix, observables, labels );
  outfile_stat.close();

  // syst correlations
  ofstream outfile_syst;
  outfile_syst.open(outfname + "_syst.tex");
  writeCorrMatrix( outfile_syst, pdf->corSystMatrix, observables, labels );
  outfile_syst.close();

}

void LatexMaker::writeCorrMatrix( ofstream& file, TMatrixDSym mat, RooArgList *observables, vector<TString> labels ) {

  file << "\\begin{tabular}{ l |";
  for ( int i=0; i < mat.GetNcols(); i++) file << "c";
  file << "}" << endl;
  file << "\\hline" << endl;
  file << "\\hline" << endl;
  file << Form("%-15s","");

  for ( int i=0; i < mat.GetNcols(); i++) {
    TString title = observables->at(i)->GetTitle();
    title.ReplaceAll("#","\\");
    if ( i < labels.size() ) title = labels[i];
    file << Form(" & %s",title.Data());
  }
  file << "\\\\" << endl;
  file << "\\hline" << endl;
  for ( int i=0; i < mat.GetNrows(); i++) {

    TString title = observables->at(i)->GetTitle();
    title.ReplaceAll("#","\\");
    if ( i < labels.size() ) title = labels[i];
    file << Form("%-15s",title.Data());

    for (int j=0; j < mat.GetNcols(); j++) {
      if ( TMath::Abs(mat[i][j]-1) < 1.e-3 ) {
        file << " &       1";
      }
      else if ( TMath::Abs(mat[i][j]) < 1.e-3 ) {
        file << " &       0";
      }
      else if ( mat[i][j] < 0 ) {
        file << Form(" &  %5.3f",mat[i][j]);
      }
      else if ( mat[i][j] > 0 ) {
        file << Form(" &   %5.3f",mat[i][j]);
      }
      else {
        file << " &       0";
      }
    }
    file << "  \\\\" << endl;

  }
  file << "\\hline" << endl;
  file << "\\hline" << endl;

  file << "\\end{tabular}" << endl;

}
