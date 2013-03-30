// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Visit http://www.johndcook.com/stand_alone_code.html for the source of this code and more like it.

#include "Win32Setup.h"

#include "libs.h"
#include "Pi.h"
#include <cmath>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "WinMath.h"

// Note that the functions Gamma and LogGamma are mutually dependent.

double Gamma
(
    double x    // We require x > 0
)
{
	if (x <= 0.0)
	{
		std::stringstream os;
        os << "Invalid input argument " << x <<  ". Argument must be positive.";
        throw std::invalid_argument( os.str() );
	}

    // Split the function domain into three intervals:
    // (0, 0.001), [0.001, 12), and (12, infinity)

    ///////////////////////////////////////////////////////////////////////////
    // First interval: (0, 0.001)
	//
	// For small x, 1/Gamma(x) has power series x + gamma x^2  - ...
	// So in this range, 1/Gamma(x) = x + gamma x^2 with error on the order of x^3.
	// The relative error over this interval is less than 6e-7.

	const double gamma = 0.577215664901532860606512090; // Euler's gamma constant

    if (x < 0.001)
        return 1.0/(x*(1.0 + gamma*x));

    ///////////////////////////////////////////////////////////////////////////
    // Second interval: [0.001, 12)

	if (x < 12.0)
    {
        // The algorithm directly approximates gamma over (1,2) and uses
        // reduction identities to reduce other arguments to this interval.

		double y = x;
        int n = 0;
        bool arg_was_less_than_one = (y < 1.0);

        // Add or subtract integers as necessary to bring y into (1,2)
        // Will correct for this below
        if (arg_was_less_than_one)
        {
            y += 1.0;
        }
        else
        {
            n = static_cast<int> (floor(y)) - 1;  // will use n later
            y -= n;
        }

        // numerator coefficients for approximation over the interval (1,2)
        static const double p[] =
        {
            -1.71618513886549492533811E+0,
             2.47656508055759199108314E+1,
            -3.79804256470945635097577E+2,
             6.29331155312818442661052E+2,
             8.66966202790413211295064E+2,
            -3.14512729688483675254357E+4,
            -3.61444134186911729807069E+4,
             6.64561438202405440627855E+4
        };

        // denominator coefficients for approximation over the interval (1,2)
        static const double q[] =
        {
            -3.08402300119738975254353E+1,
             3.15350626979604161529144E+2,
            -1.01515636749021914166146E+3,
            -3.10777167157231109440444E+3,
             2.25381184209801510330112E+4,
             4.75584627752788110767815E+3,
            -1.34659959864969306392456E+5,
            -1.15132259675553483497211E+5
        };

        double num = 0.0;
        double den = 1.0;
        int i;

        double z = y - 1;
        for (i = 0; i < 8; i++)
        {
            num = (num + p[i])*z;
            den = den*z + q[i];
        }
        double result = num/den + 1.0;

        // Apply correction if argument was not initially in (1,2)
        if (arg_was_less_than_one)
        {
            // Use identity gamma(z) = gamma(z+1)/z
            // The variable "result" now holds gamma of the original y + 1
            // Thus we use y-1 to get back the orginal y.
            result /= (y-1.0);
        }
        else
        {
            // Use the identity gamma(z+n) = z*(z+1)* ... *(z+n-1)*gamma(z)
            for (i = 0; i < n; i++)
                result *= y++;
        }

		return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Third interval: [12, infinity)

    if (x > 171.624)
    {
		// Correct answer too large to display. Force +infinity.
		double temp = DBL_MAX;
		return temp*2.0;
    }

    return exp(LogGamma(x));
}

double LogGamma
(
    double x    // x must be positive
)
{
	if (x <= 0.0)
	{
		std::stringstream os;
        os << "Invalid input argument " << x <<  ". Argument must be positive.";
        throw std::invalid_argument( os.str() );
	}

    if (x < 12.0)
    {
        return log(fabs(Gamma(x)));
    }

	// Abramowitz and Stegun 6.1.41
    // Asymptotic series should be good to at least 11 or 12 figures
    // For error analysis, see Whittiker and Watson
    // A Course in Modern Analysis (1927), page 252

    static const double c[8] =
    {
		 1.0/12.0,
		-1.0/360.0,
		1.0/1260.0,
		-1.0/1680.0,
		1.0/1188.0,
		-691.0/360360.0,
		1.0/156.0,
		-3617.0/122400.0
    };
    double z = 1.0/(x*x);
    double sum = c[7];
    for (int i=6; i >= 0; i--)
    {
        sum *= z;
        sum += c[i];
    }
    double series = sum/x;

    static const double halfLogTwoPi = 0.91893853320467274178032973640562;
    double logGamma = (x - 0.5)*log(x) - x + halfLogTwoPi + series;
	return logGamma;
}

// Can delete these functions and the #include for <iostream> if not testing.

void TestGamma()
{
	struct TestCase
	{
		double input;
		double expected;
	};

	TestCase test[] =
	{
		// Test near branches in code for (0, 0.001), [0.001, 12), (12, infinity)
		{1e-20, 1e+20},
		{2.19824158876e-16, 4.5490905327e+15},	// 0.99*DBL_EPSILON
		{2.24265050974e-16, 4.45900953205e+15}, // 1.01*DBL_EPSILON
		{0.00099, 1009.52477271},
		{0.00100, 999.423772485},
		{0.00101, 989.522792258},
		{6.1, 142.451944066},
		{11.999, 39819417.4793},
		{12, 39916800.0},
		{12.001, 40014424.1571},
		{15.2, 149037380723.0}
	};

	size_t numTests = sizeof(test) / sizeof(TestCase);

	double worst_absolute_error = 0.0;
	double worst_relative_error = 0.0;
	size_t worst_absolute_error_case = 0;
	size_t worst_relative_error_case = 0;

	for (size_t t = 0; t < numTests; t++)
	{
		double computed = Gamma( test[t].input );
		double absolute_error = fabs(computed - test[t].expected);
		double relative_error = absolute_error / test[t].expected;

		if (absolute_error > worst_absolute_error)
		{
			worst_absolute_error = absolute_error;
			worst_absolute_error_case = t;
		}

		if (relative_error > worst_relative_error)
		{
			worst_relative_error = absolute_error;
			worst_relative_error_case = t;
		}
	}

	size_t t = worst_absolute_error_case;
	double x = test[t].input;
	double y = test[t].expected;
	std::cout << "Worst absolute error: "
			  << fabs(Gamma(x) - y)
			  << "\nGamma( "
		      <<  x
			  << ") computed as "
			  << Gamma(x)
			  << " but exact value is "
			  << y
			  << "\n";

	t = worst_relative_error_case;
	x = test[t].input;
	y = test[t].expected;
	std::cout << "Worst relative error: "
			  << (Gamma(x) - y) / y
		      << "\nGamma( "
		      <<  x
			  << ") computed as "
			  << Gamma(x)
			  << " but exact value is "
			  << y
			  << "\n";
}

void TestLogGamma()
{
	struct TestCase
	{
		double input;
		double expected;
	};

	TestCase test[] =
	{
		{1e-12, 27.6310211159},
		{0.9999, 5.77297915613e-05},
		{1.0001, -5.77133422205e-05},
		{3.1, 0.787375083274},
		{6.3, 5.30734288962},
		{11.9999, 17.5020635801},
		{12, 17.5023078459},
		{12.0001, 17.5025521125},
		{27.4, 62.5755868211}
	};

	size_t numTests = sizeof(test) / sizeof(TestCase);

	double worst_absolute_error = 0.0;
	double worst_relative_error = 0.0;
	size_t worst_absolute_error_case = 0;
	size_t worst_relative_error_case = 0;

	for (size_t t = 0; t < numTests; t++)
	{
		double computed = LogGamma( test[t].input );
		double absolute_error = fabs(computed - test[t].expected);
		double relative_error = absolute_error / test[t].expected;

		if (absolute_error > worst_absolute_error)
		{
			worst_absolute_error = absolute_error;
			worst_absolute_error_case = t;
		}

		if (relative_error > worst_relative_error)
		{
			worst_relative_error = absolute_error;
			worst_relative_error_case = t;
		}
	}

	size_t t = worst_absolute_error_case;
	double x = test[t].input;
	double y = test[t].expected;
	std::cout << "Worst absolute error: "
			  << fabs(LogGamma(x) - y)
			  << "\nGamma( "
		      <<  x
			  << ") computed as "
			  << LogGamma(x)
			  << " but exact value is "
			  << y
			  << "\n";

	t = worst_relative_error_case;
	x = test[t].input;
	y = test[t].expected;
	std::cout << "Worst relative error: "
			  << (LogGamma(x) - y) / y
		      << "\nGamma( "
		      <<  x
			  << ") computed as "
			  << LogGamma(x)
			  << " but exact value is "
			  << y
			  << "\n";
}

// compute log(1+x) without losing precision for small values of x
double LogOnePlusX(double x)
{
    if (x <= -1.0)
    {
        std::stringstream os;
        os << "Invalid input argument (" << x
           << "); must be greater than -1.0";
        throw std::invalid_argument( os.str() );
    }

    if (fabs(x) > 1e-4)
    {
        // x is large enough that the obvious evaluation is OK
        return log(1.0 + x);
    }

    // Use Taylor approx. log(1 + x) = x - x^2/2 with error roughly x^3/3
    // Since |x| < 10^-4, |x|^3 < 10^-12, relative error less than 10^-8

    return (-0.5*x + 1.0)*x;
}

#ifdef _MSC_VER
// http://social.msdn.microsoft.com/Forums/en-US/Vsexpressvc/thread/25c923af-a824-40f8-8fd4-e5574bc147af/
double asinh(double value) {
	double returned;
	if(value>0.0)
		returned = log(value + sqrt(value * value + 1.0));
	else
		returned = -log(-value + sqrt(value * value + 1.0));
	return returned;
}

// http://stackoverflow.com/questions/15539116/atanh-arc-hyperbolic-tangent-function-missing-in-ms-visual-c
double atanh (double x) //implements: return (log(1+x) - log(1-x))/2
{
	return (LogOnePlusX(x) - LogOnePlusX(-x))/2.0;
}
#endif
