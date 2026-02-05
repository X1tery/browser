#include <error-handler.hpp>
#include <input-handler.hpp>
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
    while (i < source.size() && !std::iswspace(source[i])) {
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
        response.headers[header_name] = header_val;
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
    send(sockfd, http_request.c_str(), strlen(http_request.c_str()), 0);
    std::string http_response{};
    char response_buff[512];
    int num_read{0};
    bool is_cont_length{false}, is_encode_chunked{false};
    int cont_length{0};
    size_t cont_hdr_start{0}, cont_start{0};
    while (true) {
        num_read = recv(sockfd, response_buff, sizeof(response_buff) - 1, 0);
        http_response.append(response_buff);
        if (num_read == -1) throw_error("failed to recv");
        else if (num_read == 0) break;
        if (!is_cont_length && !is_encode_chunked && http_response.find("Transfer-Encoding: chunked") != std::string::npos) is_encode_chunked = true;
        else if (!is_cont_length && !is_encode_chunked && (cont_hdr_start = http_response.find("Content-Length: ")) != std::string::npos) {
            std::string temp_length{};
            for (int i = cont_hdr_start + 16; i < http_response.size(); i++) {
                if (!std::isdigit(http_response[i])) {
                    is_cont_length = true;
                    cont_length = std::stoi(temp_length, nullptr, 10);
                    break;
                }
                temp_length.push_back(http_response[i]);
            }
        }
        if (is_cont_length && cont_start == 0) {
            cont_start = http_response.find("\r\n\r\n");
            if (cont_start == std::string::npos) cont_start = 0;
            else cont_start += 4;
        }
        if (is_cont_length && http_response.substr(cont_start).size() == cont_length) break;
        else if (is_encode_chunked && http_response.substr(http_response.size() - 5) == "0\r\n\r\n") break;
        memset(response_buff, 0, sizeof(response_buff));
    }
    memset(response_buff, 0, sizeof(response_buff));
    close(sockfd);
    return http_response;
}

std::string processResponse(std::string response_src) {
    HttpResponse response = parseHTTPResponse(response_src);
    switch (response.status[0]) {
        case '1':
            break;
        case '2':
            break;
        case '3':
            while (response.status[0] == '3') {
                std::println("redirecting to: {}", response.headers["Location"]);
                response = parseHTTPResponse(sendGET(response.headers["Location"]));
            }
            break;
        case '4':
            throw_error(response.status + " " + response.phrase);
            break;
        case '5':
            throw_error(response.status + " " + response.phrase);
            break;
    }
    if (OPTIONS["-H"].size() || OPTIONS["--http"].size()) {
        std::string http_headers{};
        for (int i = 0; i < response.src.size() - 3; i++) {
            if (response.src[i] == '\r' && response.src[i + 1] == '\n' && response.src[i + 2] == '\r' && response.src[i + 3] == '\n') break;
            http_headers.push_back(response.src[i]);
        }
        std::println("{}\n", http_headers);
    }
    return response.body;
}