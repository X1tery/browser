#include <html-parser.hpp>
#include <http-client.hpp>
#include <error-handler.hpp>
#include <input-handler.hpp>
#include <unistd.h>
#include <print>

int main(int argc, char** argv) {
	processInput(argc, argv);
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &TERM_SIZE);
	std::string response = sendGET(argv[argc - 1]);
	std::string body = processResponse(response);
	std::println("{}", parseHTML(body));
	return EXIT_SUCCESS;
}