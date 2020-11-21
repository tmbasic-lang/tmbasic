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
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <nameof.hpp>

using boost::trim_copy;
using std::cerr;
using std::cout;
using std::endl;
using std::function;
using std::istringstream;
using std::make_unique;
using std::move;
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
using std::system_error;
using std::unique_ptr;
using std::vector;
using std::regex_constants::match_any;

const char kCharDiamond[] = "\x04";
const char kHtmlDiamond[] = "&diams;";
const char kCharBullet[] = "\x07";
const char kHtmlBullet[] = "&bull;";
const char kCharTriangleRight[] = "\x10";
const char kHtmlTriangleRight[] = "&#x25ba;";
const char kCharOpenCircle[] = "\x09";
const char kHtmlOpenCircle[] = "&#x25cb;";

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
    string description;
    vector<unique_ptr<Parameter>> parameters;
    vector<unique_ptr<Example>> examples;
    unique_ptr<ReturnType> returns;  // nullable
};

struct Procedure {
    string name;
    vector<unique_ptr<Overload>> overloads;
};

static void createDirectory(string path) {
    if (mkdir(path.c_str(), 0777) != 0 && errno != EEXIST) {
        throw runtime_error(string("Unable to create directory: ") + path);
    }
}

static bool tryReadFile(const string& filePath, string* output) {
    auto* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return false;
    }
    fseek(fp, 0, SEEK_END);
    auto length = ftell(fp);
    rewind(fp);

    // +1 to give us a null terminator
    auto* buffer = reinterpret_cast<char*>(calloc(length + 1, 1));
    auto readBytes = fread(buffer, 1, length, fp);
    if (readBytes != static_cast<size_t>(length)) {
        return false;
    }
    fclose(fp);

    auto bufferStr = string(buffer);
    free(buffer);
    *output = bufferStr;
    return true;
}

static string readFile(string filePath) {
    string output;
    if (!tryReadFile(filePath, &output)) {
        throw runtime_error(string("Unable to open file: ") + filePath);
    }
    return output;
}

static void writeFile(string filePath, string body) {
    auto file = ofstream(filePath);
    file << body;
}

static void copyFile(string srcFilePath, string dstFilePath) {
    writeFile(dstFilePath, readFile(srcFilePath));
}

static void forEachFile(string path, function<void(string)> func) {
    auto* dir = opendir(path.c_str());
    if (!dir) {
        throw runtime_error(string("Unable to open directory: ") + path);
    }
    while (true) {
        auto* entry = readdir(dir);
        if (!entry) {
            break;
        }
        if (entry->d_name[0] == '.') {
            continue;
        }
        func(entry->d_name);
    }
}

static void copyDir(string src, string dst) {
    createDirectory(dst);
    forEachFile(src, [&src, &dst](string filename) -> void {
        auto srcFilePath = src + "/" + filename;
        auto dstFilePath = dst + "/" + filename;
        copyFile(srcFilePath, dstFilePath);
    });
}

static string replace(string haystack, string needle, string replacement) {
    auto pos = haystack.find(needle);
    while (pos != string::npos) {
        haystack.replace(pos, needle.length(), replacement);
        pos = haystack.find(needle, pos + replacement.length());
    }
    return haystack;
}

static string replaceRegex(string haystack, string pattern, string replacement) {
    return regex_replace(haystack, regex(pattern), replacement, match_any);
}

static string replaceRegex(string haystack, string pattern, function<string(smatch&)> replacementFunc) {
    regex r(pattern);
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

static string indent(string str) {
    istringstream input(str);
    ostringstream output;
    string line;
    while (getline(input, line)) {
        output << "  " << line << "\n";
    }
    return output.str();
}

static string replaceIndentCharsWithHtmlSpans(string str) {
    return replaceRegex(str, "    ", "<span class=\"indent\">$0</span>");
}

static string addHtmlSyntaxColoringToCode(string str) {
    return replaceRegex(str, "\"[^\"]*\"", "<span class=\"string\">$0</span>");
}

static string htmlEncode(string str) {
    str = replace(str, "&", "&amp;");
    str = replace(str, "<", "&lt;");
    str = replace(str, ">", "&gt;");
    return str;
}

static string getDiagramHtml(string name) {
    string text;
    if (tryReadFile(string("diagrams/") + name + ".txt", &text)) {
        return htmlEncode(text);
    } else {
        return htmlEncode(readFile(string("../obj/doc-temp/diagrams-license/") + name + ".txt"));
    }
}

static string processText(string str) {
    str = replaceRegex(str, "t\\[(([^\\] ]+)[^\\]]*)\\]", "{$1:type_$2}");
    str = replaceRegex(str, "p\\[([^\\]]+)\\]", "{$1:procedure_$1}");
    str = replaceRegex(str, "i\\[([^\\]]+)\\]", "'$1'");
    str = replaceRegex(str, "h1\\[([^\\]]+)\\]", "$1");
    str = replaceRegex(str, "h2\\[([^\\]]+)\\]", string(kCharDiamond) + " $1");
    str = replaceRegex(str, "h3\\[([^\\]]+)\\]", "$1");
    str = replaceRegex(str, "code@([^@]+)@", [](auto& match) -> string { return indent(match[1].str()); });
    str = replaceRegex(str, "nav@([^@]+)@", "$1");
    str = replaceRegex(str, "`([^`]+)`", "'$1'");
    str = replaceRegex(str, "li@([^@]+)@\n*", string(kCharBullet) + " $1\n\n");
    str = replaceRegex(str, "ul@\n*([^@]+)\n*@\n*", "$1");
    str = replace(str, "<DIAMOND>", kCharDiamond);
    str = replace(str, "<BULLET>", kCharBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kCharTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kCharOpenCircle);
    str = replaceRegex(str, "\\{[^:\"}]+:[^:\"}]+\\}", "{$0");
    return str;
}

static string processHtml(string str) {
    str = replace(str, "<DIAMOND>", kHtmlDiamond);
    str = replace(str, "<BULLET>", kHtmlBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kHtmlTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kHtmlOpenCircle);
    str = htmlEncode(str);
    str = replaceRegex(str, "t\\[(([^\\] ]+)[^\\]]*)\\]", "<a href=\"type_$2.html\">$1</a>");
    str = replaceRegex(str, "p\\[([^\\]]+)\\]", "<a href=\"procedure_$1.html\">$1</a>");
    str = replaceRegex(str, "i\\[([^\\]]+)\\]", "<i>$1</i>");
    str = replaceRegex(str, "\n*h1\\[([^\\]]+)\\]\n*", "<h1>$1</h1>");
    str = replaceRegex(str, "\n*h2\\[([^\\]]+)\\]\n*", "<h2>$1</h2>");
    str = replaceRegex(str, "\n*h3\\[([^\\]]+)\\]\n*", "<h3>$1</h3>");
    str = replaceRegex(str, "code@\n*([^@]+)\n*@\n*", [](auto& match) -> string {
        auto x = match[1].str();
        x = replaceIndentCharsWithHtmlSpans(x);
        x = addHtmlSyntaxColoringToCode(x);
        x = trim_copy(x);
        x = replace(x, "\n", "</div><div>");
        x = string("<div class=\"code\"><tt><div>") + x + "</div></tt></div>";
        x = replace(x, "<div></div>", "<pre></pre>");
        return x;
    });
    str = replaceRegex(str, "nav@([^@]+)@", "<navbar>$1</navbar>");
    str = replaceRegex(str, "`([^`]+)`", "<tt>$1</tt>");
    str = replace(str, "</h1><br>", "</h1>");
    str = replace(str, "</h2><br>", "</h2>");
    str = replace(str, "</h3><br>", "</h3>");
    str = replace(str, "</div><br>", "</div>");
    str = replace(str, "</navbar><br>", "</navbar>");
    str = replace(str, "</pre><br>", "</pre>");
    str = replaceRegex(str, "\n*li@\n*([^@]+)\n*@\n*", "<li>$1</li>");
    str = replaceRegex(str, "\n*ul@\n*([^@]+)\n*@\n*", "<ul>$1</ul>");
    str = replaceRegex(str, "\\{([^:]+):([^}]+)\\}", "<a href=\"$2.html\">$1</a>");
    str = replace(str, "\n-----\n", "<hr>");
    str = replace(str, "\n", "<br>");
    str = replaceRegex(str, "dia\\[([^\\]]+)\\]", [](auto& match) -> string {
        return string("<pre class=\"diagram\">") + getDiagramHtml(match[1].str()) + "</pre>";
    });
    return str;
}

static void writeHtmlPage(const string& topic, const string& text, const string& htmlPageTemplate) {
    smatch match;
    if (!regex_search(text, match, regex("h1\\[([^\\]]+)\\]"))) {
        throw runtime_error(string("Cannot find h1 tag in topic: " + topic));
    }
    auto title = match[1].str();
    auto html = htmlPageTemplate;
    html = replace(html, "[TITLE]", processHtml(title));
    html = replace(html, "[BODY]", processHtml(text));
    writeFile(string("../obj/doc-html/") + topic + ".html", html);
}

static void buildTopic(
    const string& filename,
    const string& topic,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    auto inputFilePath = string("topics/") + filename;
    auto input = readFile(inputFilePath);
    *outputTxt << ".topic " << topic << "\n" << processText(trim_copy(input)) << "\n\n-----\n\n";
    writeHtmlPage(topic, input, htmlPageTemplate);
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
    return str.length() > 0 && str[0] == ch;
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
            } else if (section == ".overload") {
                auto newOverload = make_unique<Overload>();
                overload = newOverload.get();
                procedure->overloads.push_back(move(newOverload));
            } else if (section == ".description") {
                if (overload->description.length() > 0) {
                    throw runtime_error(string("Duplicate description in procedure ") + procedure->name);
                }
                overload->description = readProcedureBlock(lines, &i);
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
                overload->parameters.push_back(move(parameter));
            } else if (section == ".return") {
                // like ".return boolean"
                auto returns = make_unique<ReturnType>();
                returns->type = rest;
                returns->description = readProcedureBlock(lines, &i);
                overload->returns = move(returns);
            } else if (section == ".example") {
                auto newExample = make_unique<Example>();
                example = newExample.get();
                newExample->description = readProcedureBlock(lines, &i);
                overload->examples.push_back(move(newExample));
            } else if (section == ".example-code") {
                if (example->code.length() > 0) {
                    throw runtime_error(string("Duplicate example-code in procedure ") + procedure->name);
                }
                example->code = readProcedureBlock(lines, &i);
            } else if (section == ".example-output") {
                if (example->output.length() > 0) {
                    throw runtime_error(string("Duplicate example-output in procedure ") + procedure->name);
                }
                example->output = readProcedureBlock(lines, &i);
            } else if (section == ".") {
                break;
            }
        } else if (trim_copy(line) == "") {
            // ignore
        } else {
            throw runtime_error(string("Unexpected line: ") + line);
        }
    }

    return procedure;
}

static string formatProcedureText(const string& topicName, const Procedure& procedure) {
    ostringstream o;
    o << "nav@{TMBASIC Documentation:doc} <TRIANGLE_RIGHT> {BASIC Reference:basic} <TRIANGLE_RIGHT> {Procedure "
         "Index:procedure}@\n\n";
    o << "h1['" << procedure.name << "' Procedure]\n\n";

    for (auto& overload : procedure.overloads) {
        auto isFunction = overload->returns != nullptr;

        o << "h2[" << (isFunction ? "function " : "sub ") << procedure.name << "(";

        auto isFirstParameter = true;
        for (auto& parameter : overload->parameters) {
            if (!isFirstParameter) {
                o << ", ";
            }
            o << "i[" << parameter->name << "] as t[" << parameter->type << "]";
            isFirstParameter = false;
        }
        o << ")";
        if (isFunction) {
            o << " as t[" << overload->returns->type << "]";
        }
        o << "]\n\n";

        o << overload->description << "\n\n";

        if (overload->parameters.size() > 0) {
            o << "h3[Parameters]\n\nul@";
            for (auto& parameter : overload->parameters) {
                o << "li@i[" << parameter->name << "] as t[" << parameter->type << "]: " << parameter->description
                  << "@";
            }
            o << "@";
        }

        if (isFunction) {
            o << "h3[Return value]\n\n" << overload->returns->description << "\n\n";
        }

        for (auto& example : overload->examples) {
            o << "h3[Example]\n\n";
            if (trim_copy(example->description) != "") {
                o << example->description << "\n\n";
            }
            o << "Code:\ncode@" << example->code << "@\n\n";
            o << "Output:\ncode@" << example->output << "@\n\n";
        }

        o << "-----\n\n";
    }

    return o.str();
}

static void buildProcedure(
    const string& filename,
    vector<string>* procedureNames,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    auto inputFilePath = string("procedures/") + filename;
    auto input = readFile(inputFilePath);
    auto procedure = parseProcedure(input);
    auto topicName = string("procedure_") + procedure->name;
    procedureNames->push_back(procedure->name);
    auto text = formatProcedureText(topicName, *procedure);
    *outputTxt << ".topic " << topicName << "\n" << processText(text) << "\n";
    writeHtmlPage(topicName, text, htmlPageTemplate);
}

static void buildProcedureIndex(
    vector<string> procedureNames,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    ostringstream o;
    o << "nav@{TMBASIC Documentation:doc} <TRIANGLE_RIGHT> {BASIC Reference:basic}@\n\nh1[Procedure Index]\n\n";
    o << "ul@";
    sort(procedureNames.begin(), procedureNames.end());
    for (auto& name : procedureNames) {
        o << "li@{'" << name << "' Procedure:procedure_" << name << "}@\n";
    }
    o << "@\n";
    auto filePath = "../obj/doc-temp/procedure.txt";
    writeFile(filePath, o.str());
    buildTopic(string("../") + filePath, "procedure", outputTxt, htmlPageTemplate);
}

static string insertCp437Diagram(string input, string filename) {
    auto cp437FilePath = string("../obj/doc-temp/diagrams-cp437/") + filename;
    auto name = filename.substr(0, filename.length() - 4);  // remove ".txt"

    auto diagram = indent(readFile(cp437FilePath));
    auto tag = string("dia[") + name + "]";

    while (true) {
        auto pos = input.find(tag);
        if (pos == string::npos) {
            break;
        }

        input = input.replace(pos, tag.length(), diagram);
    }

    return input;
}

static string insertCp437Diagrams(string text) {
    forEachFile("../obj/doc-temp/diagrams-cp437", [&text](auto filename) -> void {
        text = insertCp437Diagram(text, filename);
    });
    return text;
}

int main() {
    try {
        ostringstream outputTxt;
        vector<string> procedureNames;
        auto htmlPageTemplate = readFile("html/page-template.html");
        createDirectory("../obj");
        createDirectory("../obj/doc-temp");
        createDirectory("../obj/doc-temp/diagrams-cp437");
        createDirectory("../obj/doc-html");
        copyDir("../ext/notoserif", "../obj/doc-html/notoserif");
        copyDir("../ext/opensans", "../obj/doc-html/opensans");
        copyDir("../ext/oxygenmono", "../obj/doc-html/oxygenmono");
        copyFile("html/style.css", "../obj/doc-html/style.css");
        forEachFile("topics", [&outputTxt, &htmlPageTemplate](auto filename) -> void {
            auto topic = filename.substr(0, filename.length() - 4);
            buildTopic(filename, topic, &outputTxt, htmlPageTemplate);
        });
        forEachFile("procedures", [&outputTxt, &htmlPageTemplate, &procedureNames](auto filename) -> void {
            buildProcedure(filename, &procedureNames, &outputTxt, htmlPageTemplate);
        });
        buildProcedureIndex(procedureNames, &outputTxt, htmlPageTemplate);
        writeFile("../obj/help.txt", insertCp437Diagrams(outputTxt.str()));
    } catch (const regex_error& ex) {
        ostringstream s;
        cerr << ex.what() << ": " << NAMEOF_ENUM(ex.code()) << endl;
        return -1;
    } catch (const runtime_error& ex) {
        ostringstream s;
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}
