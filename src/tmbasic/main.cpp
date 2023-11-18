#include "tmbasic/App.h"
#include "../compiler/CompilerException.h"
#include "../compiler/Publisher.h"
#include "../compiler/TargetPlatform.h"
#include "../compiler/compileProgram.h"
#include "../util/console.h"

#include <CLI11.hpp>

static void runIde(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleTitle(TEXT("TMBASIC"));
#endif
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
}

static void runProgram(const std::string& filename) {
    compiler::CompiledProgram program{};
    compiler::SourceProgram source{};
    source.load(filename);
    compiler::compileProgram(source, &program);

    vm::Interpreter interpreter{ &program.vmProgram, &std::cin, &std::cout };
    interpreter.init(program.vmProgram.startupProcedureIndex);
    while (interpreter.run(10000)) {
    }

    auto error = interpreter.getError();
    if (error.has_value()) {
        std::cerr << "Uncaught error:\n" << error->message << std::endl;
    }
}

static void publishProgram(const std::string& filename, const std::vector<std::string>& platformStrings) {
    compiler::CompiledProgram program{};
    compiler::SourceProgram source{};
    source.load(filename);
    compiler::compileProgram(source, &program);

    std::vector<compiler::TargetPlatform> publishPlatforms{};
    publishPlatforms.reserve(platformStrings.size());
    for (const auto& platformString : platformStrings) {
        publishPlatforms.push_back(compiler::parsePlatformName(platformString));
    }

    if (publishPlatforms.empty()) {
        publishPlatforms.push_back(compiler::getNativeTargetPlatform());
    }

    compiler::Publisher publisher{ program, filename };
    for (auto platform : publishPlatforms) {
        auto outputFilePath = publisher.publish(platform);
        std::cout << outputFilePath << std::endl;
    }
}

int main(int argc, char** argv) {
    util::setUtf8Locale();

    CLI::App cli{ "TMBASIC programming language" };

    // run
    {
        auto* command = cli.add_subcommand("run", "Runs a TMBASIC program.");

        std::string filename{};
        command->add_option("filename", filename, "Path to a TMBASIC source file (.bas)")->required();

        command->callback([&]() { runProgram(filename); });
    }

    // publish
    {
        auto* command = cli.add_subcommand("publish", "Publishes a TMBASIC program to a native executable.");

        std::string filename{};
        command->add_option("filename", filename, "Path to a TMBASIC source file (.bas)")->required();

        std::vector<std::string> platformStrings{};
        std::ostringstream ss{};
        ss << "Target platform(s). Options are:\n";
        for (auto platform : compiler::getTargetPlatforms()) {
            ss << "   -t " << compiler::getPlatformName(platform) << "\n";
        }
        command->add_option("-t,--target", platformStrings, ss.str());

        command->callback([&]() { publishProgram(filename, platformStrings); });
    }

    try {
        cli.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return cli.exit(e);
    } catch (compiler::CompilerException& ex) {
        std::cerr << "Error in \"" << ex.token.sourceMember->identifier << "\"\nLn " << ex.token.lineIndex + 1
                  << ", Col " << ex.token.columnIndex + 1 << "\n"
                  << ex.message << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error." << std::endl;
        return 1;
    }

    if (cli.get_subcommands().empty()) {
        runIde(argc, argv);
    }

    return 0;
}
