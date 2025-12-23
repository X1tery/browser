#pragma once
#include <sys/ioctl.h>
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>

extern const std::vector<std::string> SINGLETON_TAGS;
extern const std::unordered_map<std::string, std::string> COLOR_NAMES;
extern winsize TERM_SIZE;

class HTMLTag {
private:
    //std::unordered_map<std::string, std::string> attributes;
    //std::vector<std::variant<HTMLTag, std::string>> content;
    //std::vector<std::variant<HTMLTag, std::string>> tokenize(std::string source);  
public:
    std::unordered_map<std::string, std::string> attributes;
    std::vector<std::variant<HTMLTag, std::string>> content;
    std::string name;
    HTMLTag(std::string n, std::unordered_map<std::string, std::string> a, std::vector<std::variant<HTMLTag, std::string>> c);
};

typedef std::unordered_map<std::string, std::string> HTMLTagAttr;
typedef std::vector<std::variant<HTMLTag, std::string>> HTMLTagContent;

HTMLTagContent tokenizeHTML(std::string source);
std::string tagToStr(std::string tag, HTMLTagAttr attrs);
std::string parseTokens(HTMLTagContent tokens, HTMLTagAttr attrs);
std::string parseHTML(std::string source);