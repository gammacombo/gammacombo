#include "Graphviz.h"

Graphviz::Graphviz(OptParser *arg)
{
  assert(arg);
  this->arg = arg;
}


Graphviz::~Graphviz()
{}

///
/// Convert a string so that it compatible with the graphviz
/// syntax for element names such as nodes or edges.
///
TString Graphviz::graphvizString(TString s)
{
  s.ReplaceAll("-","_");
  return s;
}

bool Graphviz::isDmixingParameter(TString s)
{
  if ( s==TString("xD")
    || s==TString("yD") ) return true;
  return false;
}

///
/// Open a file, return the file handle.
///
ofstream& Graphviz::openFile(TString name)
{
  ofstream* dotfile = new ofstream();
  dotfile->open(name);
  if (!dotfile->is_open()) {
    cout << "Graphviz::openFile() : ERROR : Could not open file. " << name << endl;
    exit(1);
  }
  return *dotfile;
}

///
/// Print the parameter/measurement structure of a Combiner
/// into a *.dot file. The file is saved into the plots/dot
/// directory. To create a graph from it, run
///
/// dot -Tpdf plots/dot/dkdpiRunning.dot -o dkdpiRunning.pdf
///
void Graphviz::printCombiner(Combiner* cmb)
{
  // open the dot file
  ofstream& dotfile = openFile("plots/dot/circle_"+cmb->getName()+".dot");

  // print header
  dotfile << "graph combiner {\n";
  dotfile << "graph [label=\"" << graphvizString(cmb->getTitle()) << "\", labelloc=t, fontsize=30];\n";
  dotfile << "layout=circo;\n";
  dotfile << "ranksep=5;\n";
  //dotfile << "K=3;\n";

  // print measurements (=nodes)
  for ( int i=0; i<cmb->getPdfs().size(); i++ ){
    TString nodeName = graphvizString(cmb->getPdfs()[i]->getName());
    TString nodeTitle = graphvizString(cmb->getPdfs()[i]->getTitle());
    dotfile << nodeName << " [label=\"" << nodeTitle << "\"];\n";
  }

  // print shared parameters (=edges)
  for ( int i=0; i<cmb->getPdfs().size(); i++ ){
    TString nodeNamei = graphvizString(cmb->getPdfs()[i]->getName());

    // loop over parameters of pdf i
    TIterator* iti = cmb->getPdfs()[i]->getParameters()->createIterator();
    while ( RooAbsReal* vi = (RooAbsReal*)iti->Next() ){

      // check if a parameter is shared with pdf j
      for ( int j=i+1; j<cmb->getPdfs().size(); j++ ){
        TString nodeNamej = graphvizString(cmb->getPdfs()[j]->getName());

        // print edges
        TIterator* itj = cmb->getPdfs()[j]->getParameters()->createIterator();
        while ( RooAbsReal* vj = (RooAbsReal*)itj->Next() ){
          if ( TString(vi->GetName())==TString(vj->GetName()) ){
            dotfile << nodeNamei << " -- " << nodeNamej << " ";
            dotfile << "[label=\""<< vi->GetName() << "\"";
            // define edge colors
            if ( TString(vi->GetName())==TString("g") ) dotfile << ",color=red";
            if ( isDmixingParameter(vi->GetName()) ) dotfile << ",color=blue";
            dotfile << "];\n";
          }
        }
        delete itj;
      }
    }
    delete iti;
  }

  // print footer
  dotfile << "}\n";
  dotfile.close();
}

///
/// Print the parameter/measurement structure of a Combiner
/// into a *.dot file. The file is saved into the plots/dot
/// directory. To create a graph from it, run
///
/// dot -Tpdf plots/dot/dkdpiRunning.dot -o dkdpiRunning.pdf
///
void Graphviz::printCombinerLayer(Combiner* cmb)
{
  // open the dot file
  ofstream& dotfile = openFile("plots/dot/layer_"+cmb->getName()+".dot");

  // print header
  dotfile << "graph combiner {\n";
  dotfile << "graph [label=\"" << graphvizString(cmb->getTitle()) << "\", labelloc=t, fontsize=30];\n";
  dotfile << "layout=dot;\n";
  dotfile << "rankdir=LR;\n";
  dotfile << "ranksep=5;\n";
  //dotfile << "K=3;\n";

  // print measurements (=nodes)
  dotfile << "subgraph cluster0 {\n";
  dotfile << "node [style=filled,color=white];\n";
  dotfile << "style=filled;\n";
  dotfile << "color=lightgrey;\n";
  for ( int i=0; i<cmb->getPdfs().size(); i++ ){
    TString nodeName = graphvizString(cmb->getPdfs()[i]->getName());
    TString nodeTitle = graphvizString(cmb->getPdfs()[i]->getTitle());
    dotfile << nodeName << " [label=\"" << nodeTitle << "\"];\n";
  }
  dotfile << "label=\"measurements\";\n";
  dotfile << "}\n";

  // print parameters (=nodes)
  dotfile << "subgraph cluster1 {\n";
  dotfile << "node [style=filled,color=white];\n";
  dotfile << "style=filled;\n";
  dotfile << "color=lightgrey;\n";
  vector<string>& pars = cmb->getParameterNames();
  for ( int i=0; i<pars.size(); i++ ){
      dotfile << graphvizString(pars[i]) << ";\n";
  }
  dotfile << "label=\"parameters\";\n";
  dotfile << "}\n";

  // print edges
  for ( int i=0; i<cmb->getPdfs().size(); i++ ){
    TString nodeNamePdf = graphvizString(cmb->getPdfs()[i]->getName());

    // loop over parameters of pdf i
    TIterator* it = cmb->getPdfs()[i]->getParameters()->createIterator();
    while ( RooAbsReal* vi = (RooAbsReal*)it->Next() ){
      TString nodeNamePar = graphvizString(vi->GetName());
      dotfile << nodeNamePdf << " -- " << nodeNamePar;
      dotfile << "[";
      // define edge colors
      if ( nodeNamePar==TString("g") ) dotfile << "color=red";
      else dotfile << "color=black";
      dotfile << "];\n";
    }
    delete it;
  }

  // print footer
  dotfile << "}\n";
  dotfile.close();
}
