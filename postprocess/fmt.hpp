#pragma once

#include <string>

/* Present a center value with uncertainty.
 *
 * lb: lower bound
 * cn: the center value
 * ub: upper bound
 * output: <string> describe at Line 26.
 *
 * fmt_fixed() first computes lower and upper uncertainty (|cn - lb| and |cn - ub|,)
 * which are then independently expressed in two significant figures (round if necessary.)
 * In the extreme case that an uncertainty is larger than 1, round it to integers.
 * If an uncertainty is zero, show plain "0". Never use scientific notation.
 *
 * It then expresses the center value:
 * * Prefix "-" if it is negative (including -0);
 * * If at least one uncertainty is non-zero, match the precision of center value to
 *   the precision of the more-precise uncertainty;
 * * If both uncertainties are zero, show both of them plain "0",
 *   while express center value in 18 significant figures (round if necessary.)
 *   In the extreme case that the center value is larger than 10^18, round it to integers.
 *   If center value is 0, show plain "0". Never use scientific notation.
 *
 * Finally, if the two uncertainty are *written* the same (i.e. they may differ originally,
 * but are the same when expressed as above), present the final result as:
 *     [-]?<center-value>±<uncertainty>
 * otherwise, present as:
 *     [-]?<center-value>+<upper-uncertainty>-<lower-uncertainty>
 *
 * It shall throw std::runtime_error when any of the inputs are inf or NaN.
 * Its behavior is unspecified when cn < lb or ub < cn.
 *
 */
std::string fmt_fixed(double lb, double cn, double ub);
