const child_process = require("child_process");
const fs = require("fs");
const path = require("path");
const process = require("process");

const TOPIC_HOME = "doc";
const TITLE_HOME = "TMBASIC Documentation";
const TOPIC_BASIC_REFERENCE = "basicReference";
const TITLE_BASIC_REFERENCE = "BASIC Reference";
const TOPIC_PROCEDURE_INDEX = "procedureIndex";
const TITLE_PROCEDURE_INDEX = "Procedure Index";
const CHAR_DIAMOND = "\x04";
const HTML_DIAMOND = "♦";
const CHAR_BULLET = "\x07";
const HTML_BULLET = "•";
const CHAR_TRIANGLE_RIGHT = "\x10";
const HTML_TRIANGLE_RIGHT = "►";
const CHAR_OPEN_CIRCLE = "\x09";
const HTML_OPEN_CIRCLE = "○";

let outputTxt = "";
const procedureNames = [];

async function build() {
    await fs.promises.mkdir("../obj/doc-temp", { recursive: true });
    await fs.promises.mkdir("../obj/doc-temp/diagrams-cp437", { recursive: true });
    await fs.promises.mkdir("../obj/doc-txt", { recursive: true });
    await fs.promises.mkdir("../obj/doc-html", { recursive: true });
    await copyDir('../ext/notoserif', '../obj/doc-html/notoserif');
    await copyDir('../ext/opensans', '../obj/doc-html/opensans');
    await copyDir('../ext/oxygenmono', '../obj/doc-html/oxygenmono');
    await fs.promises.copyFile("html/style.css", "../obj/doc-html/style.css");
    await convertDiagramsToCp437();
    await forEachTopicFile(buildTopic);
    await forEachProcedureFile(buildProcedure);
    await buildProcedureIndex();
    await fs.promises.writeFile(`../obj/doc-txt/help.txt`, outputTxt, { options: "ascii" });
    await insertCp437Diagrams();
}

async function copyDir(src, dst) {
    await fs.promises.mkdir(dst, { recursive: true });
    const srcDir = await fs.promises.opendir(src);
    for await (const file of srcDir) {
        await fs.promises.copyFile(`${src}/${file.name}`, `${dst}/${file.name}`);
    }
}

function htmlEncode(str) {
    return str
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;");
}

async function writeHtmlPage(topic, text) {
    try {
        const template = await fs.promises.readFile("html/page-template.html", "utf8");
        const title = text.match(/h1\[([^\]]+)\]/)[1];
        const html = template
            .replace('[TITLE]', processHtml(title))
            .replace('[BODY]', processHtml(text));
        await fs.promises.writeFile(`../obj/doc-html/${topic}.html`, html);
    } catch (e) {
        console.log(`writeHtmlPage failed for ${topic}: ${text}`);
        throw e;
    }
}

function processText(str) {
    return str
        .replace(/t\[([^\]]+)\]/g, ($0, $1) => `{${$1}:${getTypeTopic($1)}}`)
        .replace(/p\[([^\]]+)\]/g, ($0, $1) => `{${$1}:procedure_${$1}}`)
        .replace(/i\[([^\]]+)\]/g, ($0, $1) => `'${$1}'`)
        .replace(/h1\[([^\]]+)\]/g, ($0, $1) => $1)
        .replace(/h2\[([^\]]+)\]/g, ($0, $1) => `<DIAMOND> ${$1}`)
        .replace(/h3\[([^\]]+)\]/g, ($0, $1) => $1)
        .replace(/code@([^@]+)@/g, ($0, $1) => indent($1))
        .replace(/nav@([^@]+)@/g, ($0, $1) => $1)
        .replace(/`([^`]+)`/g, ($0, $1) => `'${$1}'`)
        .replace(/li@([^@]+)@\n*/g, ($0, $1) => `<BULLET> ${$1}\n\n`)
        .replace(/ul@\n*([^@]+)\n*@\n*/g, ($0, $1) => $1)
        .replace(/<TOPIC_HOME>/g, TOPIC_HOME)
        .replace(/<TITLE_HOME>/g, TITLE_HOME)
        .replace(/<TOPIC_BASIC_REFERENCE>/g, TOPIC_BASIC_REFERENCE)
        .replace(/<TITLE_BASIC_REFERENCE>/g, TITLE_BASIC_REFERENCE)
        .replace(/<TOPIC_PROCEDURE_INDEX>/g, TOPIC_PROCEDURE_INDEX)
        .replace(/<TITLE_PROCEDURE_INDEX>/g, TITLE_PROCEDURE_INDEX)
        .replace(/<DIAMOND>/g, CHAR_DIAMOND)
        .replace(/<BULLET>/g, CHAR_BULLET)
        .replace(/<TRIANGLE_RIGHT>/g, CHAR_TRIANGLE_RIGHT)
        .replace(/<OPEN_CIRCLE>/g, CHAR_OPEN_CIRCLE)
        .replace(/{[^:"}]+:[^:"}]+}/g, $0 => "{" + $0);
}

function processHtml(str) {
    return str
        .replace(/<TOPIC_HOME>/g, TOPIC_HOME)
        .replace(/<TITLE_HOME>/g, TITLE_HOME)
        .replace(/<TOPIC_BASIC_REFERENCE>/g, TOPIC_BASIC_REFERENCE)
        .replace(/<TITLE_BASIC_REFERENCE>/g, TITLE_BASIC_REFERENCE)
        .replace(/<TOPIC_PROCEDURE_INDEX>/g, TOPIC_PROCEDURE_INDEX)
        .replace(/<TITLE_PROCEDURE_INDEX>/g, TITLE_PROCEDURE_INDEX)
        .replace(/<DIAMOND>/g, HTML_DIAMOND)
        .replace(/<BULLET>/g, HTML_BULLET)
        .replace(/<TRIANGLE_RIGHT>/g, HTML_TRIANGLE_RIGHT)
        .replace(/<OPEN_CIRCLE>/g, HTML_OPEN_CIRCLE)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/t\[([^\]]+)\]/g, ($0, $1) => `<a href="${getTypeTopic($1)}">${$1}</a>`)
        .replace(/p\[([^\]]+)\]/g, ($0, $1) => `<a href="procedure_${$1}.html">${$1}</a>`)
        .replace(/i\[([^\]]+)\]/g, ($0, $1) => `<i>${$1}</i>`)
        .replace(/\n*h1\[([^\]]+)\]\n*/g, ($0, $1) => `<h1>${$1}</h1>`)
        .replace(/\n*h2\[([^\]]+)\]\n*/g, ($0, $1) => `<h2>${$1}</h2>`)
        .replace(/\n*h3\[([^\]]+)\]\n*/g, ($0, $1) => `<h3>${$1}</h3>`)
        .replace(/code@\n*([^@]+)\n*@\n*/g, ($0, $1) => `<div class="code"><tt><div>${replaceIndentChars(syntaxColorCode($1)).trimEnd().replace(/\n/g, "</div><div>")}</div></tt></div>`.replace(/<div><\/div>/g, "<pre></pre>"))
        .replace(/nav@([^@]+)@/g, ($0, $1) => `<navbar>${$1}</navbar>`)
        .replace(/`([^`]+)`/g, ($0, $1) => `<tt>${$1}</tt>`)
        .replace(/<\/h1><br>/g, "</h1>")
        .replace(/<\/h2><br>/g, "</h2>")
        .replace(/<\/h3><br>/g, "</h3>")
        .replace(/<\/div><br>/g, "</div>")
        .replace(/<\/navbar><br>/g, "</navbar>")
        .replace(/<\/pre><br>/g, "</pre>")
        .replace(/\n*li@\n*([^@]+)\n*@\n*/g, ($0, $1) => `<li>${$1}</li>`)
        .replace(/\n*ul@\n*([^@]+)\n*@\n*/g, ($0, $1) => `<ul>${$1}</ul>`)
        .replace(/{([^:]+):([^}]+)}/g, ($0, $1, $2) => `<a href=\"${$2}.html\">${$1}</a>`)
        .replace(/dia\[([^\]]+)\]/g, ($0, $1) => `<pre class="diagram">${getDiagramHtml($1)}</pre>`)
        .replace(/-----/g, "<hr>")
        .replace(/\n/g, "<br>");
}

function syntaxColorCode(str) {
    return str.replace(/"[^"]*"/g, $0 => `<span class="string">${$0}</span>`);
}

function replaceIndentChars(str) {
    return str.replace(/    /g, $0 => `<span class="indent">${$0}</span>`);
}

async function convertDiagramsToCp437() {
    const dir = await fs.promises.opendir("diagrams");
    for await (const file of dir) {
        await convertDiagramToCp437(file.name);
    }
}

function convertDiagramToCp437(filename) {
    return new Promise(resolve =>
    {
        const utf8FilePath = "diagrams/" + filename;
        const cp437FilePath = "obj/doc-temp/diagrams-cp437/" + filename;
        console.log(`iconv -o ${cp437FilePath}`);
        child_process.execFile(
            "iconv",
            [
                "-f", "utf8",
                "-t", "cp437",
                "-o", "../" + cp437FilePath,
                utf8FilePath
            ], () => resolve()
        );
    });
}

async function insertCp437Diagrams() {
    const dir = await fs.promises.opendir("diagrams");
    for await (const file of dir) {
        await insertCp437Diagram(file.name);
    }
}

function insertCp437Diagram(filename) {
    return new Promise((resolve, reject) => {
        const cp437FilePath = "../obj/doc-temp/diagrams-cp437/" + filename;
        const name = filename.replace(".txt", "");
        console.log(`insert-cp437-diagram ${name}`);
        child_process.execFile(
            `../obj/insert-cp437-diagram`,
            [
                name,
                cp437FilePath,
                "../obj/doc-txt/help.txt"
            ], (error, stdout, stderr) => {
                if (error) {
                    reject(error);
                }
                process.stdout.write(stdout);
                process.stderr.write(stderr);
                resolve();
            }
        );
    });
}

function getDiagramHtml(name) {
    return fs.readFileSync(`diagrams/${name}.txt`, { encoding: "utf8" });
}

function getTypeTopic(type) {
    return "type_" + type.split(' ')[0];
}

async function forEachTopicFile(asyncFunc) {
    const dir = await fs.promises.opendir("topics");
    for await (const file of dir) {
        await asyncFunc("topics/" + file.name);
    }
}

async function buildTopic(inputFilePath) {
    const input = await fs.promises.readFile(inputFilePath, "ascii");
    const topic = path.basename(inputFilePath).replace(".txt", "");
    outputTxt += `.topic ${topic}\n` + processText(input.trim()) + "\n";
    await writeHtmlPage(topic, input);
}

async function forEachProcedureFile(asyncFunc) {
    const dir = await fs.promises.opendir("procedures");
    for await (const file of dir) {
        await asyncFunc("procedures/" + file.name);
    }
}

async function buildProcedureIndex() {
    let o = "";
    o += `nav@{<TITLE_HOME>:<TOPIC_HOME>} <TRIANGLE_RIGHT> {<TITLE_BASIC_REFERENCE>:<TOPIC_BASIC_REFERENCE>}@\n\nh1[${TITLE_PROCEDURE_INDEX}]\n\n`;
    o += "ul@";
    procedureNames.sort();
    for (const name of procedureNames) {
        o += `li@{${name}:procedure_${name}}@\n`;
    }
    o += "@\n";
    const filePath = "../obj/doc-temp/procedureIndex.txt";
    await fs.promises.writeFile(filePath, o, "ascii");
    await buildTopic(filePath);
}

async function buildProcedure(inputFilePath) {
    const input = await fs.promises.readFile(inputFilePath, "ascii");
    const procedure = parseProcedure(input);
    const topicName = "procedure_" + procedure.name;
    procedureNames.push(procedure.name);
    const text = formatProcedureText(topicName, procedure);
    outputTxt += `.topic ${topicName}\n` + processText(text) + "\n";
    await writeHtmlPage(topicName, text);
}

function parseProcedure(input) {
    const lines = input.split("\n");
    
    let procedure = null;
    let overload = null;

    for (let i = 0; i < lines.length; i++) {
        function readBlock() {
            let block = "";
            while (!lines[i + 1].startsWith(".")) {
                i++;
                block += lines[i] + "\n";
            }
            return block.trim();
        }

        const line = lines[i];
        if (line.startsWith(".")) {
            const verb = line.split(' ')[0];
            const rest = line.substring(verb.length + 1);
            switch (verb) {
                case ".procedure":
                    procedure = eval("({" + rest + "})");
                    procedure.overloads = [];
                    break;
                case ".overload":
                    overload = { parameters: [], examples: [] };
                    procedure.overloads.push(overload);
                    break;
                case ".description":
                    if (overload.hasOwnProperty("description")) {
                        throw new Error("Duplicate description");
                    } else {
                        overload.description = readBlock();
                    }
                    break;
                case ".parameter": {
                    const parameter = eval("({" + rest + "})");
                    parameter.description = readBlock();
                    overload.parameters.push(parameter);
                    break;
                }
                case ".return":
                    overload.returns = eval("({" + rest + "})");
                    overload.returns.description = readBlock();
                    break;
                case ".example":
                    example = { description: readBlock() };
                    overload.examples.push(example);
                    break;
                case ".example-code":
                    if (example.hasOwnProperty("code")) {
                        throw new Error("Duplicate example-code");
                    } else {
                        example.code = readBlock();
                    }
                    break;
                case ".example-output":
                    if (example.hasOwnProperty("output")) {
                        throw new Error("Duplicate example-output");
                    } else {
                        example.output = readBlock();
                    }
                case ".":
                    break;
            }
        } else if (line.trim() == "") {
            // ignore
        } else {
            throw new Error("Unexpected line: " + line);
        }
    }

    return procedure;
}

function formatProcedureText(topicName, procedure) {
    let o = "";
    o += `nav@{<TITLE_HOME>:<TOPIC_HOME>} <TRIANGLE_RIGHT> {<TITLE_BASIC_REFERENCE>:<TOPIC_BASIC_REFERENCE>} <TRIANGLE_RIGHT> {<TITLE_PROCEDURE_INDEX>:<TOPIC_PROCEDURE_INDEX>}@\n\n`;
    o += `h1["${procedure.name}" Procedure]\n\n`;

    for (const overload of procedure.overloads) {
        const isFunction = overload.hasOwnProperty("returns");

        o += "h2[" + (isFunction ? "function " : "sub ") + procedure.name + "(";

        let isFirstParameter = true;
        for (const parameter of overload.parameters) {
            if (!isFirstParameter) {
                o += ", ";
            }
            o += `i[${parameter.name}] as t[${parameter.type}]`
            isFirstParameter = false;
        }
        o += ")";
        if (isFunction) {
            o += ` as t[${overload.returns.type}]`;
        }
        o += "]\n\n";

        o += `${overload.description}\n\n`

        if (overload.parameters.length > 0) {
            o += "h3[Parameters]\n\nul@";
            for (const parameter of overload.parameters) {
                o += `li@i[${parameter.name}] as t[${parameter.type}]: ${parameter.description}@`;
            }
            o += "@";
        }

        if (isFunction) {
            o += `h3[Return value]\n\n`;
            o += `${overload.returns.description}\n\n`;
        }

        for (const example of overload.examples) {
            o += "h3[Example]\n\n";
            if (example.description.trim() != "") {
                o += `${example.description}\n\n`;
            }
            o += `Code:\ncode@${example.code}@\n\n`;
            o += `Output:\ncode@${example.output}@\n\n`;
        }

        o += "-----\n\n";
    }

    return o;
}

function indent(str) {
    return str.split("\n").map(x => `  ${x}`).join("\n");
}

build();
