#include "Parameter.h"


Parameter::Parameter()
{
  name = "not initialized";
  title = "not initialized";
  unit = "not initialized";
  startvalue = 0;
  Parameter::Range r = {-1e4, 1e4};
  free = r;
  phys = r;
  scan = r;
  force = r;
  bboos = r;
}


void Parameter::Print()
{
  cout << " name       = " << name << endl;
  cout << " title      = " << title << endl;
  cout << " unit       = " << unit  << endl;
  cout << " startvalue = " << startvalue  << endl;
  cout << " phys       = " << phys.min  << " ... " << phys.max << endl;
  cout << " scan       = " << scan.min  << " ... " << scan.max<< endl;
  cout << " force      = " << force.min << " ... " << force.max << endl;
  cout << " bboos      = " << bboos.min << " ... " << bboos.max << endl;
  cout << " free       = " << free.min  << " ... " << free.max << endl;
}
