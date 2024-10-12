#pragma once
#include <nlohmann/json.hpp>
#include "facade.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(LogicMethod, {
    { LogicMethod::Passive, "PL" },
    { LogicMethod::Single, "SL" },
    { LogicMethod::SingleExtended, "SLE" },
    { LogicMethod::Double, "DL" },
    { LogicMethod::DoubleExtended, "DLE" },
    { LogicMethod::Full, "FL" },
})

nlohmann::json to_json(const Configuration &cfg);

std::string to_string(const std::vector<HeuristicMethod> &dt);

struct NanoTimer
{
    NanoTimer();
    void stop();
    double seconds() const;

private:
    timespec begin, end;
};
