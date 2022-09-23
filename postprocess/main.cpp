#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <wstp.h>

#include "fmt.hpp"

using namespace std::string_literals;

struct info_t {
    long pass, fail;
    double cost; // cpu-second
    double lb, cn, ub;
    nlohmann::json config;
};

void to_json(nlohmann::json &j, const info_t &p) {
    j = nlohmann::json{
            { "pass", p.pass },
            { "fail", p.fail },
            { "cost", p.cost },
            { "lb", p.lb },
            { "cn", p.cn },
            { "ub", p.ub },
    };
}

int main(int argc, char *argv[]) {
    auto env = WSInitialize(nullptr);
    if (!env)
        throw std::runtime_error{ "WSInitialize" };

    int err;
    WSLINK link = WSOpenArgcArgv(env, argc, argv, &err);
    if (!link || err != WSEOK)
        throw std::runtime_error{ "WSOpenArgcArgv: " + std::to_string(err) };

    std::cerr << "Issuing WSActivate\n";
    if (!WSActivate(link))
        throw std::runtime_error{ "WSActivate: "s + WSErrorMessage(link) };

    if (!WSPutFunction(link, "ToExpression", 1))
        throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
    if (!WSPutString(link, R"(
lb[x_, n_, \[Alpha]_] := {If[x == 0, 0, 1 - InverseCDF[BetaDistribution[n - x + 1, x], 1 - \[Alpha]]], x/n, 1};
ub[x_, n_, \[Alpha]_] := {0, x/n, If[x == n, 1, 1 - InverseCDF[BetaDistribution[n - x, x + 1], \[Alpha]]]};
bi[x_, n_, \[Alpha]_] := If[0 < x < n, {1 - InverseCDF[BetaDistribution[n - x + 1, x], 1 - \[Alpha]/2], x/n, 1 - InverseCDF[BetaDistribution[n - x, x + 1], \[Alpha]/2]}, If[0 == x, ub[x, n, \[Alpha]], lb[x, n, \[Alpha]]]];
bis[{p_, f_}] := N@bi[p, p + f, 0.05];
)"))
        throw std::runtime_error{ "WSPutString: "s + WSErrorMessage(link) };
    if (!WSEndPacket(link))
        throw std::runtime_error{ "WSEndPacket: "s + WSErrorMessage(link) };
    std::map<std::string, info_t> dic;

    std::cerr << "Loading file\n";
    for (std::string line; std::getline(std::cin, line);) {
        auto j = nlohmann::json::parse(line);
        const auto &s = j["string"].get<std::string>();
        auto lb = dic.lower_bound(s);
        if (lb == dic.end() || dic.key_comp()(s, lb->first))
            lb = dic.insert(lb, decltype(dic)::value_type{ s, {} });
        lb->second.pass += j["result"]["pass"].get<long>();
        lb->second.fail += j["result"]["fail"].get<long>();
        lb->second.cost += j["exec"]["duration"].get<double>() * j["exec"]["cpu"].get<int>();
        lb->second.config = j["config"];
    }

    std::cerr << "Sending requests\n";
    if (!WSPutFunction(link, "EvaluatePacket", 1))
        throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
    if (!WSPutFunction(link, "Parallelize", 1))
        throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
    if (!WSPutFunction(link, "Map", 2))
        throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
    if (!WSPutSymbol(link, "bis"))
        throw std::runtime_error{ "WSPutSymbol: "s + WSErrorMessage(link) };
    if (!WSPutFunction(link, "List", (int) dic.size()))
        throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
    for (auto &[k, v]: dic) {
        if ((v.pass + v.fail) % 100000000)
            std::cerr << "Warning: " << k << " is " << v.pass + v.fail << " not N*1e8\n";
        if (!WSPutFunction(link, "List", 2))
            throw std::runtime_error{ "WSPutFunction: "s + WSErrorMessage(link) };
        WSPutInteger64(link, v.pass);
        WSPutInteger64(link, v.fail);
    }
    if (!WSPutSymbol(link, "InputForm"))
        throw std::runtime_error{ "WSPutSymbol: "s + WSErrorMessage(link) };
    if (!WSEndPacket(link))
        throw std::runtime_error{ "WSEndPacket: "s + WSErrorMessage(link) };
    if (!WSFlush(link))
        throw std::runtime_error{ "WSFlush: "s + WSErrorMessage(link) };

    std::cerr << "Receiving answers\n";
    nlohmann::json answer;
    for (auto &[k, v]: dic) {
        again:
        switch (WSGetNext(link)) {
            // case WSTKSTR:
            //     const char *str;
            //     if (!WSGetString(link, &str))
            //         throw std::runtime_error{ "WSGetString: "s + WSErrorMessage(link) };
            //     puts(str);
            //     goto again;
            case WSTKINT:
            case WSTKREAL:
                if (!WSGetReal64(link, &v.lb))
                    throw std::runtime_error{ "WSGetReal64: "s + WSErrorMessage(link) };
                if (!WSGetReal64(link, &v.cn))
                    throw std::runtime_error{ "WSGetReal64: "s + WSErrorMessage(link) };
                if (!WSGetReal64(link, &v.ub))
                    throw std::runtime_error{ "WSGetReal64: "s + WSErrorMessage(link) };
                break;
            default:
                goto again;
        }
        answer[k]["n"] = v.pass + v.fail;
        answer[k]["rate"] = fmt_fixed(100 * v.lb, 100 * v.cn, 100 * v.ub) + "%";
        answer[k]["speed"] = v.cost / static_cast<double>(v.pass + v.fail);
        answer[k]["raw"] = v;
        answer[k]["config"] = std::move(v.config);
    }

    WSDeinitialize(env);

    std::cout << answer;

    return 0;
}
