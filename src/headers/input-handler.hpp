#pragma once
#include <string>
#include <unordered_map>

extern const std::string HELP_MSG;

extern std::unordered_map<std::string, bool> OPTIONS;

void processInput(int argc, char** argv);
std::string getSrcFromFile(std::string file_name);