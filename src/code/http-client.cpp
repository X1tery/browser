#include <error-handler.hpp>
#include <http-client.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <cwctype>
#include <print>

HttpResponse parseHTTPResponse(std::string source) {
    HttpResponse response;
    response.src = source;
    size_t i = 0;
    while (i < source.size() && !std::iswspace(source[i])) {
        response.version.push_back(source[i]);
        i++;
    }
    while (i < source.size() && std::iswspace(source[i])) i++;
    while (i < source.size() && !std::iswspace(source[i])) {
        response.status.push_back(source[i]);
        i++;
    }
    while (i < source.size() && std::iswspace(source[i])) i++;
    while (i < source.size() - 1 && source[i] != '\r' && source[i + 1] != '\n') {
        response.phrase.push_back(source[i]);
        i++;
    }
    while (i < source.size() && std::iswspace(source[i])) i++;
    while (i < source.size() && std::iswspace(source[i])) {
        std::string header_name{}, header_val{};
        while (i < source.size() && source[i] != ':') {
            header_name.push_back(source[i]);
            i++;
        }
        i++;
        while (i < source.size() && std::iswspace(source[i])) i++;
        while (i < source.size() - 1 && source[i] != '\r' && source[i + 1] != '\n') {
            header_val.push_back(source[i]);
            i++;
        }
        i += 2;
    }
    while (i < source.size() && std::iswspace(source[i])) i++;
    while (i < source.size()) {
        response.body.push_back(source[i]);
        i++;
    }
    return response;
}

void translateUrl(std::string site_url, std::string& domain, std::string& location) {
    if (site_url.size() >= 7 && site_url.substr(0, 7) == "http://") {
        site_url.erase(site_url.begin(), site_url.begin() + 7);
    } else if (site_url.size() >= 8 && site_url.substr(0, 8) == "https://") {
        site_url.erase(site_url.begin(), site_url.begin() + 8);
    }
    if (site_url[site_url.size() - 1] != '/') site_url.push_back('/');
    for (size_t i = 0; i < site_url.size(); i++) {
        if (site_url[i] == '/') {
            domain = site_url.substr(0, i);
            location = site_url.substr(i);
            break;
        }
    }
}

std::string buildHTTPRequest(std::string domain, std::string location) {
    return "GET " + location + " HTTP/1.1\r\nHost: " + domain + "\r\nAccept: text/html\r\n\r\n";
}

std::string sendGET(std::string site_url) {
    std::string domain{};
    std::string location{};
    int sockfd;
    translateUrl(site_url, domain, location);
    addrinfo hints, *info, *inode;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    sockfd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (sockfd == -1) throw_error("failed to create socket");
    if (getaddrinfo(domain.c_str(), "80", &hints, &info) != 0) throw_error("failed to get address info for " + domain);
    int con_ret;
    for (inode = info; inode != nullptr; inode = inode->ai_next) {
        con_ret = connect(sockfd, inode->ai_addr, inode->ai_addrlen);
        if (con_ret == 0) break;
    }
    freeaddrinfo(info);
    if (con_ret == -1) {
        close(sockfd);
        throw_error("failed to connect to " + domain);
    }
    std::string http_request = buildHTTPRequest(domain, location);
    std::println("-----REQUEST-----\n{}", http_request);
    send(sockfd, http_request.c_str(), strlen(http_request.c_str()), 0);
    std::string http_response{};
    char response_buff[1028];
    int num_read = 0;
    while (true) {
        num_read = recv(sockfd, response_buff, 1027, 0);
        http_response.append(response_buff);
        if (num_read == -1) throw_error("failed to recv");
        else if (num_read == 0) break;
        std::println("{}", num_read);
        memset(&response_buff, 0, sizeof(response_buff));
    }
    std::println("-----RESPONSE-----\n{}", http_response);
    close(sockfd);
    return http_response;
}

std::string processResponse(std::string response_src) {
    HttpResponse response = parseHTTPResponse(response_src);
    switch (std::stoi(response.status, nullptr, 10)) { 
        default:
            break;
    }
    return response.body;
}