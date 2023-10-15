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

#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#define _USE_MATH_DEFINES
#include <cmath>

#ifdef M_PI
constexpr float  TPT_PI_FLT = float(M_PI);
constexpr double TPT_PI_DBL = double(M_PI);
#else
constexpr float  TPT_PI_FLT = 3.14159265f;
constexpr double TPT_PI_DBL = 3.14159265358979323846;
#endif
