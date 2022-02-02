#pragma once

#include <vector>
#include <string>

namespace Warnings
{

struct Warning
{
    std::string name;
    bool turnOn;
    bool turnOff;
    Warning(const std::string& _name): name(_name), turnOn(false), turnOff(false) {}
};

void validateWarnings();
extern std::vector<Warning> WarningsDefinitions;

}