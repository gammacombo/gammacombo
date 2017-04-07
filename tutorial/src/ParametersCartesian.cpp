#include "ParametersCartesian.h"

ParametersCartesian::ParametersCartesian()
{
	defineParameters();
}

///
/// Define all (nuisance) parameters.
///
///  scan:      defines scan range (for Prob and Plugin methods)
///  phys:      physically allowed range (needs to be set!)
///  free:	range applied when no boundary is required - set this far away from any possible value
///
void ParametersCartesian::defineParameters()
{
	Parameter *p = 0;

	p = newParameter("g");
	p->title = "#gamma";
	p->startvalue = DegToRad(70);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(180));
	p->phys = range(-7, 7);

	p = newParameter("d_dk");
	p->title = "#delta_{B}^{DK}";
	p->startvalue = DegToRad(127);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(180));
	p->phys = range(-7, 7);

	p = newParameter("r_dk");
	p->title = "r_{B}^{DK}";
	p->startvalue = 0.09;
	p->unit = "";
	p->scan = range(0.02, 0.2);
	p->phys = range(0, 1e4);
}

