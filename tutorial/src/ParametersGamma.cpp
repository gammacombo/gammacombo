#include "ParametersGamma.h"

ParametersGamma::ParametersGamma()
{
	defineParameters();
}

///
/// Define all (nuisance) parameters.
///
///  scan:      defines scan range (for Prob and Plugin methods)
///  phys:      physically allowed range (needs to be set!)
///  force:     min, max used by the force fit method
///
void ParametersGamma::defineParameters()
{
	Parameter *p = 0;

	p = newParameter("g");
	p->title = "#it{#gamma}";
	p->startvalue = DegToRad(70);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(180));
	p->phys = range(DegToRad(0), DegToRad(180));
	p->force = range(DegToRad(0), DegToRad(90));

	p = newParameter("d_dk");
	//p->title = "#delta_{B}^{K}";
	p->title = "#it{#delta}_{#it{B}^{#pm}}^{#it{D}#it{K}^{#pm}}";
	p->startvalue = DegToRad(140);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(180));
	p->phys = range(DegToRad(0), DegToRad(180));
	p->force = range(DegToRad(0), DegToRad(90));

	p = newParameter("r_dk");
	// p->title = "r_{B}^{K}";
	p->title = "#it{r}_{#it{B}^{#pm}}^{#it{D}#it{K}^{#pm}}";
	p->startvalue = 0.1;
	p->unit = "";
	p->scan = range(0.02, 0.2);
	p->phys = range(0.06,0.14);
	p->force = range(0.02, 0.16);

	p = newParameter("xm_dk");
	p->title = "x- (DK)";
	p->startvalue = 0.046;
	p->unit = "";
	p->scan = range(-0.2, 0.2);
	p->phys = range(-1e4, 1e4);
	p->force = range(-0.2, 0.2);

	p = newParameter("ym_dk");
	p->title = "y- (DK)";
	p->startvalue = 0.092;
	p->unit = "";
	p->scan = range(0.0, 0.3);
	p->phys = range(-1e4, 1e4);
	p->force = range(-0.2, 0.2);

	p = newParameter("xp_dk");
	p->title = "x+ (DK)";
	p->startvalue = -0.089;
	p->unit = "";
	p->scan = range(-0.2, 0.2);
	p->phys = range(-1e4, 1e4);
	p->force = range(-0.2, 0.2);

	p = newParameter("yp_dk");
	p->title = "y+ (DK)";
	p->startvalue = -0.052;
	p->unit = "";
	p->scan = range(-0.2, 0.2);
	p->phys = range(-1e4, 1e4);
	p->force = range(-0.2, 0.2);

  // B+ -> D*K+
	p = newParameter("d_dstk");
	p->title = "#it{#delta_{B}^{D*K}}";
	p->startvalue = DegToRad(210.);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(360));
	p->phys = range(DegToRad(0), DegToRad(360));
	p->force = range(DegToRad(0), DegToRad(90));

	p = newParameter("r_dstk");
	p->title = "#it{r_{B}^{D*K}}";
	p->startvalue = 0.1;
	p->unit = "";
	p->scan = range(0.0, 0.9);
	p->phys = range(0, 1.);
	p->force = range(0.02, 0.16);

	// B+ -> DK*+
  p = newParameter("d_dkst");
	p->title = "#it{#delta_{B^{#plus}}^{D^{0}K*^{#plus}}}";
	p->startvalue = DegToRad(130);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(180));
	p->phys = range(DegToRad(0), DegToRad(360));
	p->force = range(DegToRad(0), DegToRad(90));

	p = newParameter("k_dkst");
	p->title = "#kappa_{B^{#plus}}^{D^{0}K*^{#plus}}";
	p->startvalue = 0.9;
	p->unit = "";
	p->scan = range(0, 2.5);
	p->phys = range(0, 1.5);
	p->force = range(0.001, 1);

	p = newParameter("r_dkst");
	p->title = "#it{r_{B^{#plus}}^{D^{0}K*^{#plus}}}";
	p->startvalue = 0.1;
	p->unit = "";
	p->scan = range(0., 0.2);
	p->phys = range(0, 1.);
	p->force = range(0.02, 0.16);

	// B0 -> D0Kst0
	p = newParameter("d_dkstz");
	p->title = "#it{#delta_{B}^{DK}}^{#scale[0.8]{#lower[0.4]{*}#lower[0.2]{0}}}";
	p->startvalue = DegToRad(200);
	p->unit = "Rad";
	p->scan = range(DegToRad(0), DegToRad(360));
	p->phys = range(DegToRad(0), DegToRad(360));
	p->force = range(DegToRad(180), DegToRad(360));

	p = newParameter("r_dkstz");
	p->title = "#it{r_{B}^{DK}}^{#scale[0.8]{#lower[0.4]{*}#lower[0.2]{0}}}";
	p->startvalue = 0.22;
	p->unit = "";
	p->scan = range(0, 1.);
	p->phys = range(0., 1.);
	p->force = range(0.001, 0.6);

	p = newParameter("k_dkstz");
	p->title = "#kappa_{B}^{DK*^{0}}";
	p->startvalue = 0.9;
	p->unit = "";
	p->scan = range(0, 2.5);
	p->phys = range(0, 1.5);
	p->force = range(0.001, 1);

}
