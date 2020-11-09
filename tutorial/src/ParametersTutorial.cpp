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


	p = newParameter("mean_DSCB");
	p->title = "mean_{DSCB}";
	p->startvalue = 5600;
	p->unit = "MeV/c^{2}";
	p->scan = range(5300, 5700);
	p->phys = range(5300, 5700);
	p->free = range(5000, 6000);
	
        p = newParameter("sigma_DSCB");
	p->title = "sigma_{DSCB}";
	p->startvalue = 15;
	p->unit = "";
	p->scan = range(5, 100);
	p->phys = range(0.1, 1e3);
	p->free = range(0.001, 1e3);
	
        p = newParameter("alpha1_DSCB");
        p->title = "alpha1_{DSCB}";
	p->startvalue = 1.8;
	p->unit = "";
	p->scan = range(0.5, 10);
	p->phys = range(1e-3, 1e2);
	p->free = range(1e-3, 1e2);
	
        p = newParameter("alpha2_DSCB");
	p->title = "alpha2_{DSCB}";
	p->startvalue = 2.0;
	p->unit = "";
	p->scan = range(0.5, 10);
	p->phys = range(1e-3, 1e2);
	p->free = range(1e-3, 1e2);
	
        p = newParameter("n1_DSCB");
	p->title = "n1_{DSCB}";
	p->startvalue = 0.5;
	p->unit = "";
	p->scan = range(0.2, 6);
	p->phys = range(1e-3, 1e2);
	p->free = range(1e-3, 1e2);

	p = newParameter("n2_DSCB");
	p->title = "n2_{DSCB}";
	p->startvalue = 1;
	p->unit = "";
	p->scan = range(0.5, 5);
	p->phys = range(1e-3, 1e2);
	p->free = range(11e-3, 1e2);


	p = newParameter("BFsig");
	p->title = "BFsig";
	p->startvalue = 1e-8;
	p->unit = "";
	p->scan = range(0, 140e-9);
	p->phys = range(0, 1e-6);
	p->free = range(0, 1e-6);
}

