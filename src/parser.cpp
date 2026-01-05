#include <html-parser.hpp>
#include <error-handler.hpp>
#include <input-handler.hpp>
#include <unistd.h>
#include <print>

int main(int argc, char** argv) {
	processInput(argc, argv);
	std::string src = getSrcFromFile(argv[argc - 1]);
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &TERM_SIZE);
	std::print("{}", parseHTML(src));
	return EXIT_SUCCESS;
}