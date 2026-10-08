#include <BatchScriptWriter.h>

#include <Combiner.h>
#include <FileNameBuilder.h>
#include <OptParser.h>
#include <PDF_Abs.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {
  /// Quote an argument for the shell if needed.
  std::string shellQuote(const std::string& s) {
    const std::string safe = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_@%+=:,./-";
    if (!s.empty() && s.find_first_not_of(safe) == std::string::npos) return s;
    std::string quoted = "'";
    for (const char ch : s) quoted += ch == '\'' ? std::string("'\\''") : std::string(1, ch);
    return quoted + "'";
  }
}  // namespace

BatchScriptWriter::BatchScriptWriter(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    // skip arguments we dont need
    if (std::string(argv[i]) == std::string("--nbatchjobs") ||
        (i > 0 && std::string(argv[i - 1]) == std::string("--nbatchjobs")) ||
        std::string(argv[i]) == std::string("--batchstartn") ||
        (i > 0 && std::string(argv[i - 1]) == std::string("--batchstartn")) ||
        std::string(argv[i]) == std::string("--batcheos") || std::string(argv[i]) == std::string("-i")) {
      continue;
    }
    exec += shellQuote(argv[i]) + " ";
  }
  subpkg = std::string(argv[0]);
}

void BatchScriptWriter::writeScripts(const OptParser* arg, std::vector<Combiner*>* cmb, const FileNameBuilder* fnb) {

  for (int i = 0; i < arg->combid.size(); i++) {
    int combinerId = arg->combid[i];
    Combiner* c = cmb->at(combinerId);
    c->combine();
    if (!c->isCombined()) continue;

    TString methodname = "";
    if (arg->isAction("pluginbatch")) {
      methodname = "Plugin";
    } else if (arg->isAction("coveragebatch")) {
      methodname = "Coverage";
    } else {
      std::cout << "BatchScriptWriter::writeScripts() : ERROR : only need to write batch scripts for pluginbatch method"
                << std::endl;
      std::exit(1);
    }
    if (arg->isAction("uniform")) methodname += "Uniform";
    if (arg->isAction("gaus")) methodname += "Gaus";

    std::cout << "Writing submission scripts for combination " << c->getName() << std::endl;

    // same as MethodAbsScan::toyFilenameBase
    const TString combname = fnb->getCombinerFileName(c);
    TString dirname = "scan1d" + methodname + "_" + combname + "_" + arg->var[0];
    if (arg->var.size() == 2) { dirname = "scan2d" + methodname + "_" + combname + "_" + arg->var[0]; }
    if (arg->var.size() > 1) { dirname += "_" + arg->var[1]; }
    if (arg->isAction("coveragebatch")) { dirname += arg->id < 0 ? "_id0" : Form("_id%d", arg->id); }

    char cwd[1024];
    getcwd(cwd, 1024);

    TString scripts_dir_path = "sub/" + dirname;
    TString outf_dir = TString(cwd) + "/root/" + dirname;
    system(Form("mkdir -p %s", scripts_dir_path.Data()));
    TString scriptname = "scan1d" + methodname + "_" + combname + "_" + arg->var[0];
    if (arg->isAction("coveragebatch")) { scriptname += arg->id < 0 ? "_id0" : Form("_id%d", arg->id); }
    // if write to eos then make the directory
    if (arg->batchout != "" || arg->batcheos) {
      time_t t = time(0);
      struct tm* now = localtime(&t);
      int day = now->tm_mday;
      int month = now->tm_mon + 1;
      int year = now->tm_year + 1900;

      if (arg->batcheos) {
        std::string name(getenv("USER"));
        char initial = name[0];
        std::cout << "Name: " << name << " first: " << initial << std::endl;
        TString eos_path =
            Form("/eos/lhcb/user/%c/%s/gammacombo/%02d%02d%04d", initial, name.c_str(), day, month, year);
        // TString eos_path = Form("/eos/lhcb/user/t/tmombach/gammacombo/%02d%02d%04d",day,month,year);
        // system(Form("/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select mkdir %s",eos_path.Data()));
        eos_path += Form("/%s", dirname.Data());
        // system(Form("/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select mkdir -p %s",eos_path.Data()));
        outf_dir = eos_path;
      } else if (arg->batchout) {
        outf_dir = Form("%s/%02d%02d%04d/%s", arg->batchout.Data(), day, month, year, dirname.Data());
      }
    }
    if (arg->var.size() == 2) { scriptname = "scan2d" + methodname + "_" + combname + "_" + arg->var[0]; }
    if (arg->var.size() > 1) { scriptname += "_" + arg->var[1]; }
    scriptname = scripts_dir_path + "/" + scriptname;

    // write out a list of the submission files
    std::ofstream subfilelist;
    subfilelist.open(scriptname + "_sublist.txt");
    for (int job = arg->batchstartn; job < arg->batchstartn + arg->nbatchjobs; job++) {
      TString fname = scriptname + Form("_run%d", job) + ".sh";
      subfilelist << fname << std::endl;
      writeScript(fname, outf_dir, job, arg);
    }
    subfilelist.close();
    std::cout << "Written submission file list to\n\t" << scriptname << "_sublist.txt" << std::endl;
    writeCondorScript(scriptname, arg);
  }
}

// Copy the same submission script with minor differences for the datasets option
// -> only difference is that the NLL is stored in the pdf already and does not have to be combined
// -> no combiners for the datasets option
void BatchScriptWriter::writeScripts_datasets(const OptParser* arg, PDF_Abs* pdf) {

  if (!pdf) {
    std::cout << "BatchScriptWriter::writeScripts_datasets(): ERROR: No PDF given " << std::endl;
    std::exit(1);
  }

  TString methodname = "";
  if (arg->isAction("pluginbatch")) {
    methodname = "Plugin";
  } else if (arg->isAction("coveragebatch")) {
    methodname = "Coverage";
  } else {
    std::cout << "BatchScriptWriter::writeScripts() : ERROR : only need to write batch scripts for pluginbatch method"
              << std::endl;
    std::exit(1);
  }
  if (arg->isAction("uniform")) methodname += "Uniform";
  if (arg->isAction("gaus")) methodname += "Gaus";

  std::cout << "Writing submission scripts for PDF " << pdf->getName() << std::endl;

  TString dirname = "scan1dDatasets" + methodname + "_" + pdf->getName() + "_" + arg->var[0];
  if (arg->var.size() == 2) { dirname = "scan2dDatasets" + methodname + "_" + pdf->getName() + "_" + arg->var[0]; }
  if (arg->var.size() > 1) { dirname += "_" + arg->var[1]; }
  if (arg->isAction("coveragebatch")) { dirname += arg->id < 0 ? "_id0" : Form("_id%d", arg->id); }
  TString scripts_dir_path = "sub/" + dirname;
  TString outf_dir = "root/" + dirname;
  system(Form("mkdir -p %s", scripts_dir_path.Data()));
  TString scriptname = "scan1dDatasets" + methodname + "_" + pdf->getName() + "_" + arg->var[0];
  if (arg->isAction("coveragebatch")) { scriptname += arg->id < 0 ? "_id0" : Form("_id%d", arg->id); }
  // if write to eos then make the directory
  if (arg->batcheos) {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    int day = now->tm_mday;
    int month = now->tm_mon + 1;
    int year = now->tm_year + 1900;

    std::string name(getenv("USER"));
    char initial = name[0];
    std::cout << "Name: " << name << " first: " << initial << std::endl;
    TString eos_path = Form("/eos/lhcb/user/%c/%s/gammacombo/%02d%02d%04d", initial, name.c_str(), day, month, year);
    // TString eos_path = Form("/eos/lhcb/user/t/tmombach/gammacombo/%02d%02d%04d",day,month,year);
    // system(Form("/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select mkdir %s",eos_path.Data()));
    // system(Form("mkdir %s",eos_path.Data()));
    eos_path += Form("/%s", dirname.Data());
    // system(Form("/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select mkdir -p %s",eos_path.Data()));
    //  system(Form("mkdir -p %s",eos_path.Data()));
    outf_dir = eos_path;
  }
  if (arg->var.size() == 2) { scriptname = "scan2dDatasets" + methodname + "_" + pdf->getName() + "_" + arg->var[0]; }
  if (arg->var.size() > 1) { scriptname += "_" + arg->var[1]; }
  scriptname = scripts_dir_path + "/" + scriptname;

  // write out a list of the submission files
  std::ofstream subfilelist;
  subfilelist.open(scriptname + "_sublist.txt");
  for (int job = arg->batchstartn; job < arg->batchstartn + arg->nbatchjobs; job++) {
    TString fname = scriptname + Form("_run%d", job) + ".sh";
    subfilelist << fname << std::endl;
    writeScript(fname, outf_dir, job, arg);
  }
  subfilelist.close();
  writeCondorScript(scriptname, arg);
}

void BatchScriptWriter::writeCondorScript(TString fname, const OptParser* arg) {

  TString subfilename = fname + ".sub";
  // std::cout << "\t" << subfilename << std::endl;
  std::ofstream outfile;
  outfile.open(subfilename);

  char cwd[1024];
  getcwd(cwd, 1024);

  outfile << "##### auto-generated by BatchScriptWriter #####" << std::endl;
  outfile << "executable = $(subfile)" << std::endl;
  outfile << "arguments = " << std::endl;
  // add specific requirements if requested
  if (arg->batchreqs != "") {
    std::ifstream infile(arg->batchreqs.Data());
    if (infile) {
      std::string line;
      if (infile.is_open()) {
        while (getline(infile, line)) {
          if (line.empty()) continue;
          outfile << line << std::endl;
        }
      }
    }
  }

  outfile << "output = $(subfile).out" << std::endl;
  outfile << "error = $(subfile).err" << std::endl;
  outfile << Form("log = %s.log", fname.Data()) << std::endl;
  outfile << Form("queue subfile from %s_sublist.txt", fname.Data()) << std::endl;
  outfile.close();

  system(Form("chmod +x %s", subfilename.Data()));

  std::cout << "Written condor submission script to\n\t" << subfilename << std::endl;

  if (arg->batchsubmit) {
    // std::cout << Form("condor_submit %s/%s",cwd,subfilename.Data()) << std::endl;
    system(Form("condor_submit %s/%s", cwd, subfilename.Data()));
  }
}

void BatchScriptWriter::writeScript(TString fname, TString outfloc, int jobn, const OptParser* arg) {

  // sub/<dir>/<name>.sh -> root/<dir>/<name>.root
  TString rootfilename = fname;
  rootfilename.Replace(0, 3, "root");
  rootfilename.Replace(rootfilename.Length() - 3, 3, ".root");
  std::cout << "\t" << fname << std::endl;
  std::ofstream outfile;
  outfile.open(fname);

  char cwd[1024];
  getcwd(cwd, 1024);

  outfile << "#!/bin/bash" << std::endl;
  outfile << "##### auto-generated by BatchScriptWriter #####" << std::endl;
  outfile << Form("rm -f %s/%s.done", cwd, fname.Data()) << std::endl;
  outfile << Form("rm -f %s/%s.fail", cwd, fname.Data()) << std::endl;
  outfile << Form("rm -f %s/%s.run", cwd, fname.Data()) << std::endl;
  outfile << Form("rm -f %s/%s.log", cwd, fname.Data()) << std::endl;
  outfile << "mkdir -p scratch" << std::endl;
  outfile << "cd scratch" << std::endl;
  outfile << Form("source %s/../scripts/setup_lxplus.sh", cwd) << std::endl;
  outfile << Form("cp -r %s/ExpNll .", cwd) << std::endl;
  outfile << "mkdir -p bin" << std::endl;
  outfile << Form("cp %s/%s bin/", cwd, subpkg.c_str()) << std::endl;
  outfile << "mkdir -p plots/dot" << std::endl;
  outfile << Form("cp -r %s/plots/dot/* plots/dot", cwd) << std::endl;
  outfile << "mkdir -p plots/par" << std::endl;
  outfile << Form("cp -r %s/plots/par/* plots/par", cwd) << std::endl;
  outfile << "mkdir -p plots/scanner" << std::endl;
  outfile << "mkdir -p root" << std::endl;
  outfile << Form("touch %s/%s.run", cwd, fname.Data()) << std::endl;
  outfile << Form("if ( %s --nrun %d ); then", exec.c_str(), jobn) << std::endl;
  outfile << Form("\trm -f %s/%s.run", cwd, fname.Data()) << std::endl;
  outfile << Form("\ttouch %s/%s.jobcomplete", cwd, fname.Data()) << std::endl;

  TString basename = rootfilename;
  basename.Remove(0, rootfilename.Last('/') + 1);
  system(Form("mkdir -p %s", outfloc.Data()));
  TString copy_line = Form("cp %s %s/%s", rootfilename.Data(), outfloc.Data(), basename.Data());
  if (arg->batcheos)
    copy_line = Form("xrdcp -p %s root://eoslhcb.cern.ch/%s/%s", rootfilename.Data(), outfloc.Data(), basename.Data());

  outfile << Form("\techo \"Copying file to %s/%s\"", outfloc.Data(), basename.Data()) << std::endl;
  outfile << Form("\tif ( %s ); then", copy_line.Data()) << std::endl;
  outfile << Form("\t\ttouch %s/%s.done", cwd, fname.Data()) << std::endl;
  outfile << "\t\techo \"SUCCESS!\"" << std::endl;
  outfile << "\telse" << std::endl;
  outfile << Form("\t\ttouch %s/%s.fail", cwd, fname.Data()) << std::endl;
  outfile << Form("\t\trm -f %s/%s.run", cwd, fname.Data()) << std::endl;
  outfile << "\tfi" << std::endl;

  outfile << "else" << std::endl;
  outfile << Form("\ttouch %s/%s.fail", cwd, fname.Data()) << std::endl;
  outfile << Form("\trm -f %s/%s.run", cwd, fname.Data()) << std::endl;
  outfile << "fi" << std::endl;
  outfile.close();

  system(Form("chmod +x %s", fname.Data()));
}
