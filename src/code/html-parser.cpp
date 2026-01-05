#include <html-parser.hpp>
#include <input-handler.hpp>
#include <algorithm>
#include <unistd.h>
#include <print>
#include <sstream>
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

HTMLTag::HTMLTag(std::string n, std::unordered_map<std::string, std::string> a, std::vector<std::variant<HTMLTag, std::string>> c) : name(n), attributes(a), content(c) {};

HTMLTagContent tokenizeHTML(std::string source) {
    HTMLTagContent result;
    for (size_t i = 0; i < source.size(); i++) {
        switch (source[i]) {
            case '<':
            {
                if (source.size() - i > 4 && (source.substr(i, 4) == "<!--")) {
                    while (i < source.size() - 3 && source.substr(i, 3) != "-->") i++;
                    i += 2;
                    break;
                }
                if (source.size() - i > 9 && (source.substr(i, 9) == "<!DOCTYPE" || source.substr(i, 9) == "<!doctype")) {
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
                    if (tag_attributes.count("alt")) result.push_back(HTMLTag(tag_name, tag_attributes, {"\033[1m" + tag_attributes["alt"]}));
                    else if (tag_name == "br") result.push_back(HTMLTag(tag_name, tag_attributes, {" "}));
                    else if (tag_name == "hr") {
                        std::string hr_str{};
                        if (TERM_SIZE.ws_col > 8) {
                            hr_str = "  ";
                            for (int i = 0; i < TERM_SIZE.ws_col - 4; i++) hr_str.push_back('-');
                            hr_str.append("  ");
                        } else {
                            for (int i = 0; i < TERM_SIZE.ws_col; i++) hr_str.push_back('-');
                        }
                        result.push_back(HTMLTag(tag_name, tag_attributes, {hr_str}));
                    }
                    else result.push_back(HTMLTag(tag_name, tag_attributes, {}));
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
                    if (tag_name == "script" || tag_name == "style") result.push_back(HTMLTag(tag_name, tag_attributes, {source.substr(i, j)}));
                    else result.push_back(HTMLTag(tag_name, tag_attributes, tokenizeHTML(source.substr(i, j))));
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
    if (attrs.count("dd")) tag = "\t" + tag;
    if (attrs.count("bold")) tag = "\033[1m" + tag;
    if (attrs.count("italic")) tag = "\033[3m" + tag;
    if (attrs.count("underline")) tag = "\033[4m" + tag;
    if (attrs.count("a")) tag = "\033[4m\033[38;2;0;0;255m" + tag;
    for (auto& [attr, val] : attrs) {
        if (attr == "align") {
            if (val == "left") continue;
            else if (val == "right") {
                if (TERM_SIZE.ws_col > tag.size()) tag.insert(tag.begin(), TERM_SIZE.ws_col - tag.size() + 4, ' ');
                continue;
            } else if (val == "center") {
                if (TERM_SIZE.ws_col > tag.size()) tag.insert(tag.begin(), (TERM_SIZE.ws_col - tag.size()) / 2 + 4, ' ');
                continue;
            } else if (val == "justify") continue;
        } else if (attr == "bgcolor") {
            std::string ansi_clr{};
            if (val[0] != '#') val = COLOR_NAMES.at(val);
            ansi_clr = "\033[48;2;" + std::to_string(std::stoi(val.substr(1, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(3, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(5, 2), nullptr, 16)) + "m";
            size_t i = 0;
            while (i < tag.size() && std::iswspace(tag[i])) i++;
            tag.insert(tag.begin() + i, ansi_clr.begin(), ansi_clr.end());
            continue;
        } else if (attr == "color") {
            std::string ansi_clr{};
            if (val[0] != '#') val = COLOR_NAMES.at(val);
            ansi_clr = "\033[38;2;" + std::to_string(std::stoi(val.substr(1, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(3, 2), nullptr, 16)) + ";" + std::to_string(std::stoi(val.substr(5, 2), nullptr, 16)) + "m";
            size_t i = 0;
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
    int ol_count = 0;
    for (std::variant<HTMLTag, std::string> tag : tokens) {
        if (tag.index() == 0) {
            HTMLTagAttr new_attrs{attrs};
            std::string tag_name = std::get<HTMLTag>(tag).name;
            if (tag_name.size() >= 2 && tag_name[0] == 'h' && std::isdigit(tag_name[1])) new_attrs["bold"];
            else if (tag_name == "b" || tag_name == "strong" || tag_name == "dt") new_attrs["bold"];
            else if (tag_name == "i" || tag_name == "em") new_attrs["italic"];
            else if (tag_name == "span") {
                if (*(result.end() - 1) == '\n') result.erase(result.end() - 1);
                new_attrs["span"];
            }
            else if (tag_name == "u") new_attrs["underline"];
            else if (tag_name == "a") new_attrs["a"];
            else if (tag_name == "dd") new_attrs["dd"];
            else if (tag_name == "ol") new_attrs["ol"];
            else if (tag_name == "ul") new_attrs["ul"];
            else if (tag_name == "title"  || tag_name == "style" || tag_name == "script") continue;
            new_attrs.insert(attrs.begin(), attrs.end());
            if (tag_name == "li" && attrs.count("ol")) {
                ol_count++;
                result.append(" " + std::to_string(ol_count) + ". ");
                new_attrs.erase("ol");
            }
            else if (tag_name == "li" && attrs.count("ul")) {
                result.append(" * ");
                new_attrs.erase("ul");
            }
            new_attrs.insert(std::get<HTMLTag>(tag).attributes.begin(), std::get<HTMLTag>(tag).attributes.end());
            result.append(parseTokens(std::get<HTMLTag>(tag).content, new_attrs));
        } else {
            result.append(tagToStr(std::get<std::string>(tag), attrs));
        }
    }
    return result;
}

std::string parseHTML(std::string source_raw) {
    if (OPTIONS["-s"].size() || OPTIONS["--source"].size()) return source_raw;
    std::stringstream stream(source_raw);
    std::string source, line;
    while (std::getline(stream, line)) {
        size_t i;
        for (i = 0; i != line.size() && std::iswspace(line[i]); i++);
        if (i == line.size()) continue;
        size_t j;
        for (j = line.size(); std::iswspace(line[j - 1]); j--);
        source.append(line.substr(i, j - i));
    }
    return parseTokens(tokenizeHTML(source));
}