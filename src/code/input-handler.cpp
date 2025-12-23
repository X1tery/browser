#include <input-handler.hpp>
#include <error-handler.hpp>
#include <algorithm>
#include <fstream>
#include <string>
#include <print>
#include <format>

const std::string HELP_MSG {"\
Usage:\n\
\t-h, --help\n\
\tShow this message\
"};

std::unordered_map<std::string, bool> OPTIONS {
	{"-h", false},
    {"--help", false}
};

void processInput(int argc, char** argv) {
for (int i = 1; i < argc - 1; i++) {
        auto pos = OPTIONS.find(argv[i]);
		if (pos != OPTIONS.end()) {
		    OPTIONS[argv[i]] = true;
        } else {
			throw_error(std::format("unknown option \"{}\"", argv[i]));
		}
	}
    if (OPTIONS["-h"] || OPTIONS["--help"] || argc < 2 || (argc == 2 && (static_cast<std::string>(argv[1]) == "-h" || static_cast<std::string>(argv[1]) == "--help"))) {
        std::println("{}", HELP_MSG);
        exit(EXIT_SUCCESS);
    }
}

std::string getSrcFromFile(std::string file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) throw_error(std::format("file \"{}\" not found", file_name));
    std::string source, line;
    while (std::getline(file, line)) {
        size_t i;
        for (i = 0; i != line.size() && std::iswspace(line[i]); i++);
        if (i == line.size()) continue;
        size_t j;
        for (j = line.size(); std::iswspace(line[j - 1]); j--);
        source.append(line.substr(i, j - i));
    }
    return source;
}