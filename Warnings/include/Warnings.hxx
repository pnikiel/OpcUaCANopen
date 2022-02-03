#pragma once

#include <vector>
#include <string>
#include <map>

#include <Configuration.hxx>

namespace Warnings
{

struct Warning
{
    //std::string name;
    bool turnOn;
    bool turnOff;
    Warning(/*const std::string& _name*/): /*name(_name), */turnOn(false), turnOff(false) {}
};

void validateWarnings();
void passToConfiguration(Configuration::Warnings& warningsConfig);

extern std::map<std::string, Warning> WarningsDefinitions;

}