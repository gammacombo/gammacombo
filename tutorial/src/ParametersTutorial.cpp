#include "ParametersTutorial.h"

ParametersTutorial::ParametersTutorial()
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
void ParametersTutorial::defineParameters()
{
	Parameter *p = 0;

	p = newParameter("a_gaus");
	p->title = "a_{Gaus}";
	p->startvalue = -0.5;
	p->unit = "";
	p->scan = range(-2.5, 2.5);
	p->phys = range(-1e4, 1e4); // to implement a Feldman-Cousins like forbidden region, set the allowed region here and use --pr
	p->free = range(-1e4, 1e4);

	p = newParameter("b_gaus");
	p->title = "b_{Gaus}";
	p->startvalue = 1.5;
	p->unit = "";
	p->scan = range(-2, 4);
	p->phys = range(-1e4, 1e4);
	p->free = range(-1e4, 1e4);
}

