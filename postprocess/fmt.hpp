#pragma once

#include <string>
/* fmt_fixed() takes three doubles, returns a string contains a value of measurement uncertainty.
 * Input lb, cn, and ub follows IEEE754 in double.
 * Output follows the following formats:
 * "<middle_value>+<upper uncertainty>-<lower uncertainty>", or
 * "<middle_value>±<measurement uncertainty>", if upper = lower.
 * All values display in the string keep 2 significant figures;
 * middle value follows the uncertainty with better precisions.
 * */
std::string fmt_fixed(double lb, double cn, double ub);
