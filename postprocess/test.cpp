#include <bandit/bandit.h>

#include "fmt.hpp"

using namespace snowhouse;
using namespace bandit;

go_bandit([]{
    describe("fmt_fixed", []{
        describe("zero", []{
            it("easy", []{ AssertThat(fmt_fixed(0, 0, 0), Equals("0±0")); });
            it("weird1", []{ AssertThat(fmt_fixed(-0, 0, 0), Equals("0±0")); });
            it("weird2", []{ AssertThat(fmt_fixed(-0, -0, 0), Equals("0±0")); });
            it("giant", []{ AssertThat(fmt_fixed(1e25, 1e25, 1e25), Equals("10000000000000000905969664±0")); });
            it("giant2", []{ AssertThat(fmt_fixed(1e18, 1e18+1, 1e18+2), Equals("1000000000000000000±0")); });
            it("large", []{ AssertThat(fmt_fixed(1234.5678, 1234.5678, 1234.5678), Equals("1234.56780000000003±0")); });
            it("small", []{ AssertThat(fmt_fixed(0.005678, 0.005678, 0.005678), Equals("0.00567799999999999989±0")); });
            it("normal1", []{ AssertThat(fmt_fixed(0, 1, 2), Equals("1.0±1.0")); });
            it("normal2", []{ AssertThat(fmt_fixed(0.1, 1, 1.9), Equals("1.00±0.90")); });
        });
        describe("symmetric", []{
            it("easy", [] { AssertThat(fmt_fixed(4, 5, 6), Equals("5.0±1.0")); });
            it("half", []{ AssertThat(fmt_fixed(3.8501, 5, 6.1499), Equals("5.0±1.1")); });
            it("large", [] { AssertThat(fmt_fixed(994, 995, 996), Equals("995.0±1.0")); });
            it("small", [] { AssertThat(fmt_fixed(0.004, 0.005, 0.006), Equals("0.0050±0.0010")); });
            it("signed_small", [] { AssertThat(fmt_fixed(-0.00010, 0, 0.00010), Equals("0.00000±0.00010")); });
            it("signed_large", [] { AssertThat(fmt_fixed(-1000, 0, 1000), Equals("0±1000")); }); // TODO
            it("edge_rounding1", [] { AssertThat(fmt_fixed(99.0001,100,100.9999), Equals("100.0±1.0")); });
        });
        describe("asymmetric", []{
            it("easy", [] { AssertThat(fmt_fixed(4, 5, 8), Equals("5.0+3.0-1.0")); });
            it("easy_neg", [] { AssertThat(fmt_fixed(-8, -5, -4), Equals("-5.0+1.0-3.0")); });
            it("large", [] { AssertThat(fmt_fixed(994, 995, 998), Equals("995.0+3.0-1.0")); });
            it("small", [] { AssertThat(fmt_fixed(0.004, 0.005, 0.008), Equals("0.0050+0.0030-0.0010")); });
            it("margin", [] { AssertThat(fmt_fixed(-0, 0, 0.0001), Equals("0.00000+0.00010-0")); });
            it("margin2", [] {AssertThat(fmt_fixed(-0.0001, 0, 0), Equals("0.00000+0-0.00010")); });
            it("signed_small", [] { AssertThat(fmt_fixed(-10, 0, 5.0), Equals("0.0+5.0-10")); });
            it("signed_large", [] { AssertThat(fmt_fixed(-0.010, 0, 1000), Equals("0.000+1000-0.010")); });
            it("inf_upper", [] { AssertThrows(std::runtime_error, \
                                    fmt_fixed(0, 0, std::numeric_limits<double>::infinity())); });
            it("inf_center", [] { AssertThrows(std::runtime_error, fmt_fixed(0, std::numeric_limits<double>::infinity(), \
                                    std::numeric_limits<double>::infinity())); });
            it("inf_lower", [] { AssertThrows(std::runtime_error, fmt_fixed(std::numeric_limits<double>::infinity(), \
                                    std::numeric_limits<double>::infinity(), \
                                    std::numeric_limits<double>::infinity())); });
            it("nan_upper", [] { AssertThrows(std::runtime_error, \
                                    fmt_fixed(0, 0, std::numeric_limits<double>::quiet_NaN())); });
            it("nan_center", [] { AssertThrows(std::runtime_error, fmt_fixed(0, std::numeric_limits<double>::quiet_NaN(), \
                                    0)); });
            it("nan_lower", [] { AssertThrows(std::runtime_error, fmt_fixed(std::numeric_limits<double>::quiet_NaN(), 0, 0)); });
            it("large_upper", [] {AssertThat(fmt_fixed(0, 0.1, 1e18), Equals("0.10+1000000000000000000-0.10")); });
            it("large_lower", [] {AssertThat(fmt_fixed(-1e18,0.001,0.1), Equals("0.001+0.099-1000000000000000000")); });

        });
    });
});

int main(int argc, char* argv[]) {
    return bandit::run(argc, argv);
}
