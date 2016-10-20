#include "OptParser.h"

///
/// Organize command line options.
/// The strategy is to have one object of this class present in the
/// main program, which then gets passed down to all other objects.
/// The command line options are made available through data members
/// of this class.
/// Options can be booked, either in blocks using the respective functions,
/// or individually. Only booked options get parsed from the command line.
///
/// To add a new option,
/// - add a data member
/// - add its initializer to the constructor (vectors don't need initializers).
/// - add its name to defineOptions()
/// - add its definition and parsing to parseArguments()
///
OptParser::OptParser():
  cmd("", ' ', "")
{
	defineOptions();

	// always book these options
	bookedOptions.push_back("debug");
	bookedOptions.push_back("interactive");
	bookedOptions.push_back("usage");
	bookedOptions.push_back("var");
	bookedOptions.push_back("verbose");

	// Initialize the variables.
	// For more complex arguments these are also the default values.
	controlplot = false;
	coverageCorrectionID = 0;
	coverageCorrectionPoint = 0;
	debug = false;
	digits = -99;
	enforcePhysRange = false;
    filenamechange = "";
	group = "GammaCombo";
	groupPos = "";
  hfagLabel = "";
  hfagLabelPos = "";
	id = -99;
	importance = false;
  info = false;
	interactive = false;
	jobdir = ".";
	largest = false;
  latex = false;
  plotlegstyle = "default";
	lightfiles = false;
  batchstartn = 1;
  nbatchjobs = -99;
  batcheos = false;
	nBBpoints = -99;
	ndiv = 407;
	ndivy = 407;
	nosyst = false;
	npoints1d = -99;
	npoints2dx = -99;
	npoints2dy = -99;
	npointstoy = -99;
  ncoveragetoys = -99;
	nrun = -99;
	ntoys = -99;
	nsmooth = 1;
	parevol = false;
  plotext = "";
	plotid = -99;
	plotlegend = true;
	plotlegx = -99;
	plotlegy = -99;
	plotlegsizex = -99;
	plotlegsizey = -99;
	plotgroupx = -99;
	plotgroupy = -99;
  plotHFAGLabelPosX = 0;
  plotHFAGLabelPosY = 0;
  plotHFAGLabelScale = 1;
	plotlog = false;
	plotmagnetic = false;
	plotnsigmacont = 2;
	plotpluginonly = false;
	plotprelim = false;
	plotpulls = false;
	plotunoff = false;
  plotymin = -99.;
  plotymax = -99.;
	pluginPlotRangeMax = -100;
	pluginPlotRangeMin = -100;
	intprob = false;
	probforce = false;
	probimprove = false;
	printcor = false;
  printSolX = -999.;
  printSolY = -999.;
  queue = "";
  save = "";
	scanforce = false;
	scanrangeMax = -101;
	scanrangeMin = -101;
	scanrangeyMax = -102;
	scanrangeyMin = -102;
	smooth2d = false;
  toyFiles = "";
	usage = false;
	verbose = false;
}

///
/// Define the names of all available command line options.
/// Other places of the code will check against this list.
///
void OptParser::defineOptions()
{
	availableOptions.push_back("action");
	availableOptions.push_back("asimov");
	availableOptions.push_back("asimovfile");
  availableOptions.push_back("batchstartn");
  availableOptions.push_back("batcheos");
	availableOptions.push_back("combid");
	availableOptions.push_back("color");
	availableOptions.push_back("controlplots");
	availableOptions.push_back("covCorrect");
	availableOptions.push_back("covCorrectPoint");
	availableOptions.push_back("debug");
	availableOptions.push_back("digits");
	availableOptions.push_back("evol");
  availableOptions.push_back("filename");
  availableOptions.push_back("fillstyle");
	availableOptions.push_back("fix");
	availableOptions.push_back("ext");
  availableOptions.push_back("hfagLabel");
  availableOptions.push_back("hfagLabelPos");
	availableOptions.push_back("id");
	availableOptions.push_back("importance");
  availableOptions.push_back("info");
	availableOptions.push_back("interactive");
	//availableOptions.push_back("jobdir");
	availableOptions.push_back("jobs");
	availableOptions.push_back("largest");
  availableOptions.push_back("latex");
	availableOptions.push_back("leg");
	availableOptions.push_back("legsize");
  availableOptions.push_back("legstyle");
	availableOptions.push_back("group");
	availableOptions.push_back("grouppos");
	availableOptions.push_back("lightfiles");
	availableOptions.push_back("loadParamsFile");
	availableOptions.push_back("log");
	availableOptions.push_back("magnetic");
  availableOptions.push_back("nbatchjobs");
  //availableOptions.push_back("nBBpoints");
	availableOptions.push_back("nosyst");
	availableOptions.push_back("npoints");
	availableOptions.push_back("npoints2dx");
	availableOptions.push_back("npoints2dy");
	availableOptions.push_back("npointstoy");
	availableOptions.push_back("ncoveragetoys");
	availableOptions.push_back("nrun");
	availableOptions.push_back("ntoys");
	availableOptions.push_back("nsmooth");
	//availableOptions.push_back("pevid");
	availableOptions.push_back("pr");
	availableOptions.push_back("physrange");
  availableOptions.push_back("plotext");
	availableOptions.push_back("plotid");
  availableOptions.push_back("plotrangey");
	availableOptions.push_back("intprob");
	availableOptions.push_back("po");
	availableOptions.push_back("prelim");
  availableOptions.push_back("printsolx");
  availableOptions.push_back("printsoly");
	availableOptions.push_back("probforce");
	//availableOptions.push_back("probimprove");
	availableOptions.push_back("ps");
	availableOptions.push_back("pulls");
	availableOptions.push_back("qh");
  availableOptions.push_back("queue");
  availableOptions.push_back("randomizeToyVars");
  availableOptions.push_back("removeRange");
  availableOptions.push_back("save");
	availableOptions.push_back("sn");
	availableOptions.push_back("sn2d");
	availableOptions.push_back("scanforce");
	availableOptions.push_back("scanrange");
	availableOptions.push_back("scanrangey");
	availableOptions.push_back("smooth2d");
  availableOptions.push_back("toyFiles");
	availableOptions.push_back("title");
	availableOptions.push_back("usage");
	availableOptions.push_back("unoff");
	availableOptions.push_back("var");
	availableOptions.push_back("verbose");
	//availableOptions.push_back("relation");
	availableOptions.push_back("pluginplotrange");
	availableOptions.push_back("plotnsigmacont");
	availableOptions.push_back("plot2dcl");
	availableOptions.push_back("ndiv");
	availableOptions.push_back("ndivy");
	availableOptions.push_back("printcor");
}

///
/// Book all available options, if you are lazy.
///
void OptParser::bookAllOptions()
{
	for ( int i=0; i<availableOptions.size(); i++ )
		bookedOptions.push_back(availableOptions[i]);
}

///
/// Book options associated to plotting.
///
void OptParser::bookPlottingOptions()
{
	bookedOptions.push_back("color");
	bookedOptions.push_back("digits");
	bookedOptions.push_back("ext");
	bookedOptions.push_back("leg");
	bookedOptions.push_back("legsize");
	bookedOptions.push_back("group");
	bookedOptions.push_back("grouppos");
	bookedOptions.push_back("log");
	bookedOptions.push_back("magnetic");
	bookedOptions.push_back("prelim");
	bookedOptions.push_back("ps");
	bookedOptions.push_back("plotnsigmacont");
	bookedOptions.push_back("plot2dcl");
	bookedOptions.push_back("ndiv");
	bookedOptions.push_back("ndivy");
	bookedOptions.push_back("title");
	bookedOptions.push_back("unoff");
}

///
/// Book options associated to the Plugin method
/// and the Berger-Boos method (batch job handling,
/// control plots).
///
void OptParser::bookPluginOptions()
{
  bookedOptions.push_back("batchstartn");
  bookedOptions.push_back("batcheos");
  bookedOptions.push_back("controlplots");
	bookedOptions.push_back("id");
	bookedOptions.push_back("importance");
	bookedOptions.push_back("jobs");
	bookedOptions.push_back("lightfiles");
  bookedOptions.push_back("nbatchjobs");
	//bookedOptions.push_back("nBBpoints");
	bookedOptions.push_back("npointstoy");
	bookedOptions.push_back("nrun");
	bookedOptions.push_back("ntoys");
	bookedOptions.push_back("nsmooth");
	//bookedOptions.push_back("pevid");
	bookedOptions.push_back("pr");
	bookedOptions.push_back("physrange");
	bookedOptions.push_back("intprob");
	bookedOptions.push_back("po");
	bookedOptions.push_back("pluginplotrange");
}

///
/// Book options associated to the Prob method.
///
void OptParser::bookProbOptions()
{
	bookedOptions.push_back("asimov");
	bookedOptions.push_back("asimovfile");
	bookedOptions.push_back("evol");
	bookedOptions.push_back("npoints");
	bookedOptions.push_back("npoints2dx");
	bookedOptions.push_back("npoints2dy");
	bookedOptions.push_back("pr");
	bookedOptions.push_back("physrange");
	bookedOptions.push_back("sn");
	bookedOptions.push_back("sn2d");
	bookedOptions.push_back("probforce");
	//bookedOptions.push_back("probimprove");
	bookedOptions.push_back("pulls");
	bookedOptions.push_back("scanforce");
	bookedOptions.push_back("scanforce");
}

///
/// Book options associated to the flow control.
///
void OptParser::bookFlowcontrolOptions()
{
	bookedOptions.push_back("action");
	bookedOptions.push_back("combid");
	bookedOptions.push_back("fix");
	//bookedOptions.push_back("jobdir");
	bookedOptions.push_back("nosyst");
}

///
/// Book the given option. Checks if it is available.
/// \param opt Book this option.
///
void OptParser::bookOption(TString opt)
{
	if ( !isIn<TString>(availableOptions, opt) ){
		cout << "OptParser::bookOption() : ERROR : No such option! Check OptParser::defineOptions()." << endl;
		return;
	}
	bookedOptions.push_back(opt);
}

///
/// Check the --action argument. Was action 's' given?
///
bool OptParser::isAction(TString s)
{
	return isIn<TString>(action, s);
}

///
/// Check the --quickhack argument. Was hack 'id' given?
///
bool OptParser::isQuickhack(int id)
{
	return isIn<int>(qh, id);
}

///
/// Parse the command line for booked options. Then apply some
/// post-processing and checks, where necessary. Save the parsed
/// values into the datamembers. If an option was not given or not booked,
/// it will assume the default value given here.
///
void OptParser::parseArguments(int argc, char* argv[])
{
  //CmdLine cmd("", ' ', "");
  //cmd = CmdLine("", ' ', "");

	// --------------- arguments that take a value
	TCLAP::ValueArg<string> scanrangeArg("", "scanrange", "Restrict the scan range to a given range. "
			"In 2D scans, this corresponds to the x variable. "
			"Format: --scanrange min:max.", false, "default", "string");
	TCLAP::ValueArg<string> scanrangeyArg("", "scanrangey", "For 2D plots, restrict the scan range "
			"of the y variable to a given range. "
			"Format: --scanrangey min:max.", false, "default", "string");
  TCLAP::ValueArg<string> plotrangeyArg("", "plotrangey", "Plot range of the y-axis for 1D plots. Default 0:1. For log plots 1.e-3:1. "
      "Format: --plotrangey min:max.",false, "default", "string");
	TCLAP::ValueArg<int> ndivArg("", "ndiv", "Set the number of axis divisions (x axis in 1d and 2d plots): "
			"ndiv=N1 + 100*N2 + 10000*N3, "
			"N1 = number of 1st divisions (N2 = 2nd, N3 = 3rd). Default is 407. To enable bin optimization, pre-pend "
			"a '-'.", false, -1, "int");
	TCLAP::ValueArg<int> coverageCorrectionIDArg("","covCorrect","Correct for the coverage given a flat transformation of the pvalue distribution. You can also pass a coverage correction point (see --covCorrectPoint)\n"
			"0: no correction (default)\n"
			"1: linear correction\n"
			"2: linear + exponential correction\n"
			"3: linear + 1/x correction\n",
			false, 0, "int");
	TCLAP::ValueArg<int> coverageCorrectionPointArg("","covCorrectPoint","Point to use for coverage correction",false,0,"int");
	TCLAP::ValueArg<int> ndivyArg("", "ndivy", "Set the number of axis divisions (y axis in 2d plots): "
			"ndivy=N1 + 100*N2 + 10000*N3, "
			"N1 = number of 1st divisions (N2 = 2nd, N3 = 3rd). Default is 407. To enable bin optimization, pre-pend "
			"a '-'.", false, -1, "int");
	TCLAP::ValueArg<int> plotidArg("p", "plotid", "Make control plot with given ID (see --controlplots). "
			"Available IDs are 1-6. If not given, all control plots are made.", false, 0, "int");
	TCLAP::ValueArg<int> digitsArg("s", "digits", "Set the number of printed"
			" digits right of the decimal point. Default is automatic.", false, -1, "int");
  TCLAP::ValueArg<string> plotextArg("", "plotext", "Add an extension to the output plot name.",false, "","string");
	TCLAP::ValueArg<string> plotlegArg("", "leg", "Adjust the plot legend.\n"
			"Disable the legend with --leg off .\n"
			"2d plots: set the position of the legend. "
			"Format: --leg xmin:ymin in normalized coordinates [0,1]. Default: 0.17:0.75", false, "default", "string");
	TCLAP::ValueArg<string> plotlegsizeArg("", "legsize", "Adjust the plot legend size.\n"
			"2d plots: set the size of the legend. "
			"Format: --legsize xsize:ysize in normalized coordinates [0,1]. Default: 0.38:0.15", false, "default", "string");
  TCLAP::ValueArg<string> plotlegstyleArg("", "legstyle", "Change the legend style.", false, "default", "string");
	TCLAP::ValueArg<string> pluginplotrangeArg("", "pluginplotrange", "Restrict the Plugin plot to a given range to "
			"rejcet low-statistics outliers. Format: --pluginplotrange min-max.", false, "default", "string");
	TCLAP::ValueArg<int> plotnsigmacontArg("", "ncontours", "plot this many sigma contours in 2d plots (max 5)", false, 2, "int");
	TCLAP::ValueArg<string> filenameadditionArg("","ext","Add this piece into the file name (in case you don't want files/plots to be overwritten", false, "", "string");
  TCLAP::ValueArg<string> filenamechangeArg("","filename", "Change filename to this name (after the basename of the executable)", false, "", "string");
  TCLAP::ValueArg<string> hfagLabelArg("", "hfagLabel", "Use the HFAG label with a name (e.g. ICHEP 2016). Passing \'default\' gives the HFAG label with no subname", false, "", "string");
  TCLAP::ValueArg<string> hfagLabelPosArg("", "hfagLabelPos", "Set the position and scale of the HFAG logo. "
      "Format: --hfagLabelPos xpos:ypos:scale in noramlised coordinates [0,1]. To use default values "
      "for one coordinate, use 'def': --hfagLabelPos def:y:def", false, "default", "string");
	TCLAP::ValueArg<string> plotgroupArg("", "group", "Set the group logo. Use '--group off' to disable the logo. "
			"See also --grouppos. Default: GammaCombo", false, "GammaCombo", "string");
	TCLAP::ValueArg<string> plotgroupposArg("", "grouppos", "Set the position of the group logo. "
			"Format: --grouppos xmin:ymin in normalized coordinates [0,1]. To use default values "
			"for one coordinate, use 'def': --grouppos def:y.", false, "default", "string");
  TCLAP::ValueArg<float> printSolXArg("","printsolx", "x coordinate to print solution at in 1D plots", false, -999., "float");
  TCLAP::ValueArg<float> printSolYArg("","printsoly", "y coordinate to shift solution by in 1D plots", false, -999., "float");
  TCLAP::ValueArg<string> queueArg("q","queue","Batch queue to submit to. If none is given then the scripts will be written but not submitted.", false, "", "string");
  TCLAP::ValueArg<int> batchstartnArg("","batchstartn", "number of first batch job (e.g. if you have already submitted 100 you can submit another 100 starting from 101)", false, 1, "int");
  TCLAP::ValueArg<int> nbatchjobsArg("","nbatchjobs", "number of jobs to write scripts for and submit to batch system", false, 0, "int");
	TCLAP::ValueArg<int> nBBpointsArg("", "nBBpoints", "number of BergerBoos points per scanpoint", false, 1, "int");
	TCLAP::ValueArg<int> idArg("", "id", "When making controlplots (--controlplots), only consider the "
			"scan point with this ID, that is a specific value of the scan parameter. "
			, false, -1, "int");
  TCLAP::ValueArg<int> nsmoothArg("", "nsmooth", "number of smoothings to apply to final 1-CL plot. Default: 1", false, 1, "int");
	TCLAP::ValueArg<int> ntoysArg("", "ntoys", "number of toy experiments per job. Default: 25", false, 25, "int");
	TCLAP::ValueArg<int> nrunArg("", "nrun", "Number of toy run. To be used with --action pluginbatch.", false, 1, "int");
	TCLAP::ValueArg<int> npointsArg("", "npoints", "Number of scan points used by the Prob method. \n"
			"1D plots: Default 100 points. \n"
			"2D plots: Default 50 points per axis. In the 2D case, equal number of points "
			"for both axes are set. See also --npoints2dx and --npoints2dy.", false, -1, "int");
	TCLAP::ValueArg<int> npoints2dxArg("", "npoints2dx", "Number of 2D scan points, x axis. Default: 50", false, -1, "int");
	TCLAP::ValueArg<int> npoints2dyArg("", "npoints2dy", "Number of 2D scan points, y axis. Default: 50", false, -1, "int");
	TCLAP::ValueArg<int> npointstoyArg("", "npointstoy", "Number of scan points used by the plugin method. Default: 100", false, 100, "int");
	TCLAP::ValueArg<int> ncoveragetoysArg("", "ncoveragetoys", "Number of toys to throw in the coverage method. Default: 100", false, 100, "int");
	TCLAP::MultiArg<string> jobsArg("j", "jobs", "Range of toy job ids to be considered. "
			"To be used with --action plugin. "
			"Can be given multiple times when plotting more than one combinations. In that case, they need to be given in same "
			"order as the -c options. \n"
			"Format (range):  -j min-max \n"
			"Format (single): -j n", false, "string");
	TCLAP::ValueArg<string> jobdirArg("", "jobdir", "Give absolute job-directory if working on batch systems.", false, "default", "string");
  TCLAP::ValueArg<string> toyFilesArg("", "toyFiles", "Pass some different toy files, for example if you want 1D projection of 2D FC.", false, "default", "string" );
  TCLAP::ValueArg<string> saveArg("","save", "Save the workspace this file name", false, "", "string");

	// --------------- switch arguments
  TCLAP::SwitchArg batcheosArg("","batcheos", "When submitting batch jobs (for plugin) write the output to eos", false);
	TCLAP::SwitchArg plotpluginonlyArg("", "po", "Make a 1-CL plot just showing the plugin curves.", false);
	TCLAP::SwitchArg interactiveArg("i", "interactive", "Enables interactive mode (requires X11 session). Exit with Ctrl+c.", false);
	TCLAP::SwitchArg intprobArg("", "intprob", "Use the internal (=Prob) chi2min histogram"
			" instead of the chi2min from the toy files to evaluate 1-CL of the plugin method.", false);
	TCLAP::SwitchArg parevolArg("e", "evol", "Plots the parameter evolution of the profile likelihood.", false);
	TCLAP::SwitchArg controlplotArg("", "controlplots", "Make controlplots analysing the generated toys.", false);
	TCLAP::SwitchArg plotmagneticArg("", "magnetic", "In 2d plots, enable magnetic plot borders which will "
			"attract the 2sigma curves.", false);
	TCLAP::SwitchArg verboseArg("v", "verbose", "Enables verbose output.", false);
	TCLAP::SwitchArg debugArg("d", "debug", "Enables debug level output.", false);
	TCLAP::SwitchArg usageArg("u", "usage", "Prints usage information and exits.", false);
	TCLAP::SwitchArg scanforceArg("f", "scanforce", "Use a stronger minimum finding method for the Plugin method.", false);
	TCLAP::SwitchArg probforceArg("", "probforce", "Use a stronger minimum finding method for the Prob method.", false);
	TCLAP::SwitchArg probimproveArg("", "probimprove", "Use IMPROVE minimum finding for the Prob method.", false);
	TCLAP::SwitchArg largestArg("", "largest", "Report largest CL interval: lowest boundary of "
			"all intervals to highest boundary of all intervals. Useful if two intervals are very "
			"close together.", false);
  TCLAP::SwitchArg latexArg("", "latex", "Make latex tables of observables and correlations", false);
	TCLAP::SwitchArg plotlogArg("", "log", "make logarithmic 1-CL plots", false);
	TCLAP::SwitchArg plotpullsArg("", "pulls", "Make a pull plot illustrating the consistency "
			"of the best solution with the observables.", false);
	TCLAP::SwitchArg lightfilesArg("", "lightfiles", "Produce only light weight root files for the plugin toys."
			" They cannot be used for control plots but save disk space.", false);
	TCLAP::SwitchArg plotprelimArg("", "prelim", "Plot 'Preliminiary' into the plots. See also --unoff .", false);
	TCLAP::SwitchArg plotunoffArg("", "unoff", "Plot 'Unofficial' into the plots. See also --prelim .", false);
	TCLAP::SwitchArg prArg("", "pr", "Enforce the physical range on all parameters (needed to reproduce "
			"the standard Feldman-Cousins with boundary example). If set, no nuisance will be allowed outside the "
			"'phys' limit. However, toy generation of observables is not affected.", false);
  TCLAP::SwitchArg infoArg("", "info", "Print information about the passed combiners and exit", false);
	TCLAP::SwitchArg importanceArg("", "importance", "Enable importance sampling for plugin toys.", false);
	TCLAP::SwitchArg nosystArg("", "nosyst", "Sets all systematic errors to zero.", false);
	TCLAP::SwitchArg printcorArg("", "printcor", "Print the correlation matrix of each solution found.", false);
	TCLAP::SwitchArg smooth2dArg("", "smooth2d", "Smooth 2D p-value or cl histograms for nicer contour (particularly useful for 2D plugin)", false);

	// --------------- aruments that can be given multiple times
	vector<string> vAction;
	//vAction.push_back("bb");
	//vAction.push_back("bbbatch");
  vAction.push_back("coverage");
  vAction.push_back("coveragebatch");
	vAction.push_back("plot");
	//vAction.push_back("plot2d");
	vAction.push_back("plugin");
	vAction.push_back("pluginbatch");
	//vAction.push_back("prob");
	vAction.push_back("runtoys");
	//vAction.push_back("scantree");
	vAction.push_back("test");
  vAction.push_back("uniform");
  vAction.push_back("gaus");
	ValuesConstraint<string> cAction(vAction);
	TCLAP::MultiArg<string> actionArg("a", "action", "Perform action", false, &cAction);
	TCLAP::MultiArg<string> varArg("", "var", "Scan variable (default: g). Can be given twice, in which case "
			"2D scans are performed.", false, "string");
	TCLAP::MultiArg<string> relationArg("", "relation", "Provide the truth relation of the PDF family PDF_GenericGaus, "
			"that connects an observable to the parameters. "
			"Example: --relation 'x+y'. Default: idendity.", false, "string");
	TCLAP::MultiArg<string> combidArg("c", "combid", "ID of combination to be computed. "
			"Use -u to get a list of available combinations. \n"
			"One can also specify modifications to a given combination by adding "
			"or removing PDFs. The syntax is \n"
			" -c 26         # this scans combination number 26\n"
			" -c 26:+12     # this adds PDF number 12 to combination number 26\n"
			" -c 26:+12,-13 # this adds PDF 12 and removes PDF 13 from combination number 26\n"
			"Use -u to get a list of available PDF numbers."
			, false, "int");
	TCLAP::MultiArg<int> colorArg("", "color", "ID of color to be used for the combination. "
			"Default: 0 for first scanner, 1 for second, etc.", false, "int");
  TCLAP::MultiArg<int> fillstyleArg("", "fillstyle", "Fill style of the 1D scan to be used for the combination. Default is 1001 (solid) for all.", false, "int");
  TCLAP::MultiArg<int> pevidArg("", "pevid", "ID of combination used for the profile likelihood"
			"that determines the parameter evolution for the Plugin toy generation. If not given, "
			"the --combid will be used. Use -u to get a list of possible choices.", false, "int");
	TCLAP::MultiArg<int> qhArg("", "qh", "Quick hacks.\n"
			"1: Move up the printed solutions in 1d log plots such that they don't clash with the 95% clabel.\n"
			"2: Move the CL labels to the middle of the 1d plots.\n"
			"3: add 180 deg to the d_dpi axis and solution in the 1d plot.\n"
			"4: move plotted numbers a bit to the left to not cover 1d plot axis.\n"
			"5: Test toy generation in the 1d plugin method. At each scan point, 10 toys are printed out- but nothing is fit.\n"
			"8: Switch on new CL interval maker output.\n"
			"9: Don't remove duplicate/equivalent solutions.\n"
			"10: Don't plot fill pattern for 2D contours to make cleaner looking plots.\n"
			"11: Don't plot dashed lines of 2D contours.\n"
			"12: Use transpareny for 2D contours and filled 1D scans.\n"
			"13: Don't use transparency for the last plotted 2D contour.\n"
			"14: In 2D plots, reduce the y title offset and enlarge the pad accordingly.\n"
			"15: In 2D plots, remove the X% CL content line.\n"
			"16: In parameter evolution plots, add also the full evolution over the scan, in addition to just plotting the best evolution.\n"
      "17: In 2D plots with the PLUGIN and PROB methods, plot the PLUGIN first then the PROB.\n"
      "18: In 2D plots with PLUGIN and PROB methods, set legend titles as PLUGIN and PROB instead of (Plugin) and (Prob).\n"
      "19: In 1D plots, no vertical lines.\n"
      "20: In 1D plots, only central value line.\n"
      "21: Don't add the solution to 1D 1-CL plots.\n"
      "22: In 1D plots draw the legend without changing the y-axis (need also --leg off option).\n"
      "23: Move the CL labels to the right of the 1d plots.\n"
      "24: No fill colours, just lines, in 1D plots.\n"
      "25: Last scan in 1D has thicker line.\n"
      "26: In 2D plots, slightly smaller text size for legend.\n"
      "27: In 2D plots, do not draw any fill color (only the fill style).\n"
			, false, "int");
	TCLAP::MultiArg<string> titleArg("", "title", "Override the title of a combination. "
			"If 'default' is given, the default title for that combination is used. "
            "If 'noleg' is given, this entry is not shown in the legend. "
			"Example: --title 'This is the 1. combination.' --title 'And this the second.'", false, "string");
	TCLAP::MultiArg<string> fixArg("", "fix", "Fix one or more parameters in a combination. "
			"If 'none' is given, all parameters are floated (default). "
			"If given multiple times, the first --fix argument refers to the first combination, "
			"the second one to the second and so on. "
			"If given a single time, it is applied to all combinations. \n"
			"Example: --fix 'g=1.7,r_dk=-0.09' \n"
			"To fix just the parameters in the second combination, do\n"
			"Example: --fix none --fix 'g=1.7,r_dk=0.09' \n"
			, false, "string");
	TCLAP::MultiArg<string> physrangeArg("", "prange", "Adjust the physical range of one or more parameters in a combination. "
			"The ranges are enforced through the --pr option. "
			"If 'def' is given, the default ranges are used. "
			"If given multiple times, the first --fix argument refers to the first combination, "
			"the second one to the second and so on. "
			"If given a single time, it is applied to all combinations. \n"
			"Example: --prange 'g=1.7:1.9,r_dk=0.09:0.2' \n"
			"To modify only the parameters in the second combination, do\n"
			"Example: --prange def --prange 'g=1.7:1.9,r_dk=0.09:0.2' \n"
      "Set to -999:-999 to remove range \n"
			, false, "string");
  TCLAP::MultiArg<string> randomizeToyVarsArg("","randomizeToyVars", "A list of nuisance parameters to randomize in the toy generation for the plugin method when the -a uniform, -a flat or -a gaus methods are also used. Pass as comma sepearted list e.g --randomizeToyVars 'r_dk,r_dpi,d_dk' . Pass once per combiner. If nothing is given here but you pass -a uniform, flat or gaus then ALL nuisance parameter values will be varied in the toys."
      , false, "string");
  TCLAP::MultiArg<string> removeRangeArg("","removeRange","Remove the range entirely of one or more parameters in a combination. "
      "The range are enforced through the --pr option."
      "If 'all' is given, all parameter ranges are removed"
      "Can also use regex matching"
      , false, "string");
	TCLAP::MultiArg<float> snArg("", "sn", "--sn x. Save nuisances to parameter cache file at certain points after a "
			"1d scan was performed. This can be used to set these as starting points "
			"for further scans. "
			"Parameters will be saved for scan point bin that contains x. "
			"Angles have to be given in radians.", false, "float");
	TCLAP::MultiArg<string> sn2dArg("", "sn2d", "Save nuisances as for --sn (1d case) but for 2d scans at the given points. "
			"Format: x:y", false, "string");
	TCLAP::MultiArg<string> loadParamsFileArg("", "parfile", "Load starting parameters for the corresponding "
			"combination from this particular file. "
			"If 'default' is given, the default file for that combination is used, which is found in "
			"plots/par/*_start.dat ."
			"Example: --parfile parsForFirstCombination.dat --parfile parsForSecondCombination.dat", false, "string");
	TCLAP::MultiArg<int> asimovArg("", "asimov", "Run an Asimov toy, in which all observables are set to "
			"truth values defined in a parameter .dat file. This can be the default one, or a configured one "
			"using --parfile.\n"
			"The given value selects the parameter point from the .dat file: first=1, second=2, ...\n"
			"Can be given once per combination. "
			"Example: --asimov 2 --asimov 1\n"
			"This will run an Asimov toy for the first combination at point 2, and another one "
			"for the second combination at point 1. If you only want to run an Asimov for the second "
			"combination, select asimov point 0 for the first one.", false, "int");
	TCLAP::MultiArg<string> asimovFileArg("", "asimovfile", "Load the parameter point to set an Asimov "
			"toy from this file. "
			"If 'default' is given, the default file for that combination is used, which is found in\n"
			"plots/par/*_genpoints.dat\n"
			"The argument can be given multiple times to configure different "
			"files for different Asimov combiners. "
			"Example: --asimovfile parameters.dat", false, "string");
	TCLAP::MultiArg<int> plotsolutionsArg("", "ps", "Include solutions in the plots.\n"
			"1D plots: the numerical value and 1sigma errors are plotted.\n"
			" 0: don't plot\n"
			" 1: at central value\n"
			" 2: at left interval boundary\n"
			" 3: at right interval boundary.\n"
			" 4: a little more left of the left interval boundary.\n"
			"2D plots: markers are plotted at the position of the solution.\n"
			" 0: don't plot\n"
			" 1: plot markers at all local minima\n"
			" 2: plot markers only at best-fit point and equivalent ones (DeltaChi2<0.01).\n"
			"When --ps is only given once, its value will be used for all plotted "
			"combiners. If given less than the number of combinations (-c), the "
			"remaining ones will not plot any solution.",
			false, "int");
  TCLAP::MultiArg<int> plot2dclArg("","2dcl","Plot '2d' confidence level contours in 2d plots.\n"
      "2D plots only:\n"
      " 0: don't plot 2dcl\n"
      " 1: do plot 2dcl\n"
      "When --2dcl is only given once, its value will be used for all plotted "
      "combiners. If given less than the number of combinations (-c), the "
      "remaining ones will plots without --2dcl.",
      false, "int");

	//
	// let TCLAP parse the command line
	// The order is alphabetical - this order defines how the options
	// are ordered on the command line, unfortunately in reverse.
	//
	if ( isIn<TString>(bookedOptions, "verbose" ) ) cmd.add( verboseArg );
	if ( isIn<TString>(bookedOptions, "var" ) ) cmd.add(varArg);
	if ( isIn<TString>(bookedOptions, "usage" ) ) cmd.add( usageArg );
	if ( isIn<TString>(bookedOptions, "unoff" ) ) cmd.add( plotunoffArg );
	if ( isIn<TString>(bookedOptions, "title" ) ) cmd.add( titleArg );
  if ( isIn<TString>(bookedOptions, "toyFiles" ) ) cmd.add( toyFilesArg );
	if ( isIn<TString>(bookedOptions, "sn2d" ) ) cmd.add(sn2dArg);
	if ( isIn<TString>(bookedOptions, "sn" ) ) cmd.add(snArg);
	if ( isIn<TString>(bookedOptions, "smooth2d" ) ) cmd.add( smooth2dArg );
	if ( isIn<TString>(bookedOptions, "scanrangey" ) ) cmd.add( scanrangeyArg );
	if ( isIn<TString>(bookedOptions, "scanrange" ) ) cmd.add( scanrangeArg );
	if ( isIn<TString>(bookedOptions, "scanforce" ) ) cmd.add( scanforceArg );
  if ( isIn<TString>(bookedOptions, "save" ) ) cmd.add( saveArg );
	if ( isIn<TString>(bookedOptions, "relation" ) ) cmd.add(relationArg);
  if ( isIn<TString>(bookedOptions, "removeRange" ) ) cmd.add(removeRangeArg);
  if ( isIn<TString>(bookedOptions, "randomizeToyVars" ) ) cmd.add(randomizeToyVarsArg);
	if ( isIn<TString>(bookedOptions, "qh" ) ) cmd.add(qhArg);
  if ( isIn<TString>(bookedOptions, "queue") ) cmd.add(queueArg);
	if ( isIn<TString>(bookedOptions, "pulls" ) ) cmd.add( plotpullsArg );
	if ( isIn<TString>(bookedOptions, "ps" ) ) cmd.add( plotsolutionsArg );
	if ( isIn<TString>(bookedOptions, "probimprove" ) ) cmd.add( probimproveArg );
	if ( isIn<TString>(bookedOptions, "probforce" ) ) cmd.add( probforceArg );
  if ( isIn<TString>(bookedOptions, "printsolx" ) ) cmd.add( printSolXArg );
  if ( isIn<TString>(bookedOptions, "printsoly" ) ) cmd.add( printSolYArg );
	if ( isIn<TString>(bookedOptions, "printcor" ) ) cmd.add( printcorArg );
	if ( isIn<TString>(bookedOptions, "prelim" ) ) cmd.add( plotprelimArg );
	if ( isIn<TString>(bookedOptions, "po" ) ) cmd.add( plotpluginonlyArg );
	if ( isIn<TString>(bookedOptions, "pluginplotrange" ) ) cmd.add( pluginplotrangeArg );
	if ( isIn<TString>(bookedOptions, "intprob" ) ) cmd.add( intprobArg );
  if ( isIn<TString>(bookedOptions, "plotrangey" ) ) cmd.add( plotrangeyArg );
	if ( isIn<TString>(bookedOptions, "plotnsigmacont" ) ) cmd.add(plotnsigmacontArg);
	if ( isIn<TString>(bookedOptions, "plotid" ) ) cmd.add(plotidArg);
  if ( isIn<TString>(bookedOptions, "plotext" ) ) cmd.add(plotextArg);
	if ( isIn<TString>(bookedOptions, "plot2dcl" ) ) cmd.add( plot2dclArg );
	if ( isIn<TString>(bookedOptions, "pr" ) ) cmd.add( prArg );
	if ( isIn<TString>(bookedOptions, "physrange" ) ) cmd.add(physrangeArg);
	if ( isIn<TString>(bookedOptions, "pevid" ) ) cmd.add( pevidArg );
  if ( isIn<TString>(bookedOptions, "nsmooth" ) ) cmd.add(nsmoothArg);
	if ( isIn<TString>(bookedOptions, "ntoys" ) ) cmd.add(ntoysArg);
	if ( isIn<TString>(bookedOptions, "nrun" ) ) cmd.add(nrunArg);
	if ( isIn<TString>(bookedOptions, "npointstoy" ) ) cmd.add(npointstoyArg);
	if ( isIn<TString>(bookedOptions, "ncoveragetoys" ) ) cmd.add(ncoveragetoysArg);
	if ( isIn<TString>(bookedOptions, "npoints2dy" ) ) cmd.add(npoints2dyArg);
	if ( isIn<TString>(bookedOptions, "npoints2dx" ) ) cmd.add(npoints2dxArg);
	if ( isIn<TString>(bookedOptions, "npoints" ) ) cmd.add(npointsArg);
	if ( isIn<TString>(bookedOptions, "nosyst" ) ) cmd.add( nosystArg );
	if ( isIn<TString>(bookedOptions, "ndivy" ) ) cmd.add(ndivyArg);
	if ( isIn<TString>(bookedOptions, "ndiv" ) ) cmd.add(ndivArg);
	if ( isIn<TString>(bookedOptions, "nBBpoints" ) ) cmd.add(nBBpointsArg);
  if ( isIn<TString>(bookedOptions, "nbatchjobs" ) ) cmd.add(nbatchjobsArg);
	if ( isIn<TString>(bookedOptions, "magnetic" ) ) cmd.add( plotmagneticArg );
	if ( isIn<TString>(bookedOptions, "log" ) ) cmd.add( plotlogArg );
	if ( isIn<TString>(bookedOptions, "loadParamsFile" ) ) cmd.add( loadParamsFileArg );
	if ( isIn<TString>(bookedOptions, "lightfiles" ) ) cmd.add( lightfilesArg );
	if ( isIn<TString>(bookedOptions, "legsize" ) ) cmd.add( plotlegsizeArg );
  if ( isIn<TString>(bookedOptions, "legstyle" ) ) cmd.add( plotlegstyleArg );
	if ( isIn<TString>(bookedOptions, "leg" ) ) cmd.add( plotlegArg );
	if ( isIn<TString>(bookedOptions, "largest" ) ) cmd.add( largestArg );
  if ( isIn<TString>(bookedOptions, "latex" ) ) cmd.add( latexArg );
	if ( isIn<TString>(bookedOptions, "jobs" ) ) cmd.add(jobsArg);
	if ( isIn<TString>(bookedOptions, "jobdir" ) ) cmd.add( jobdirArg );
	if ( isIn<TString>(bookedOptions, "interactive" ) ) cmd.add( interactiveArg );
  if ( isIn<TString>(bookedOptions, "info" ) ) cmd.add( infoArg );
	if ( isIn<TString>(bookedOptions, "importance" ) ) cmd.add( importanceArg );
	if ( isIn<TString>(bookedOptions, "id" ) ) cmd.add(idArg);
  if ( isIn<TString>(bookedOptions, "hfagLabel" ) ) cmd.add(hfagLabelArg);
  if ( isIn<TString>(bookedOptions, "hfagLabelPos" ) ) cmd.add(hfagLabelPosArg);
	if ( isIn<TString>(bookedOptions, "group" ) ) cmd.add( plotgroupArg );
	if ( isIn<TString>(bookedOptions, "grouppos" ) ) cmd.add( plotgroupposArg );
	if ( isIn<TString>(bookedOptions, "fix" ) ) cmd.add(fixArg);
  if ( isIn<TString>(bookedOptions, "fillstyle" ) ) cmd.add( fillstyleArg );
	if ( isIn<TString>(bookedOptions, "ext" ) ) cmd.add(filenameadditionArg);
    if ( isIn<TString>(bookedOptions, "filename" ) ) cmd.add( filenamechangeArg );
	if ( isIn<TString>(bookedOptions, "evol" ) ) cmd.add(parevolArg);
	if ( isIn<TString>(bookedOptions, "digits" ) ) cmd.add(digitsArg);
	if ( isIn<TString>(bookedOptions, "debug" ) ) cmd.add(debugArg);
	if ( isIn<TString>(bookedOptions, "covCorrectPoint" ) ) cmd.add(coverageCorrectionPointArg);
	if ( isIn<TString>(bookedOptions, "covCorrect" ) ) cmd.add(coverageCorrectionIDArg);
	if ( isIn<TString>(bookedOptions, "controlplots" ) ) cmd.add(controlplotArg);
	if ( isIn<TString>(bookedOptions, "combid" ) ) cmd.add(combidArg);
	if ( isIn<TString>(bookedOptions, "color" ) ) cmd.add(colorArg);
  if ( isIn<TString>(bookedOptions, "batchstartn" ) ) cmd.add( batchstartnArg );
  if ( isIn<TString>(bookedOptions, "batcheos" ) ) cmd.add(batcheosArg);
	if ( isIn<TString>(bookedOptions, "asimovfile" ) ) cmd.add( asimovFileArg );
	if ( isIn<TString>(bookedOptions, "asimov") ) cmd.add(asimovArg);
	if ( isIn<TString>(bookedOptions, "action") ) cmd.add(actionArg);
	cmd.parse( argc, argv );

	//
	// copy over parsed values into data members
	//
	asimov            = asimovArg.getValue();
	color             = colorArg.getValue();
	controlplot       = controlplotArg.getValue();
	digits            = digitsArg.getValue();
	enforcePhysRange  = prArg.getValue();
	filenameaddition  = filenameadditionArg.getValue();
  filenamechange    = filenamechangeArg.getValue();
  fillstyle         = fillstyleArg.getValue();
  hfagLabel         = hfagLabelArg.getValue();
	group             = plotgroupArg.getValue();
	id                = idArg.getValue();
	importance        = importanceArg.getValue();
  info              = infoArg.getValue();
	interactive       = interactiveArg.getValue();
	intprob           = intprobArg.getValue();
	jobdir            = TString(jobdirArg.getValue());
	largest           = largestArg.getValue();
  latex             = latexArg.getValue();
	lightfiles        = lightfilesArg.getValue();
  batchstartn       = batchstartnArg.getValue();
  batcheos          = batcheosArg.getValue();
  nbatchjobs        = nbatchjobsArg.getValue();
	nBBpoints         = nBBpointsArg.getValue();
	ndiv              = ndivArg.getValue();
	ndivy             = ndivyArg.getValue();
	nosyst            = nosystArg.getValue();
	npoints1d         = npointsArg.getValue()==-1 ? 100 : npointsArg.getValue();
	npoints2dx        = npoints2dxArg.getValue()==-1 ? (npointsArg.getValue()==-1 ? 50 : npointsArg.getValue()) : npoints2dxArg.getValue();
	npoints2dy        = npoints2dyArg.getValue()==-1 ? (npointsArg.getValue()==-1 ? 50 : npointsArg.getValue()) : npoints2dyArg.getValue();
	npointstoy        = npointstoyArg.getValue();
  ncoveragetoys     = ncoveragetoysArg.getValue();
	nrun	            = nrunArg.getValue();
	ntoys	            = ntoysArg.getValue();
  nsmooth           = nsmoothArg.getValue();
	parevol           = parevolArg.getValue();
	pevid             = pevidArg.getValue();
  plotext           = plotextArg.getValue();
	plotid            = plotidArg.getValue();
	plotlog           = plotlogArg.getValue();
  plotlegstyle      = plotlegstyleArg.getValue();
	plotmagnetic      = plotmagneticArg.getValue();
	plotnsigmacont    = plotnsigmacontArg.getValue();
	plotpluginonly    = plotpluginonlyArg.getValue();
	plotprelim        = plotprelimArg.getValue();
	plotpulls         = plotpullsArg.getValue();
	plotunoff         = plotunoffArg.getValue();
	printcor          = printcorArg.getValue();
  printSolX         = printSolXArg.getValue();
  printSolY         = printSolYArg.getValue();
	probforce         = probforceArg.getValue();
	probimprove       = probimproveArg.getValue();
	qh                = qhArg.getValue();
  queue             = TString(queueArg.getValue());
  save              = saveArg.getValue();
	savenuisances1d   = snArg.getValue();
	scanforce         = scanforceArg.getValue();
	smooth2d          = smooth2dArg.getValue();
  toyFiles          = toyFilesArg.getValue();
	usage             = usageArg.getValue();
	verbose           = verboseArg.getValue();

	//
	// The following options need some post-processing to
	// check for allowed values and set sensible things.
	//

	// -c
	// Test parsing:
	//int resultCmbId = 0;
	//vector<int> resultAddPdf;
	//vector<int> resultDelPdf;
	//TString parseMe = combidArg.getValue()[0];
	//cout << "parseMe " << parseMe << endl;
	//parseCombinerString(parseMe, resultCmbId, resultAddPdf, resultDelPdf);
	//cout << "resultCmbId " << resultCmbId << endl;
	//for ( int i=0; i<resultAddPdf.size(); i++){cout << "resultAddPdf " << resultAddPdf[i] << endl;}
	//for ( int i=0; i<resultDelPdf.size(); i++){cout << "resultDelPdf " << resultDelPdf[i] << endl;}
	//exit(0);
	vector<string> tmp = combidArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ){
		int resultCmbId = 0;
		vector<int> resultAddDelPdf;
		parseCombinerString(tmp[i], resultCmbId, resultAddDelPdf);
		combid.push_back(resultCmbId);
		combmodifications.push_back(resultAddDelPdf);
	}

	// --action
	tmp = actionArg.getValue();  ///< can't assign directly because of TString cast
	for ( int i = 0; i < tmp.size(); i++ ) action.push_back(tmp[i]);

	// --var
	tmp = varArg.getValue();
	if ( tmp.size()>2 ){
		cout << "Argument error --var: please give two instances at maximum." << endl;
		exit(1);
	}
	for ( int i = 0; i < tmp.size(); i++ ) var.push_back(tmp[i]);
	if ( tmp.size()==0 ) var.push_back("g");

	// --relation
	tmp = relationArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ) relation.push_back(tmp[i]);
	if ( tmp.size()==0 ) relation.push_back("NoDefaultEquation");

	// --title
	tmp = titleArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ) title.push_back(tmp[i]);
	if ( tmp.size()==0 ) title.push_back("default");

	// --parfile
	tmp = loadParamsFileArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ) loadParamsFile.push_back(tmp[i]);

	// --asimovfile
	tmp = asimovFileArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ) asimovfile.push_back(tmp[i]);

	// --debug
	debug = debugArg.getValue();
	if ( debug ) verbose = true;

	// --covCorrect
	coverageCorrectionID = coverageCorrectionIDArg.getValue();
	if (coverageCorrectionID < 0 || coverageCorrectionID > 3){
		cout << "Argument error: covCorrect has to be in the range [0,3]" << endl;
		exit(1);
	}
	coverageCorrectionPoint = coverageCorrectionPointArg.getValue();

	// --sn2d
	for ( int i = 0; i < sn2dArg.getValue().size(); i++ ){
		TString parseMe = sn2dArg.getValue()[i];
		TString x = parseMe;
		TString y = parseMe;
		x.Replace(x.Index(":"), x.Sizeof(), "");
		y.Replace(0, y.Index(":")+1, "");
		savenuisances2dx.push_back(x.Atof());
		savenuisances2dy.push_back(y.Atof());
	}

	// --jobs
	if ( jobsArg.getValue().size()>1 && jobsArg.getValue().size()!=combid.size() ){
		cout << "Argument error: Please give as many job ranges (-j) as combinations (-c)." << endl;
		exit(1);
	}
	if ( jobsArg.getValue().size()==0 ){
		// default: job 1
		jmin.push_back(1);
		jmax.push_back(1);
	}
	for ( int i=0; i<jobsArg.getValue().size(); i++ ){
		TString usage = "";
		usage += "Required format: '-j N | -j A-B'\n";
		usage += "  Examples:\n";
		usage += "  -j 10\n";
		usage += "  -j 10-20\n";
		TString parseMe = jobsArg.getValue()[i];
		TRegexp range("^[0-9]+-[0-9]+$");
		if ( parseMe.Contains(range) ){
			// range found
			TString x = parseMe;
			TString y = parseMe;
			x.Replace(x.Index("-"), x.Sizeof(), "");
			y.Replace(0, y.Index("-")+1, "");
			int min = convertToDigitWithCheck(x,usage);
			int max = convertToDigitWithCheck(y,usage);
			if ( min>max ){
				cout << "Argument error: job range min>max." << endl;
				exit(1);
			}
			jmin.push_back(min);
			jmax.push_back(max);
		}
		else{
			// single number found
			int n = convertToDigitWithCheck(parseMe,usage);
			jmin.push_back(n);
			jmax.push_back(n);
		}
	}

	// --leg
	TString usage = "";
	usage += "Required format: '--leg 0.a:0.b'\n";
	usage += "  Examples:\n";
	usage += "  --leg 0.5:0.75\n";
	usage += "  --leg 0.5:def\n";
	usage += "  --leg off\n";
	if ( TString(plotlegArg.getValue())==TString("off") ){
		plotlegend = false;
	}
	else{
		plotlegend = true;
		parsePosition(plotlegArg.getValue(), plotlegx, plotlegy, usage);
	}

	// --legsize
	usage = "";
	usage += "Required format: '--legsize 0.a:0.b'\n";
	usage += "  Examples:\n";
	usage += "  --legsize 0.4:0.2\n";
	usage += "  --legsize 0.4:def\n";
	usage += "  --legsize def:0.2\n";
	parsePosition(plotlegsizeArg.getValue(), plotlegsizex, plotlegsizey, usage);

	// --grouppos
	usage = "";
	usage += "Required format: '--grouppos 0.a:0.b'\n";
	usage += "  Examples:\n";
	usage += "  --grouppos 0.5:0.75\n";
	usage += "  --grouppos 0.5:def\n";
	groupPos = plotgroupposArg.getValue();
	parsePosition(groupPos, plotgroupx, plotgroupy, usage);

  // --hfagLabelPos
  usage = "";
  usage += "Required format: '--hfagLabelPos 0.a:0.b:c'\n";
  usage += "  Examples:\n";
  usage += "  --hfagLabelPos 0.8:0.8:1\n";
  usage += "  --hfagLabelPos 0.6:0.9:def\n";
  hfagLabelPos = hfagLabelPosArg.getValue();
  parsePositionAndScale(hfagLabelPos, plotHFAGLabelPosX, plotHFAGLabelPosY, plotHFAGLabelScale, usage);

	// --pluginplotrange
	parseRange(pluginplotrangeArg.getValue(), pluginPlotRangeMin, pluginPlotRangeMax);

	// --scanrange
	parseRange(scanrangeArg.getValue(), scanrangeMin, scanrangeMax);
	parseRange(scanrangeyArg.getValue(), scanrangeyMin, scanrangeyMax);

  // --plotrange
  parseRange(plotrangeyArg.getValue(), plotymin, plotymax );

	// --prange
	tmp = physrangeArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ){ // loop over instances of --prange
		vector<RangePar> ranges;
		// parse default string
		if ( TString(tmp[i])==TString("def") ){
			physRanges.push_back(ranges);
			continue;
		}
		// parse list of ranges: "foopar=5:6.2,barpar=7.4:8.5"
		TObjArray *rangesArray = TString(tmp[i]).Tokenize(","); // split string at ","
		for ( int j=0; j<rangesArray->GetEntries(); j++ ){ // loop over ranges
			TString rangeString = ((TObjString*)rangesArray->At(j))->GetString();
			RangePar p;
			TString parsedRangeStr;
			bool check = parseAssignment(rangeString, p.name, parsedRangeStr);
			check = check && parseRange(parsedRangeStr, p.min, p.max);
			if ( check ) ranges.push_back(p);
			else{
				cout << "ERROR : parse error in --prange argument: " << rangeString << endl << endl;
			}
		}
		physRanges.push_back(ranges);
	}
	// test code for --prange
	//for ( int i = 0; i < physRanges.size(); i++ ){
		//cout << "combination " << i << endl;
		//for ( int j = 0; j < physRanges[i].size(); j++ ){
			//cout << physRanges[i][j].name << " = " << physRanges[i][j].min << " ... " << physRanges[i][j].max << endl;
		//}
	//}
	//exit(0);

  // --randomizeToyVars
  tmp = randomizeToyVarsArg.getValue();
  for ( int i = 0; i < tmp.size(); i++ ) { // loop over instances of --randomizeToyVars
    TObjArray *parsArray = TString(tmp[i]).Tokenize(","); // split string at ","
    vector<TString> pars;
    for ( int j=0; j<parsArray->GetEntries(); j++){
      TString par = ((TObjString*)parsArray->At(j))->GetString();
      pars.push_back(par);
    }
    randomizeToyVars.push_back(pars);
  }

  // --removeRange
  tmp = removeRangeArg.getValue();
  for ( int i = 0; i < tmp.size(); i++) { // loop over instances of --removeRange
    TObjArray *parsArray = TString(tmp[i]).Tokenize(","); // split string at ","
    vector<TString> pars;
    for ( int j=0; j<parsArray->GetEntries(); j++){
      TString par = ((TObjString*)parsArray->At(j))->GetString();
      pars.push_back(par);
    }
    removeRanges.push_back(pars);
  }

	// --fix
	tmp = fixArg.getValue();
	for ( int i = 0; i < tmp.size(); i++ ){ // loop over instances of --fix
		vector<FixPar> assignments;
		// parse 'none' default string
		if ( TString(tmp[i])==TString("none") ){
			fixParameters.push_back(assignments);
			continue;
		}
		// parse list of fixed parameters: "foopar=5,barpar=7"
		TObjArray *assignmentArray = TString(tmp[i]).Tokenize(","); // split string at ","
		for ( int j=0; j<assignmentArray->GetEntries(); j++ ){ // loop over assignments
			TString assignmentString = ((TObjString*)assignmentArray->At(j))->GetString();
			FixPar p;
			if ( parseAssignment(assignmentString, p.name, p.value) ){
				assignments.push_back(p);
			}
			else{
				cout << "ERROR : parse error in --fix argument: " << assignmentString << endl << endl;
			}
		}
		fixParameters.push_back(assignments);
	}
	// // test code for --fix
	// for ( int i = 0; i < fixParameters.size(); i++ ){
	// 	cout << "combination " << i << endl;
	// 	for ( int j = 0; j < fixParameters[i].size(); j++ ){
	// 		cout << fixParameters[i][j].name << " = " << fixParameters[i][j].value << endl;
	// 	}
	// }
	// exit(0);

	// --ps
	// If --ps is only given once, apply the given setting to all
	// combiners.
	plotsolutions     = plotsolutionsArg.getValue();
	if ( plotsolutions.size()==1 && combid.size()>1 ){
		for ( int i=1; i<combid.size(); i++ ){
			plotsolutions.push_back(plotsolutions[0]);
		}
	}
	// If --ps is given more than once, but not for every combiner,
	// fill the remaining ones up with 0=don't plot solution
	else if ( plotsolutions.size() < combid.size() ){
		for ( int i=plotsolutions.size(); i<combid.size(); i++ ){
			plotsolutions.push_back(0);
		}
	}

	plot2dcl = plot2dclArg.getValue();
  // If --2dcl is not given, apply 0 to all
	if ( plot2dcl.size()==0){
		for (int i=0; i<10; i++){
			plot2dcl.push_back(0);
		}
	}
  // combiners
	// If --2dcl is only given once, apply the given setting to all
  // combiners
  if ( plot2dcl.size()==1 && combid.size()>0 ){
    for (int i=0; i<combid.size(); i++ ){
      plot2dcl.push_back(plot2dcl[0]);
    }
  }
  // If --2dcl is given more than once, but not for every combiner,
  // fill the remaining ones up with 0=don't plot 2dcl
  else if ( plot2dcl.size() > 1 && plot2dcl.size() <= combid.size() ) {
    for ( int i=plot2dcl.size(); i<combid.size(); i++ ) {
      plot2dcl.push_back(0);
    }
  }

	// check --po argument
	if ( plotpluginonly && !isAction("plugin") ){
		cout << "ERROR : --po can only be given when -a plugin is set." << endl;
		exit(1);
	}
}

///
/// Parse the position arguments, which define the position of
/// the legend and logo in the plots. The position needs to be
/// given as x:y, where x and y are floating point numbers between
/// 0.0 and 1.0. To use the default position for either one, use
/// "def".
/// \param parseMe	- parse this string. Example formats:
///                       0.1:0.5, .1:.5, def:0.6, .75:def
/// \param x		- return value x
/// \param y		- return value y
/// \param usage	- string containing some usage information which is
///			  printed when there is an error.
///
void OptParser::parsePosition(TString parseMe, float &x, float &y, TString usage)
{
	if ( parseMe==TString("default") ){
		x = -1.;
		y = -1.;
		return;
	}
	if ( parseMe==TString("off") ){
		return;
	}
	TRegexp format1("^0?\\.[0-9]+:0?\\.[0-9]+$");
	TRegexp format2("^def:0?\\.[0-9]+$");
	TRegexp format3("^0?\\.[0-9]+:def$");
	if ( !( parseMe.Contains(format1) || parseMe.Contains(format2) || parseMe.Contains(format3) ) ){
		cout << "position parse error: could not parse " << parseMe << endl;
		cout << usage << endl;
		exit(1);
	}
	TString xStr = parseMe;
	TString yStr = parseMe;
	xStr.Replace(xStr.Index(":"), xStr.Sizeof(), "");
	yStr.Replace(0, yStr.Index(":")+1, "");
	if ( xStr.EqualTo("def") ){
		x = -1;
	}
	else {
		x = xStr.Atof();
	}
	if ( yStr.EqualTo("def") ){
		y = -1;
	}
	else {
		y = yStr.Atof();
	}
	if ( ! ( (x==-1 || (0.0<=x && x<=1.0)) && (y==-1 || (0.0<=y && y<=1.0)) )){
		// should never be reached
		cout << "Argument error: coordinates out of range: x=" << x << ", y=" << y << endl;
		cout << "They need to be in  [0,1], or equal to -1 to set the default value." << endl;
		exit(1);
	}
}

void OptParser::parsePositionAndScale(TString parseMe, Double_t& x, Double_t& y, Double_t& scale, TString usage) {
	if ( parseMe==TString("default") ){
		x = 0;
		y = 0;
    scale = 0;
		return;
	}
	if ( parseMe==TString("off") ){
		return;
	}
	TRegexp format1("^0?\\.[0-9]+:0?\\.[0-9]+:[0-9]+\\.[0-9]+$");
	TRegexp format2("^0?\\.[0-9]+:0?\\.[0-9]+:def$");
	TRegexp format3("def:0?\\.[0-9]+:[0-9]+\\.[0-9]+$");
	TRegexp format4("def:0?\\.[0-9]+:def$");
	TRegexp format5("^0?\\.[0-9]+:def:[0-9]+\\.[0-9]+$");
	TRegexp format6("^0?\\.[0-9]+:def:def$");
	if ( !( parseMe.Contains(format1) || parseMe.Contains(format2) || parseMe.Contains(format3) || parseMe.Contains(format4) || parseMe.Contains(format5) || parseMe.Contains(format6) ) ){
		cout << "position parse error: could not parse " << parseMe << endl;
		cout << usage << endl;
		exit(1);
	}
	TString xStr = parseMe;
  TString sStr = parseMe;
	xStr.Replace(xStr.Index(":"), xStr.Sizeof(), "");
	sStr.Replace(0, sStr.Index(":")+1, "");
	TString yStr = sStr;
  TString zStr = sStr;
  yStr.Replace(yStr.Index(":"), yStr.Sizeof(), "");
  zStr.Replace(0, zStr.Index(":")+1, "");
	if ( xStr.EqualTo("def") ){
		x = 0;
	}
	else {
		x = xStr.Atof();
	}
	if ( yStr.EqualTo("def") ){
		y = 0;
	}
	else {
		y = yStr.Atof();
	}
	if ( ! ( (x==-1 || (0.0<=x && x<=1.0)) && (y==-1 || (0.0<=y && y<=1.0)) )){
		// should never be reached
		cout << "Argument error: coordinates out of range: x=" << x << ", y=" << y << endl;
		cout << "They need to be in  [0,1], or equal to -1 to set the default value." << endl;
		exit(1);
	}
  if ( zStr.EqualTo("def") ) {
    scale = 1;
  }
  else {
    scale = zStr.Atof();
  }
}

///
/// Parse the range arguments.
/// \param parseMe Format: "min-max", e.g. "3.28:7.34"
/// \param min return value
/// \param max return value
///
bool OptParser::parseRange(TString parseMe, float &min, float &max)
{
	if ( parseMe==TString("default") ){
		min = -104;
		max = -104;
	}
	else {
		TString minStr = parseMe;
		TString maxStr = parseMe;
		minStr.Replace(minStr.Index(":"), minStr.Sizeof(), "");
		maxStr.Replace(0, maxStr.Index(":")+1, "");
		min = minStr.Atof();
		max = maxStr.Atof();
	}
	if ( min>max ){
		return false;
	}
	return true;
}

///
/// Parse a variable assignment string.
/// \param parseMe Format: "foovar=3.14"
/// \param name return string
/// \param value return value
///
bool OptParser::parseAssignment(TString parseMe, TString &name, TString &value)
{
	TString nameStr = parseMe;
	TString valueStr = parseMe;
	if ( parseMe.Index("=") == -1 ) return false; // parse error: '=' not found
	nameStr.Replace(nameStr.Index("="), nameStr.Sizeof(), "");
	valueStr.Replace(0, valueStr.Index("=")+1, "");
	name = nameStr;
	value = valueStr;
	return true;
}

///
/// Parse a variable assignment string.
/// \param parseMe Format: "foovar=3.14"
/// \param name return string
/// \param value return value
///
bool OptParser::parseAssignment(TString parseMe, TString &name, float &value)
{
	TString valueStr;
	parseAssignment(parseMe, name, valueStr);
	value = valueStr.Atof();
	return true;
}

///
/// Helper function for parseCombinerString().
/// Checks if a string is an integer (pos or neg), if not, exits with printing the usage.
///
int OptParser::convertToIntWithCheck(TString parseMe, TString usage)
{
	if ( !( !parseMe.Contains(".") && !parseMe.Contains(",") && parseMe.IsFloat() ) ){
		cout << "ERROR : could not parse argument. This string is not a positive or negative integer: '" << parseMe << "'" << endl;
		cout << usage << endl;
		exit(1);
	}
	return parseMe.Atoi();
}

///
/// Helper function for parseCombinerString().
/// Checks if a string is a digit (positive integer), if not, exits with printing the usage.
///
int OptParser::convertToDigitWithCheck(TString parseMe, TString usage)
{
	if ( !parseMe.IsDigit() ){
		cout << "ERROR : could not parse argument. This string is not a positive integer: '" << parseMe << "'" << endl;
		cout << usage << endl;
		exit(1);
	}
	return parseMe.Atoi();
}

///
/// Parse -c string (combiner ID with add/del PDF modification requests).
/// The string to parse looks sth like "combinerId:+pdfId1,-pdfId2"
/// Examples:
/// -c 26
/// -c 26:+12,+23
/// -c 26:+12,-3
///
/// \param parseMe		- the string provided to -c
/// \param resultCmbId		- resulting combiner ID
/// \param resultAddDelPdf	- vector of all PDF IDs, that are requested to be added or deleted
/// 				 	to/from the combiner. If it is supposed to be added, a positive PDF ID
///					is stored, if it is supposed to be deleted, a negative PDF ID is stored
///
void OptParser::parseCombinerString(TString parseMe, int& resultCmbId, vector<int>& resultAddDelPdf)
{
	resultCmbId = 0;
	resultAddDelPdf.clear();
	TString usage = "";
	usage += "Required format: '-c combinerId[:+pdfId1[,-pdfId2,...]]'\n";
	usage += "  Examples:\n";
	usage += "  -c 26\n";
	usage += "  -c 26:+12\n";
	usage += "  -c 26:+12,-3\n";
	// simplest case, no modification
	if ( !parseMe.Contains(":") ){
		resultCmbId = convertToDigitWithCheck(parseMe, usage);
		return;
	}
	// advanced case, there are PDF modifications
	// 1. parse leading combiner ID
	TObjArray *array = parseMe.Tokenize(":"); // split string at ":"
	if ( array->GetEntries()!=2 ){
		cout << "-c parse error: too many ':'. " << usage << endl;
		exit(1);
	}
	TString combinerIdStr = ((TObjString*)array->At(0))->GetString(); // gets the part before the colon
	resultCmbId = convertToDigitWithCheck(combinerIdStr, usage);
	// 2. parse list of PDF IDs
	TString pdfIdsListStr = ((TObjString*)array->At(1))->GetString(); // gets the part after the colon
	TObjArray *arrayCommaList = pdfIdsListStr.Tokenize(","); // split string at ","
	for ( int j=0; j<arrayCommaList->GetEntries(); j++ ){
		TString pdfId = ((TObjString*)arrayCommaList->At(j))->GetString();
		if ( ! (pdfId.BeginsWith("+") || pdfId.BeginsWith("-") ) ){
			cout << "-c parse error: first character not a + or -. " << usage << endl;
			exit(1);
		}
		resultAddDelPdf.push_back(convertToIntWithCheck(pdfId, usage));
	}
	delete array;
	delete arrayCommaList;
}

///
/// Check if a combiner with a given ID is an Asimov combiner.
/// The ID is the position of the -c argument on the command line.
///
/// \param id - position of -c argument
///
bool OptParser::isAsimovCombiner(int id)
{
	return id<asimov.size() && asimov[id]>0;
}

