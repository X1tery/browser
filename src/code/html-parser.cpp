#include <html-parser.hpp>
#include <algorithm>
#include <unistd.h>
#include <print>
#include <cwctype>

const std::vector<std::string> SINGLETON_TAGS{
    "area",
    "base",
    "br",
    "col",
    "embed",
    "hr",
    "img",
    "input",
    "link",
    "meta",
    "param",
    "source",
    "track",
    "wbr"
};

const std::unordered_map<std::string, std::string> COLOR_NAMES {
    {"red", "#FF0000"},
    {"cyan", "#00FFFF"},
    {"aqua", "#00FFFF"},
    {"blue", "#0000FF"},
    {"darkblue", "#0000A0"},
    {"lightblue", "#ADD8E6"},
    {"purple", "#800080"},
    {"yellow", "#FFFF00"},
    {"lime", "#00FF00"},
    {"fuchsia", "#FF00FF"},
    {"white", "#FFFFFF"},
    {"silver", "#C0C0C0"},
    {"grey", "#808080"},
    {"black", "#000000"},
    {"orange", "#FFA500"},
    {"brown", "#A52A2A"},
    {"maroon", "#800000"},
    {"green", "#008000"},
    {"olive", "#808000"},
    {"teal", "#008080"},
    {"navy", "#000080"},
};

winsize TERM_SIZE;

HTMLTag::HTMLTag(std::string n, HTMLTagAttr a, HTMLTagContent c) : name(n), attributes(a), content(c) {};

HTMLTagContent tokenizeHTML(std::string source) {
    HTMLTagContent result;
    for (size_t i = 0; i < source.size(); i++) {
        switch (source[i]) {
            case '<':
            {
                if (source.substr(i, 4) == "<!--") {
                    while (i < source.size() - 3 && source.substr(i, 3) != "-->") i++;
                    i += 2;
                    break;
                }
                if (source.substr(i, 9) == "<!DOCTYPE") {
                    while (i < source.size() - 1 && source[i] != '>') i++;
                    break;
                }
                i++;
                std::string tag_name;
                while (i < source.size() && !std::iswspace(source[i]) && source[i] != '>') {
                    tag_name.push_back(source[i]);
                    i++;
                }
                if (i >= source.size()) break;
                bool is_singleton = false;
                if (std::count_if(SINGLETON_TAGS.begin(), SINGLETON_TAGS.end(), [tag_name](auto it){
                    return tag_name == it || tag_name == it + "/";
                })) is_singleton = true;
                HTMLTagAttr tag_attributes{};
                while (i < source.size() && source[i] != '>') {
                    i++;
                    std::string attr_key{};
                    while (i < source.size() && source[i] != '>' && source[i] != '=' && !std::iswspace(source[i])) {
                        attr_key.push_back(source[i]);
                        i++;
                    }
                    if (i < source.size() && source[i] == '=') i++;
                    std::string attr_val{};
                    if (i < source.size() && source[i] != '\"' && source[i] != '\'') {
                        while (i < source.size() && source[i] != '>' && !std::iswspace(source[i])) {
                            attr_val.push_back(source[i]);
                            i++;
                        }
                    } else if (i < source.size()) {
                        if (source[i] != '\"' && source[i] != '\'') attr_val.push_back(source[i]);
                        i++;
                        while (i < source.size() && source[i] != '>' && source[i] != '\"' && source[i] != '\'') {
                            attr_val.push_back(source[i]);
                            i++;
                        }
                    }
                    tag_attributes[attr_key] = attr_val;
                }
                if (is_singleton) {
                    result.push_back(HTMLTag(tag_name, tag_attributes, {}));
                } else {
                    i++;
                    int j = 0, c = 0;
                    while (i + j < source.size()) {
                        if (source.substr(i + j, 1 + tag_name.size()) == "<" + tag_name) {
                            c++;
                            j += 1 + tag_name.size();
                            continue;
                        }
                        else if (c == 0 && source.substr(i + j, 3 + tag_name.size()) == "</" + tag_name + ">") break;
                        else if (source.substr(i + j, 3 + tag_name.size()) == "</" + tag_name + ">") {
                            c--;
                            j += 3 + tag_name.size();
                            continue;
                        }
                        j++;
                    }
                    result.push_back(HTMLTag(tag_name, tag_attributes, tokenizeHTML(source.substr(i, j))));
                    i += j + tag_name.size() + 2;
                }
            }
            break;
            default:
            {
                std::string html_text{};
                while (i < source.size() && source[i] != '<') {
                    html_text.push_back(source[i]);
                    i++;
                }
                i--;
                result.push_back(html_text);
            }    
            break;
        }
    }
    return result;
}

std::string tagToStr(std::string tag, HTMLTagAttr attrs) {
    if (attrs.count("li")) tag = " - " + tag;
    if (attrs.count("bold")) tag = "\033[1m" + tag;
    if (attrs.count("italic")) tag = "\033[3m" + tag;
    if (attrs.count("underline")) tag = "\033[4m" + tag;
    if (attrs.count("a")) tag = "\033[4m\033[38;2;0;0;255m" + tag;
    if (attrs.count("alt")) tag = "\033[1m" + attrs["alt"];
    for (auto& [attr, val] : attrs) {
        if (attr == "align") {
            if (val == "left") continue;
            else if (val == "right") {
                tag.insert(tag.begin(), TERM_SIZE.ws_col - tag.size() + 4, ' ');
                continue;
            } else if (val == "center") {
                tag.insert(tag.begin(), (TERM_SIZE.ws_col - tag.size()) / 2 + 4, ' ');
                continue;
            } else if (val == "justify") continue;
        } else if (attr == "bgcolor") {
            std::string ansi_clr{};
            if (val[0] != '#') val = COLOR_NAMES.at(val);
            ansi_clr = "\033[48;2;" + std::to_string(std::stoi(val.substr(1, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(3, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(5, 2), nullptr, 16)) + "m";
            int i = 0;
            while (i < tag.size() && std::iswspace(tag[i])) i++;
            tag.insert(tag.begin() + i, ansi_clr.begin(), ansi_clr.end());
            continue;
        } else if (attr == "color") {
            std::string ansi_clr{};
            if (val[0] != '#') val = COLOR_NAMES.at(val);
            ansi_clr = "\033[38;2;" + std::to_string(std::stoi(val.substr(1, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(3, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(5, 2), nullptr, 16)) + "m";
            int i = 0;
            while (i < tag.size() && std::iswspace(tag[i])) i++;
            tag.insert(tag.begin() + i, ansi_clr.begin(), ansi_clr.end());
            continue;
        }
    }
    tag.append("\033[0m");
    if (!attrs.count("span")) tag.push_back('\n');
    return tag;
}

std::string parseTokens(HTMLTagContent tokens, HTMLTagAttr attrs = {}) {
    std::string result{};
    for (std::variant<HTMLTag, std::string> tag : tokens) {
        if (tag.index() == 0) {
            HTMLTagAttr new_attrs{attrs};
            std::string tag_name = std::get<HTMLTag>(tag).name;
            if (tag_name.size() >= 2 && tag_name[0] == 'h' && std::isdigit(tag_name[1])) new_attrs["bold"];
            else if (tag_name == "b" || tag_name == "strong") new_attrs["bold"];
            else if (tag_name == "i" || tag_name == "em") new_attrs["italic"];
            else if (tag_name == "u") new_attrs["underline"];
            else if (tag_name == "span") new_attrs["span"];
            else if (tag_name == "li") new_attrs["li"];
            else if (tag_name == "a") new_attrs["a"];
            std::get<HTMLTag>(tag).attributes.insert(new_attrs.begin(), new_attrs.end());
            result.append(parseTokens(std::get<HTMLTag>(tag).content, std::get<HTMLTag>(tag).attributes));
        } else {
            result.append(tagToStr(std::get<std::string>(tag), attrs));
        }
    }
    return result;
}

std::string parseHTML(std::string source) {
    return parseTokens(tokenizeHTML(source));
}