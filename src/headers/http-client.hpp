#pragma once
#include <string>
#include <unordered_map>

struct HttpResponse {
    std::string src;
    std::string version;
    std::string status;
    std::string phrase;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

HttpResponse parseHTTPResponse(std::string source);
std::string buildHTTPRequest(std::string domain, std::string location);
std::string sendGET(std::string site_url);
std::string processResponse(std::string response);