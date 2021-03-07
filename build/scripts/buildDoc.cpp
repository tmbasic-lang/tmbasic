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
using std::unordered_set;
using std::vector;
using std::regex_constants::match_any;

const char kCharDiamond[] = "\x04";
const char kHtmlDiamond[] = "♦";
const char kCharBullet[] = "\x07";
const char kHtmlBullet[] = "•";
const char kCharTriangleRight[] = "\x10";
const char kHtmlTriangleRight[] = "►";
const char kCharOpenCircle[] = "\x09";
const char kHtmlOpenCircle[] = "○";

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
};

struct Procedure {
    string name;
    string description;
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
    return replace(str, "    ", "<span class=\"indent\">    </span>");
}

static string addHtmlSyntaxColoringToCode(string str) {
    str = replaceRegex(str, "\"[^\"]*\"", "<span class=\"string\">$0</span>");
    str = replaceRegex(str, "'.*", "<span class=\"comment\">$0</span>");
    return str;
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

static string processTitle(string str) {
    str = replaceRegex(str, "`([^`]+)`", "\"$1\"");
    return str;
}

static string processText(string str) {
    str = replaceRegex(str, "t\\[(([^\\] ]+)[^\\]]*)\\]", "{$1:type_$2}");
    str = replaceRegex(str, "p\\[([^\\]]+)\\]", "{$1:procedure_$1}");
    str = replaceRegex(str, "i\\[([^\\]]+)\\]", "'$1'");
    str = replaceRegex(str, "h1\\[([^\\]]+)\\]", "$1");
    str = replaceRegex(str, "h2\\[([^\\]]+)\\]", string(kCharDiamond) + " $1");
    str = replaceRegex(str, "h3\\[([^\\]]+)\\]", "$1");
    str = replaceRegex(str, "bar\\[([^\\]]+)\\]", "$1");
    str = replaceRegex(str, "code@([^@]+)@", [](auto& match) -> string { return indent(match[1].str()); });
    str = replaceRegex(str, "nav@([^@]+)@", "$1");
    str = replaceRegex(str, "`([^`]+)`", "\"$1\"");
    str = replaceRegex(str, "li@([^@]+)@\n*", string(kCharBullet) + " $1\n\n");
    str = replaceRegex(str, "ul@\n*([^@]+)\n*@\n*", "$1");
    str = replaceRegex(str, "([^{])\\{([^:{]+):(http[^}]+)\\}", "$1$2");
    str = replace(str, "<DIAMOND>", kCharDiamond);
    str = replace(str, "<BULLET>", kCharBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kCharTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kCharOpenCircle);
    return str;
}

static string processHtml(string str) {
    str = replace(str, "<DIAMOND>", kHtmlDiamond);
    str = replace(str, "<BULLET>", kHtmlBullet);
    str = replace(str, "<TRIANGLE_RIGHT>", kHtmlTriangleRight);
    str = replace(str, "<OPEN_CIRCLE>", kHtmlOpenCircle);
    str = htmlEncode(str);
    str = replace(str, kHtmlTriangleRight, string("<wbr>") + kHtmlTriangleRight);
    str = replaceRegex(str, "t\\[(([^\\] ]+)[^\\]]*)\\]", "<a href=\"type_$2.html\">$1</a>");
    str = replaceRegex(str, "p\\[([^\\]]+)\\]", "<a href=\"procedure_$1.html\">$1</a>");
    str = replaceRegex(str, "i\\[([^\\]]+)\\]", "<i>$1</i>");
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
    str = replaceRegex(str, "([^{])\\{([^:{]+):([^}]+)\\}", "$1<a href=\"$3.html\">$2</a>");
    str = replace(str, "{{", "{");
    str = replace(str, "--", "—");
    str = replace(str, "\n\n", "<div class=\"paragraphBreak\"></div>");
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
    html = replace(html, "[TITLE]", processTitle(title) + " - TMBASIC Documentation");
    html = replace(html, "[BODY]", processHtml(text));
    writeFile(string("../obj/doc-html/") + topic + ".html", html);
}

static void buildTopic(
    const string& cp437Filename,
    const string& utf8Filename,
    const string& topic,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    *outputTxt << ".topic " << topic << "\n" << processText(trim_copy(readFile(cp437Filename))) << "\n\n";
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
                if (procedure->description.length() > 0) {
                    throw runtime_error(string("Duplicate description in procedure ") + procedure->name);
                }
                procedure->description = readProcedureBlock(lines, &i);
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
    o << "nav@{TMBASIC Documentation:doc} <TRIANGLE_RIGHT> {BASIC Reference:ref} <TRIANGLE_RIGHT> {Procedure "
         "Index:procedure}@\n\n";
    o << "h1[`" << procedure.name << "` Procedure]\n\n" << procedure.description << "\n\n";

    for (auto& overload : procedure.overloads) {
        auto isFunction = overload->returns != nullptr;

        o << "h2[" << (isFunction ? "function " : "sub ") << procedure.name << "(";

        auto isFirstParameter = true;
        for (auto& parameter : overload->parameters) {
            if (!isFirstParameter) {
                o << ", ";
            }
            o << parameter->name << " as ";
            if (parameter->type == "T") {
                o << "T";
            } else {
                o << "t[" << parameter->type << "]";
            }
            isFirstParameter = false;
        }
        o << ")";
        if (isFunction) {
            o << " as ";
            if (overload->returns->type == "T") {
                o << "T";
            } else {
                o << "t[" << overload->returns->type << "]";
            }
        }
        o << "]\n\n";

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
            o << "bar[Code]\ncode@" << example->code << "@\n\n";
            o << "bar[Output]\ncode@" << example->output << "@\n\n";
        }

        o << "\n\n";
    }

    return o.str();
}

static unique_ptr<Procedure> buildProcedure(
    const string& cp437Filename,
    const string& utf8Filename,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    auto cp437Procedure = parseProcedure(readFile(cp437Filename));
    auto topicName = string("procedure_") + cp437Procedure->name;
    *outputTxt << ".topic " << topicName << "\n"
               << processText(formatProcedureText(topicName, *cp437Procedure)) << "\n";
    auto utf8Procedure = parseProcedure(readFile(utf8Filename));
    writeHtmlPage(topicName, formatProcedureText(topicName, *utf8Procedure), htmlPageTemplate);
    return utf8Procedure;
}

static bool compareProceduresByName(const unique_ptr<Procedure>& lhs, const unique_ptr<Procedure>& rhs) {
    return lhs->name.compare(rhs->name) < 0;
}

static void buildProcedureIndex(
    vector<unique_ptr<Procedure>> procedures,
    ostringstream* outputTxt,
    const string& htmlPageTemplate) {
    sort(procedures.begin(), procedures.end(), compareProceduresByName);

    ostringstream o;
    o << "nav@{TMBASIC Documentation:doc} <TRIANGLE_RIGHT> {BASIC Reference:ref}@\n\nh1[Procedure Index]\n\n";

    o << "h2[By Name]\n\nul@";
    vector<string> procedureNames;
    for (auto& x : procedures) {
        procedureNames.push_back(x->name);
    }
    for (auto& name : procedureNames) {
        o << "li@{`" << name << "` Procedure:procedure_" << name << "}@\n";
    }
    o << "@\n\n";

    unordered_set<string> typeNamesSet;
    for (auto& p : procedures) {
        for (auto& o : p->overloads) {
            if (o->parameters.size() == 0) {
                typeNamesSet.insert("(None)");
            } else {
                typeNamesSet.insert(o->parameters[0]->type);
            }
        }
    }

    vector<string> typeNames;
    for (auto& s : typeNamesSet) {
        typeNames.push_back(s);
    }
    sort(typeNames.begin(), typeNames.end());

    o << "h2[By Parameter Type]\n\n";
    for (auto& t : typeNames) {
        o << "h3[" << t << "]\n\nul@";
        for (auto& p : procedures) {
            auto includeThisProcedure = false;
            for (auto& o : p->overloads) {
                includeThisProcedure |= (o->parameters.size() == 0 && t == "(None)") ||
                    (o->parameters.size() > 0 && t == o->parameters[0]->type);
            }
            if (includeThisProcedure) {
                o << "li@{`" << p->name << "` Procedure:procedure_" << p->name << "}@\n";
            }
        }
        o << "@\n\n";
    }

    auto filePath = "../obj/doc-temp/procedure.txt";
    writeFile(filePath, o.str());
    buildTopic(filePath, filePath, "procedure", outputTxt, htmlPageTemplate);
}

static string insertCp437Diagram(string input, string filename) {
    auto cp437FilePath = string("../obj/doc-temp/diagrams-cp437/") + filename;
    auto name = filename.substr(0, filename.length() - 4);  // remove ".txt"

    auto diagram = indent(readFile(cp437FilePath));

    // escape any { symbols in the diagram
    diagram = replace(diagram, "{", "{{");

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
        vector<unique_ptr<Procedure>> procedures;
        auto htmlPageTemplate = readFile("html/page-template-1.html") + "[TITLE]" +
            readFile("html/page-template-2.html") + "[BODY]" + readFile("html/page-template-3.html");
        createDirectory("../obj");
        createDirectory("../obj/doc-temp");
        createDirectory("../obj/doc-temp/diagrams-cp437");
        createDirectory("../obj/doc-html");
        forEachFile("../obj/doc-temp/topics-cp437", [&outputTxt, &htmlPageTemplate](auto filename) -> void {
            auto topic = filename.substr(0, filename.length() - 4);
            buildTopic(
                string("../obj/doc-temp/topics-cp437/") + filename, string("topics/") + filename, topic, &outputTxt,
                htmlPageTemplate);
        });
        forEachFile(
            "../obj/doc-temp/procedures-cp437", [&outputTxt, &htmlPageTemplate, &procedures](auto filename) -> void {
                procedures.push_back(buildProcedure(
                    string("../obj/doc-temp/procedures-cp437/") + filename, string("procedures/") + filename,
                    &outputTxt, htmlPageTemplate));
            });
        buildProcedureIndex(move(procedures), &outputTxt, htmlPageTemplate);
        writeFile("../obj/resources/help/help.txt", insertCp437Diagrams(outputTxt.str()));
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
