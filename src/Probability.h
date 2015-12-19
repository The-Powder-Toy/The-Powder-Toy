/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef tptmath_h
#define tptmath_h

// This file is used for EMP, to simulate many EMP going off at once at the end of the frame

#include <cmath>

namespace Probability
{
	// X ~ binomial(n,p), returns P(X>=1)
	// e.g. If a reaction has n chances of occurring, each time with probability p, this returns the probability that it occurs at least once.
	float binomial_gte1(int n, float p);
	float randFloat();

	class SmallKBinomialGenerator
	{
	protected:
		float *cdf;
		unsigned int maxK;
	public:
		// Class to generate numbers from a binomial distribution, up to a maximum value (maxK).
		// Results which would have been above maxK return maxK.
		// Note: maxK must be small, otherwise the method used in this class is inefficient. (n and p can be any valid value)
		SmallKBinomialGenerator(unsigned int n, float p, unsigned int maxK_);
		~SmallKBinomialGenerator();
		unsigned int calc(float randFloat);
	};
}

#endif
