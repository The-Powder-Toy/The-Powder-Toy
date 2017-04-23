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

#ifndef TPT_MINMAX_H
#define TPT_MINMAX_H

#ifdef _MSC_VER
// less than VS2013. Untested since I don't use VS2012 anymore
#if _MSC_VER < 1800
#define fmin min
#define fminf min
#define fmax max
#define fmaxf max
#else
// >= VS2013
#include <algorithm>
#define NOMINMAX
#endif
#else
// not using visual studio, std::min and std::max are normal
#include <algorithm>
#endif

#endif
