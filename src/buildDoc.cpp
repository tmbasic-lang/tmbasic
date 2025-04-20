#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using std::cerr;
using std::function;
using std::istringstream;
using std::make_unique;
using std::ofstream;
using std::ostringstream;
using std::regex;
using std::regex_error;
using std::regex_replace;
using std::regex_search;
using std::runtime_error;
using std::smatch;
using std::sort;
using std::string;
using std::unique_ptr;
using std::unordered_set;
using std::vector;
using std::regex_constants::match_any;

const string kCharDiamond = "\x04";
const string kHtmlDiamond = "♦";
const string kCharBullet = "\x07";
const string kHtmlBullet = "•";
const string kCharTriangleRight = "\x10";
const string kHtmlTriangleRight = "►";
const string kCharOpenCircle = "\x09";
const string kHtmlOpenCircle = "○";
const string kCharEmDash = "-";
const string kHtmlEmDash = "—";

struct Parameter {
    string name;
    string description;
    string type;
};

struct Example {
    string description;
    string code;
    string output;
};

struct ReturnType {
    string description;
    string type;
};

struct Overload {
    vector<unique_ptr<Parameter>> parameters;
    vector<unique_ptr<Example>> examples;
    unique_ptr<ReturnType> returns;  // nullable
    vector<string> errorCodes;
};

struct Procedure {
    string name;
    string blurb;
    string description;
    string category;
    vector<unique_ptr<Overload>> overloads;
};

static std::string to_lower_copy(const std::string& str) {
    std::string result;
    for (char const c : str) {
        result += std::tolower(c);
    }
    return result;
}

// There's another copy of this function in strings.cpp
std::string trim_copy(const std::string& str) {
    if (str.empty()) {
        return str;
    }

    // Find first non-whitespace character
    size_t const start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";  // String contains only whitespace
    }

    // Find last non-whitespace character
    size_t const end = str.find_last_not_of(" \t\n\r\f\v");

    // Return the trimmed substring
    return str.substr(start, end - start + 1);
}

static void createDirectory(const string& path) {
    if (mkdir(path.c_str(), 0777) != 0 && errno != EEXIST) {
        throw runtime_error(string("Unable to create directory: ") + path);
    }
}

static bool tryReadFile(const string& filePath, string* output) {
    auto* fp = fopen(filePath.c_str(), "rb");  // NOLINT
    if (fp == nullptr) {
        return false;
    }
    fseek(fp, 0, SEEK_END);
    auto length = ftell(fp);
    rewind(fp);

    // +1 to give us a null terminator
    auto* buffer = reinterpret_cast<char*>(calloc(length + 1, 1));  // NOLINT
    auto readBytes = fread(buffer, 1, length, fp);
    if (readBytes != static_cast<size_t>(length)) {
        return false;
    }
    fclose(fp);  // NOLINT

    auto bufferStr = string(buffer);
    free(buffer);  // NOLINT
    *output = bufferStr;
    return true;
}

static string readFile(const string& filePath) {
    string output;
    if (!tryReadFile(filePath, &output)) {
        throw runtime_error(string("Unable to open file: ") + filePath);
    }
    return output;
}

static void writeFile(const string& filePath, const string& body) {
    auto file = ofstream(filePath);
    file << body;
}

static void forEachFile(const string& path, const function<void(string)>& func) {
    auto* dir = opendir(path.c_str());
    if (dir == nullptr) {
        throw runtime_error(string("Unable to open directory: ") + path);
    }
    while (true) {
        auto* entry = readdir(dir);
        if (entry == nullptr) {
            break;
        }
        if (entry->d_name[0] == '.') {
            continue;
        }
        func(entry->d_name);
    }
    closedir(dir);
}

static string replace(string haystack, const string& needle, const string& replacement) {
    auto pos = haystack.find(needle);
    while (pos != string::npos) {
        haystack.replace(pos, needle.length(), replacement);
        pos = haystack.find(needle, pos + replacement.length());
    }
    return haystack;
}

static string replaceRegex(const string& haystack, const string& pattern, const string& replacement) {
    return regex_replace(haystack, regex(pattern), replacement, match_any);
}

static string replaceRegex(string haystack, const string& pattern, const function<string(smatch&)>& replacementFunc) {
    regex const r(pattern);
    while (true) {
        smatch match;
        if (regex_search(haystack, match, r)) {
            auto replacement = replacementFunc(match);
            haystack.replace(match.position(0), match.length(0), replacement);
        } else {
            break;
        }
    }
    return haystack;
}

static string indent(const string& str) {
    istringstream input(str);
    ostringstream output;
    string line;
    while (getline(input, line)) {
        output << "  " << line << "\n";
    }
    return output.str();
}

static string replaceIndentCharsWithHtmlSpans(string str) {
    return replace(std::move(str), "    ", "<span class=\"indent\">    </span>");
}

static string addHtmlSyntaxColoringToCode(string str) {
    str = replaceRegex(str, R"("[^"]*")", "<span class=\"string\">$0</span>");
    str = replaceRegex(str, "'.*", "<span class=\"comment\">$0</span>");
    return str;
}

static string htmlEncode(string str) {
    str = replace(str, "&", "&amp;");
    str = replace(str, "<", "&lt;");
    str = replace(str, ">", "&gt;");
    return str;
}

static string processTitle(string str) {
    str = replaceRegex(str, "`([^`]+)`", "\"$1\"");
    return str;
}

static string processText(string str) {
    str = replaceRegex(str, R"(t\[(([^\] ]+)[^\]]*)\])", [](auto& match) -> string {
        return string("{") + match[1].str() + ":type_" + to_lower_copy(match[2].str()) + "}";
    });
    str = replaceRegex(str, R"(p\[([^\]]+)\])", [](auto& match) -> string {
        return string("{") + match[1].str() + ":procedure_" + to_lower_copy(match[1].str()) + "}";
    });
    str = replaceRegex(str, R"(i\[([^\]]+)\])", "$1");
    str = replaceRegex(str, R"(b\[([^\]]+)\])", "$1");
    str = replaceRegex(str, R"(h1\[([^\]]+)\])", "$1");
    str = replaceRegex(str, R"(h2\[([^\]]+)\])", string(kCharDiamond) + " $1");
    str = replaceRegex(str, R"(h3\[([^\]]+)\])", "$1");
    str = replaceRegex(str, R"(bar\[([^\]]+)\])", "$1");
    str = replaceRegex(str, "code@([^@]+)@", [](auto& match) -> string { return indent(match[1].str()); });
    str = replaceRegex(str, "pre@([^@]+)@", [](auto& match) -> string { return indent(match[1].str()); });
    str = replaceRegex(str, "nav@([^@]+)@", "$1");
    str = replaceRegex(str, "`([^`]+)`", "$1");
    str = replaceRegex(str, "li@([^@]+)@\n*", string(kCharBullet) + " $1\n\n");
    str = replaceRegex(str, "ul@\n*([^@]+)\n*@\n*", "$1");
    str = replaceRegex(str, "([^{])\\{([^:{]+):(http[^}]+)\\}", "$1$2");
    str = replace(str, "<DIAMOND>", kCharDiamond);
    str = replace(str, "<BULLET>", kCharBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kCharTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kCharOpenCircle);
    str = replace(str, "<EM_DASH>", kCharEmDash);
    return str;
}

static string processHtml(string str) {
    str = replace(str, "<DIAMOND>", kHtmlDiamond);
    str = replace(str, "<BULLET>", kHtmlBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kHtmlTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kHtmlOpenCircle);
    str = replace(str, "<EM_DASH>", kHtmlEmDash);
    str = htmlEncode(str);
    str = replace(str, kHtmlTriangleRight, string("<wbr>") + kHtmlTriangleRight);
    str = replaceRegex(str, R"(t\[(([^\] ]+)[^\]]*)\])", [](auto& match) -> string {
        return string("<a href=\"type_") + to_lower_copy(match[2].str()) + ".html\">" + match[1].str() + "</a>";
    });
    str = replaceRegex(str, R"(p\[([^\]]+)\])", [](auto& match) -> string {
        return string("<a href=\"procedure_") + to_lower_copy(match[1].str()) + ".html\">" + match[1].str() + "</a>";
    });
    str = replaceRegex(str, R"(i\[([^\]]+)\])", "<i>$1</i>");
    str = replaceRegex(str, R"(b\[([^\]]+)\])", "<b>$1</b>");
    str = replaceRegex(str, "\n*h1\\[([^\\]]+)\\]\n*", "<h1>$1</h1>");
    str = replaceRegex(str, "\n*h2\\[([^\\]]+)\\]\n*", "<h2>$1</h2>");
    str = replaceRegex(str, "\n*h3\\[([^\\]]+)\\]\n*", "<h3>$1</h3>");
    str = replaceRegex(str, "\n*bar\\[([^\\]]+)\\]\n*", "<div class=\"bar\"><span>$1</span></div>");
    str = replaceRegex(str, "code@\n*([^@]+)\n*@\n*", [](auto& match) -> string {
        auto x = match[1].str();
        x = addHtmlSyntaxColoringToCode(x);
        x = replaceIndentCharsWithHtmlSpans(x);
        x = trim_copy(x);
        x = replace(x, "\n", "</div><div>");
        x = string("<div class=\"code\"><div>") + x + "</div></div>";
        x = replace(x, "<div></div>", "<pre></pre>");
        return x;
    });
    str = replaceRegex(str, "pre@\n*([^@]+)\n*@\n*", [](auto& match) -> string {
        return string("<div><pre class=\"diagram\">") + match[1].str() + "</pre></div>";
    });
    str = replaceRegex(str, "nav@([^@]+)@", "<div class=\"backlinks\">$1</div>");
    str = replaceRegex(str, "`([^`]+)`", "<span class=\"tt\">$1</span>");
    str = replace(str, "</h1><br>", "</h1>");
    str = replace(str, "</h2><br>", "</h2>");
    str = replace(str, "</h3><br>", "</h3>");
    str = replace(str, "</div><br>", "</div>");
    str = replace(str, "</pre><br>", "</pre>");
    str = replaceRegex(str, "\n*li@\n*([^@]+)\n*@\n*", "<li>$1</li>");
    str = replaceRegex(str, "\n*ul@\n*([^@]+)\n*@\n*", "<ul>$1</ul>");
    str = replaceRegex(str, "([^{])\\{([^:{]+):(http[^}]+)\\}", "$1<a href=\"$3\">$2</a>");
    str = replaceRegex(str, "([^{])\\{([^{}]+[^:]):([^}]+)\\}", [](auto& match) -> string {
        auto a = match[1].str();
        auto b = replace(match[2].str(), "::", ":");
        auto c = match[3].str();
        return a + "<a href=\"" + c + ".html\">" + b + "</a>";
    });
    str = replace(str, "{{", "{");
    str = replace(str, "--", "—");
    str = replace(str, "\n\n", "<div class=\"paragraphBreak\"></div>");
    return str;
}

static void writeHtmlPage(const string& topic, const string& text, const string& htmlPageTemplate) {
    smatch match;
    if (!regex_search(text, match, regex(R"(h1\[([^\]]+)\])"))) {
        throw runtime_error(string{ "Cannot find h1 tag in topic: " } + topic);
    }
    auto title = match[1].str();
    auto html = htmlPageTemplate;
    html = replace(html, "[TITLE]", processTitle(title) + " - TMBASIC");
    html = replace(html, "[BODY]", processHtml(text));
    writeFile(string("../obj/doc-html/") + topic + ".html", html);
}

static void buildTopic(
    const string& utf8Filename,
    const string& topic,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    *outputTxt << ".topic " << topic << "\n" << processText(trim_copy(readFile(utf8Filename))) << "\n\n";
    writeHtmlPage(topic, readFile(utf8Filename), htmlPageTemplate);
}

static vector<string> splitStringIntoLines(const string& input) {
    vector<string> lines;
    istringstream stream(input);
    string line;
    while (getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

static bool startsWith(const string& str, char ch) {
    return !str.empty() && str[0] == ch;
}

static string readProcedureBlock(const vector<string>& lines, size_t* i) {
    ostringstream block;
    while (!startsWith(lines[*i + 1], '.')) {
        (*i)++;
        block << lines[*i] << "\n";
    }
    return trim_copy(block.str());
}

static unique_ptr<Procedure> parseProcedure(const string& input) {
    auto lines = splitStringIntoLines(input);
    auto procedure = make_unique<Procedure>();
    Overload* overload = nullptr;
    Example* example = nullptr;

    for (size_t i = 0; i < lines.size(); i++) {
        auto& line = lines[i];

        if (startsWith(line, '.')) {
            smatch match;
            if (!regex_search(line, match, regex("^([^ ]+)"))) {
                throw runtime_error(string("Unable to parse line: ") + line);
            }

            auto section = match[1].str();
            auto rest = trim_copy(line.substr(section.length()));

            if (section == ".procedure") {
                // like ".procedure hasValue"
                procedure->name = rest;
            } else if (section == ".category") {
                procedure->category = rest;
            } else if (section == ".overload") {
                auto newOverload = make_unique<Overload>();
                overload = newOverload.get();
                procedure->overloads.push_back(std::move(newOverload));
            } else if (section == ".description") {
                if (!procedure->description.empty()) {
                    throw runtime_error(string("Duplicate description in procedure ") + procedure->name);
                }
                procedure->description = readProcedureBlock(lines, &i);
            } else if (section == ".blurb") {
                if (!procedure->blurb.empty()) {
                    throw runtime_error(string("Duplicate blurb in procedure ") + procedure->name);
                }
                procedure->blurb = rest;
            } else if (section == ".parameter") {
                // like ".parameter this: optional T"
                smatch parameterMatch;
                if (!regex_search(rest, parameterMatch, regex("([^:]+):(.+)"))) {
                    throw runtime_error(string("Unable to parse parameter: ") + rest);
                }

                auto parameter = make_unique<Parameter>();
                parameter->name = trim_copy(parameterMatch[1].str());
                parameter->type = trim_copy(parameterMatch[2].str());
                parameter->description = readProcedureBlock(lines, &i);
                overload->parameters.push_back(std::move(parameter));
            } else if (section == ".return") {
                // like ".return boolean"
                auto returns = make_unique<ReturnType>();
                returns->type = rest;
                returns->description = readProcedureBlock(lines, &i);
                overload->returns = std::move(returns);
            } else if (section == ".errors") {
                auto block = readProcedureBlock(lines, &i);
                vector<string> errorCodes{};
                std::istringstream ss{ block };
                std::string line;
                while (std::getline(ss, line)) {
                    errorCodes.push_back(line);
                }
                overload->errorCodes = std::move(errorCodes);
            } else if (section == ".example") {
                auto newExample = make_unique<Example>();
                example = newExample.get();
                newExample->description = readProcedureBlock(lines, &i);
                overload->examples.push_back(std::move(newExample));
            } else if (section == ".example-code") {
                if (!example->code.empty()) {
                    throw runtime_error(string("Duplicate example-code in procedure ") + procedure->name);
                }
                example->code = readProcedureBlock(lines, &i);
            } else if (section == ".example-output") {
                if (!example->output.empty()) {
                    throw runtime_error(string("Duplicate example-output in procedure ") + procedure->name);
                }
                example->output = readProcedureBlock(lines, &i);
            } else if (section == ".") {
                break;
            }
        } else if (trim_copy(line).empty()) {
            // ignore
        } else {
            throw runtime_error(string("Unexpected line: ") + line);
        }
    }

    return procedure;
}

static string getProcedureCategoryTopic(const string& category) {
    ostringstream topic;
    topic << "procedures_";
    for (auto ch : category) {
        if (ch == ' ') {
            topic << '_';
        } else if (ch == '/') {
            // eat it
        } else {
            topic << static_cast<char>(tolower(ch));
        }
    }
    return topic.str();
}

static string formatProcedureText(const Procedure& procedure) {
    auto categoryTopic = getProcedureCategoryTopic(procedure.category);

    ostringstream o;
    o << "nav@{TMBASIC Documentation:doc} <TRIANGLE_RIGHT> {Procedures:: " << procedure.category << ":" << categoryTopic
      << "}@\n\n";
    o << "h1[`" << procedure.name << "` Procedure]\n\n" << procedure.description << "\n\n";

    auto overloadNumber = 0;
    for (const auto& overload : procedure.overloads) {
        auto isFunction = overload->returns != nullptr;

        // only show the overload header if there are multiple overloads
        overloadNumber++;
        if (procedure.overloads.size() > 1) {
            o << "h2[Overload #" << overloadNumber << ": " << procedure.name << "(";
            for (size_t i = 0; i < overload->parameters.size(); i++) {
                auto& parameter = overload->parameters.at(i);
                if (i > 0) {
                    o << ", ";
                }
                o << parameter->name;
            }
            o << ")]\n\n";
        }

        o << "bar[Declaration]\ncode@\n" << (isFunction ? "function " : "sub ") << procedure.name << "(";

        for (size_t i = 0; i < overload->parameters.size(); i++) {
            auto& parameter = overload->parameters.at(i);
            if (i > 0) {
                o << ", ";
            }
            if (overload->parameters.size() > 1 && (i % 3) == 0) {
                o << "\n    ";
            }
            o << parameter->name << " as " << parameter->type;
        }
        if (overload->parameters.size() > 1 && !overload->parameters.empty()) {
            o << "\n";
        }
        if (isFunction) {
            o << ") as " << overload->returns->type;
        } else {
            o << ")";
        }
        o << "@\n";

        if (!overload->parameters.empty()) {
            o << "h3[Parameters]\n\nul@";
            for (auto& parameter : overload->parameters) {
                o << "li@i[" << parameter->name << "] as t[" << parameter->type << "] <EM_DASH> "
                  << parameter->description << "@";
            }
            o << "@\n";
        }

        if (isFunction) {
            o << "h3[Return value]\n\n";
            if (overload->returns->type != "T") {
                o << "t[" << overload->returns->type << "] <EM_DASH> ";
            }
            o << overload->returns->description << "\n\n";
        }

        if (!overload->errorCodes.empty()) {
            o << "h3[Possible error codes]\n\nul@";
            for (auto& errorCode : overload->errorCodes) {
                o << "li@`" << errorCode << "`@\n";
            }
            o << "@\n";
        }

        for (auto& example : overload->examples) {
            o << "h3[Example]\n\n";
            if (!trim_copy(example->description).empty()) {
                o << example->description << "\n\n";
            }
            o << "bar[Code]\ncode@" << example->code << "@\n\n";
            if (!example->output.empty()) {
                o << "bar[Output]\ncode@" << example->output << "@\n\n";
            }
        }

        o << "\n\n";
    }

    return o.str();
}

static unique_ptr<Procedure> buildProcedure(
    const string& utf8Filename,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    auto procedure = parseProcedure(readFile(utf8Filename));
    auto topicName = string("procedure_") + to_lower_copy(procedure->name);
    *outputTxt << ".topic " << topicName << "\n" << processText(formatProcedureText(*procedure)) << "\n";
    writeHtmlPage(topicName, formatProcedureText(*procedure), htmlPageTemplate);
    return procedure;
}

static bool compareProceduresByName(const unique_ptr<Procedure>& lhs, const unique_ptr<Procedure>& rhs) {
    return lhs->name.compare(rhs->name) < 0;
}

static void buildProcedureCategoryPages(
    const vector<unique_ptr<Procedure>>& procedures,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    unordered_set<string> categoriesSet;
    for (const auto& p : procedures) {
        categoriesSet.insert(p->category);
    }

    for (const auto& category : categoriesSet) {
        auto topic = getProcedureCategoryTopic(category);

        ostringstream o;
        o << "nav@{TMBASIC Documentation:doc}@\n\nh1[Procedures: " << category << "]\n\nul@";

        for (const auto& x : procedures) {
            if (x->category == category) {
                o << "li@b[{`" << x->name << "`:procedure_" << to_lower_copy(x->name) << "}] <EM_DASH> " << x->blurb
                  << "@\n";
            }
        }

        o << "@\n\n";

        ostringstream filePathStream;
        filePathStream << "../obj/doc-temp/" << topic << ".txt";
        auto filePath = filePathStream.str();
        writeFile(filePath, o.str());
        buildTopic(filePath, topic, outputTxt, htmlPageTemplate);
    }
}

static void buildProcedureIndex(
    const vector<unique_ptr<Procedure>>& procedures,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    ostringstream o;
    o << "nav@{TMBASIC Documentation:doc}@\n\nh1[All Procedures]\n\n";

    o << "ul@";
    for (const auto& x : procedures) {
        o << "li@b[{`" << x->name << "`:procedure_" << to_lower_copy(x->name) << "}] <EM_DASH> " << x->blurb << "@\n";
    }
    o << "@\n\n";

    unordered_set<string> typeNamesSet;
    for (const auto& p : procedures) {
        for (auto& o : p->overloads) {
            if (o->parameters.empty()) {
                typeNamesSet.insert("(None)");
            } else {
                typeNamesSet.insert(o->parameters[0]->type);
            }
        }
    }

    vector<string> typeNames;
    typeNames.reserve(typeNamesSet.size());
    for (const auto& s : typeNamesSet) {
        typeNames.push_back(s);
    }
    sort(typeNames.begin(), typeNames.end());

    const auto* filePath = "../obj/doc-temp/procedures.txt";
    writeFile(filePath, o.str());
    buildTopic(filePath, "procedures", outputTxt, htmlPageTemplate);
}

int main() {
    try {
        ostringstream outputTxt;
        vector<unique_ptr<Procedure>> procedures;
        auto htmlPageTemplate = readFile("help/html/page-template-1.html") + "[TITLE]" +
            readFile("help/html/page-template-2.html") + "[BODY]" + readFile("help/html/page-template-3.html");
        createDirectory("../obj");
        createDirectory("../obj/doc-temp");
        createDirectory("../obj/doc-html");
        forEachFile("help/topics", [&outputTxt, &htmlPageTemplate](auto filename) -> void {
            auto topic = filename.substr(0, filename.length() - 4);
            buildTopic(string("help/topics/") + filename, topic, &outputTxt, htmlPageTemplate);
        });
        forEachFile("help/procedures", [&outputTxt, &htmlPageTemplate, &procedures](auto filename) -> void {
            procedures.push_back(buildProcedure(string("help/procedures/") + filename, &outputTxt, htmlPageTemplate));
        });
        sort(procedures.begin(), procedures.end(), compareProceduresByName);
        buildProcedureCategoryPages(procedures, &outputTxt, htmlPageTemplate);
        buildProcedureIndex(procedures, &outputTxt, htmlPageTemplate);
        writeFile("../help-temp/help.txt", outputTxt.str());
    } catch (const regex_error& ex) {
        ostringstream const s;
        cerr << ex.what() << ": " << ex.code() << '\n';
        return -1;
    } catch (const runtime_error& ex) {
        ostringstream const s;
        cerr << ex.what() << '\n';
        return -1;
    }

    return 0;
}
