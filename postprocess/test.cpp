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
            it("large", []{ AssertThat(fmt_fixed(1234.5678, 1234.5678, 1234.5678), Equals("1234.56780000000003±0")); });
            it("small", []{ AssertThat(fmt_fixed(0.005678, 0.005678, 0.005678), Equals("0.00567799999999999989±0")); });
        });
        describe("symmetric", []{
            it("easy", [] { AssertThat(fmt_fixed(4, 5, 6), Equals("5.0±1.0")); });
            it("half", []{ AssertThat(fmt_fixed(3.8501, 5, 6.1499), Equals("5.0±1.1")); });
            it("large", [] { AssertThat(fmt_fixed(994, 995, 996), Equals("995.0±1.0")); });
            it("small", [] { AssertThat(fmt_fixed(0.004, 0.005, 0.006), Equals("0.0050±0.0010")); });
            it("signed_small", [] { AssertThat(fmt_fixed(-0.00010, 0, 0.00010), Equals("0.00000±0.00010")); });
            it("signed_large", [] { AssertThat(fmt_fixed(-1000, 0, 1000), Equals("0±1000")); }); // TODO
        });
        describe("asymmetric", []{
            it("easy", [] { AssertThat(fmt_fixed(4, 5, 8), Equals("5.0+3.0-1.0")); });
            it("large", [] { AssertThat(fmt_fixed(994, 995, 998), Equals("995.0+3.0-1.0")); });
            it("small", [] { AssertThat(fmt_fixed(0.004, 0.005, 0.008), Equals("0.0050+0.0030-0.0010")); });
            it("margin", [] { AssertThat(fmt_fixed(-0, 0, 0.0001), Equals("0.00000+0.00010-0")); });
            it("signed_small", [] { AssertThat(fmt_fixed(-10, 0, 5.0), Equals("0.0+5.0-10")); });
            it("signed_large", [] { AssertThat(fmt_fixed(-0.010, 0, 1000), Equals("0.000+1000-0.010")); });
        });
    });
});

int main(int argc, char* argv[]) {
    return bandit::run(argc, argv);
}
