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

#include <numeric>
#include <cstdlib>
#include "Probability.h"

namespace Probability
{

float binomial_gte1(int n, float p)
{
	return 1.0f - std::pow(1.0f-p, n);
}

float randFloat()
{
	return static_cast<float>(rand())/RAND_MAX;
}

SmallKBinomialGenerator::SmallKBinomialGenerator(unsigned int n, float p, unsigned int maxK_)
{
	maxK = maxK_;
	cdf = new float[maxK];

	float *pdf = new float[maxK];
	// initial values, k=0:
	float pTerm = std::pow(1.0f-p, static_cast<float>(n)); // the p^k * (1-p)^(n-k) term
	unsigned int coeffN = 1, coeffD = 1; // (N / D) evaluates to the same result as the n!/(k!(n-k)!) term
	for (unsigned int k=0; k<maxK; k++)
	{
		pdf[k] = pTerm * (float(coeffN) / coeffD);
		pTerm *= p/(1.0f-p);
		coeffN *= n-k; // Part of the n! will no longer cancelled out by (n-k)!
		coeffD *= k+1; // k! (k+1 because this is for next k)
	}

	std::partial_sum(pdf, pdf+maxK, cdf);
	delete[] pdf;
}

SmallKBinomialGenerator::~SmallKBinomialGenerator()
{
	delete[] cdf;
}

unsigned int SmallKBinomialGenerator::calc(float randFloat)
{
	for (unsigned int k=0; k<maxK; k++)
	{
		if (randFloat<cdf[k])
			return k;
	}
	return maxK;
}

}
