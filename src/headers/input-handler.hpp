#pragma once
#include <string>
#include <unordered_map>
#include <vector>

extern const std::string HELP_MSG;

extern std::unordered_map<std::string, std::string> OPTIONS;
extern const std::vector<std::string> NO_VAL_OPS;

void processInput(int argc, char** argv);
std::string getSrcFromFile(std::string file_name);