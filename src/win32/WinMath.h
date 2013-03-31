// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Visit http://www.johndcook.com/stand_alone_code.html for the source of this code and more like it.

#ifndef GAMMA_H
#define GAMMA_H

// Note that the functions Gamma and LogGamma are mutually dependent.
double LogGamma(double);
double Gamma(double);
double LogOnePlusX(double x);

#ifdef _MSC_VER
// http://social.msdn.microsoft.com/Forums/en-US/Vsexpressvc/thread/25c923af-a824-40f8-8fd4-e5574bc147af/
double asinh(double value);

// http://stackoverflow.com/questions/15539116/atanh-arc-hyperbolic-tangent-function-missing-in-ms-visual-c
double atanh (double x); //implements: return (log(1+x) - log(1-x))/2
#endif

#endif

