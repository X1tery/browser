#include <input-handler.hpp>
#include <error-handler.hpp>
#include <algorithm>
#include <fstream>

#include <string>
#include <print>
#include <format>

const std::string HELP_MSG {"\
usage:\n\
\n\
{COMMAND} [OPTIONS] {URL/PATH}\n\
\n\
options:\n\
\n\
\t-h, --help\n\
\tshow this message\n\
\n\
\t-s , --source\n\
\tprint the html source instead of the parsed page\n\
\n\
\t-H, --http\n\
\tinclude the http headers received\n\
\n\
made by @X1tery\
"};

std::unordered_map<std::string, std::string> OPTIONS {
	{"-h", ""},
    {"--help", ""}, 
    {"-s", ""},
    {"--source", ""},
    {"-H", ""},
    {"--http", ""}
};

const std::vector<std::string> NO_VAL_OPS {
    "-h",
    "--help",
    "-s",
    "--source",
    "-H",
    "--http"
};

void processInput(int argc, char** argv) {
for (int i = 1; i < argc - 1; i++) {
        auto pos = OPTIONS.find(argv[i]);
		if (pos != OPTIONS.end()) {
            if (std::count(NO_VAL_OPS.begin(), NO_VAL_OPS.end(), std::string(argv[i]))) {
                if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
                    std::println("{}", HELP_MSG);
                    exit(EXIT_SUCCESS);
                }
                OPTIONS[argv[i]] = "1";
            } else {
                if (i < argc - 2) OPTIONS[argv[i]] = argv[i + 1];
                else throw_error(std::format("wrong usage of option \"{}\"", argv[i]));
                i++;
            }
        } else {
			throw_error(std::format("unknown option \"{}\"", argv[i]));
		}
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