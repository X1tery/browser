#include <error-handler.hpp>
#include <print>
#include <cstdio>

void throw_error(std::string error_msg) {
    std::println(stderr, "{} :(", error_msg);
    exit(EXIT_FAILURE);
}