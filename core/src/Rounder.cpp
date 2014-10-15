#include "Rounder.h"

Rounder::Rounder(OptParser *arg, float cllo, float clhi, float central)
: m_cllo(cllo), m_clhi(clhi), m_central(central)
{
  assert(arg);
  this->arg = arg;
}

Rounder::~Rounder()
{}

int Rounder::getNsubdigits()
{
  if ( arg->digits>-1 ) return arg->digits;
	return TMath::Max(calcNsubdigits(fabs(m_central-m_cllo)), calcNsubdigits(fabs(m_central-m_clhi)));
}

float Rounder::CLlo()
{
	return Round(m_cllo, getNsubdigits());
}

float Rounder::CLhi()
{
	return Round(m_clhi, getNsubdigits());
}

float Rounder::central()
{
	return Round(m_central, getNsubdigits());
}

///
/// Compute rounded negative error.
/// Use the rounded central values and interval boundaries for this.
/// Would we round the +/- errors themselves, we'd be geting inconsistent intervals!
/// \return minus error (always positive)
///
float Rounder::errNeg()
{
  return fabs(central()-CLlo());
}

float Rounder::errPos()
{
  return fabs(central()-CLhi());
}
