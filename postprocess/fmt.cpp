#include "fmt.hpp"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>

#include <iostream>

/* <v> rounded at <digits> sig. figures, shift left into [1, 10^(1-<min>) )
 * Returns {number of left shift, v}
 * Anyway, (-inf, 0] -> 0, 0
 */
static auto offset_of(double v, int digits, int min) {
    if (v <= 0)
        return std::make_pair(std::numeric_limits<int>::max(), 0.0);
    auto n = 0;
    while (v >= 10 && n > min)
        v /= 10, n--;
    while (v < 1)
        v *= 10, n++;
    if (digits < std::numeric_limits<double>::digits10) {
        v *= std::pow(10, digits - 1);
        v = std::round(v);
        v /= std::pow(10, digits - 1);
        while (v >= 10 && n > min)
            v /= 10, n--;
    }
    return std::make_pair(n, v);
}

std::string fmt_fixed(double lb, double cn, double ub) {
    static constexpr auto UNC_DIGITS = 2;

    if (std::isinf(lb) || std::isnan(lb))
        throw std::runtime_error{ "lb not finite" };
    if (std::isinf(cn) || std::isnan(cn))
        throw std::runtime_error{ "cn not finite" };
    if (std::isinf(ub) || std::isnan(ub))
        throw std::runtime_error{ "ub not finite" };

    if (lb > cn) lb = cn;
    if (ub < cn) ub = cn;

    auto le = cn - lb, ue = ub - cn;
    auto [ole, vle] = offset_of(le, UNC_DIGITS, 1 - UNC_DIGITS);
    auto [oue, vue] = offset_of(ue, UNC_DIGITS, 1 - UNC_DIGITS);
    auto [ocn, vcn] = offset_of(std::fabs(cn), std::numeric_limits<int>::max(), std::numeric_limits<int>::min());

    int finest_precision;
    if (vle == 0 && vue == 0)
        finest_precision = std::numeric_limits<double>::max_digits10 + ocn; // fullest precision
    else if (vle != 0 && vue == 0)
        finest_precision = ole + UNC_DIGITS - 1;
    else if (vle == 0 && vue != 0)
        finest_precision = oue + UNC_DIGITS - 1;
    else
        finest_precision = std::max(ole, oue) + UNC_DIGITS - 1;
    if (finest_precision < 0)
        finest_precision = 0;

    std::stringstream ss;
    ss << std::fixed;
    if (vle == 0 && vue == 0) {
        if (cn == 0)
            ss << "0";
        else
            ss << std::setprecision(finest_precision) << cn;
        ss << "\u00b10";
        return ss.str();
    }
    if (std::signbit(cn))
        ss << '-';
    cn *= std::pow(10, finest_precision);
    cn = std::round(cn);
    cn /= std::pow(10, finest_precision);
    ss << std::setprecision(finest_precision) << cn;

    if (ole == oue && vle == vue) {
        ss << "\u00b1" << std::setprecision(ole + UNC_DIGITS - 1) << vle * std::pow(10, -ole);
    } else {
        if (vue != 0)
            ss << "+" << std::setprecision(oue + UNC_DIGITS - 1) << vue * std::pow(10, -oue);
        else
            ss << "+0";
        if (vle != 0)
            ss << "-" << std::setprecision(ole + UNC_DIGITS - 1) << vle * std::pow(10, -ole);
        else
            ss << "-0";
    }

    return ss.str();
}
