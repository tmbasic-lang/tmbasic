#include "compiler/CompiledProgram.h"
#include "compiler/makeExeFile.h"
#include "shared/serializedProgramConstants.h"
#include "compiler/TargetPlatform.h"
#include "shared/filesystem.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace compiler {

// See ProgramReader in vm/Program.cpp.
class ProgramWriter {
   public:
    std::vector<uint8_t> vec;

    template <typename TInt>
    void writeInt(TInt value) {
        std::array<uint8_t, sizeof(TInt)> bytes{};
        memcpy(bytes.data(), &value, sizeof(TInt));
        vec.insert(vec.end(), bytes.begin(), bytes.end());
    }

    void writeDecimal(const decimal::Decimal& dec) {
        auto triple = dec.as_uint128_triple();
        writeInt<uint8_t>(triple.sign);
        writeInt<uint64_t>(triple.hi);
        writeInt<uint64_t>(triple.lo);
        writeInt<int64_t>(triple.exp);
    }

    void writeString(const std::string& utf8) {
        writeInt<uint32_t>(static_cast<uint32_t>(utf8.size()));
        vec.insert(vec.end(), utf8.begin(), utf8.end());
    }
};

std::vector<uint8_t> CompiledProgram::serialize() const {
    ProgramWriter writer{};
    writer.writeInt<uint32_t>(static_cast<uint32_t>(vmStartupProcedureIndex));

    for (const auto& p : vmProcedures) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kProcedureByte));
        writer.writeInt<uint32_t>(static_cast<uint32_t>(p.size()));
        writer.vec.insert(writer.vec.end(), p.begin(), p.end());
    }

    for (const auto& v : vmGlobalValues) {
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kValueByte));
        writer.writeDecimal(v);
    }

    for (const auto& p : vmGlobalObjects) {
        const auto& objectType = p.first;
        const auto& initialString = p.second;
        writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kObjectByte));
        writer.writeInt<uint8_t>(static_cast<uint8_t>(objectType));
        if (objectType == shared::ObjectType::kString) {
            writer.writeString(initialString);
        }
    }

    writer.writeInt<uint8_t>(static_cast<uint8_t>(shared::kEofByte));

    return writer.vec;
}

static std::string getTempExeFilename(TargetPlatform platform) {
#ifdef _WIN32
    auto pid = static_cast<int64_t>(GetCurrentProcessId());
#else
    auto pid = static_cast<int64_t>(getpid());
#endif
    return fmt::format("tmbasic-debug-{}{}", pid, compiler::getPlatformExeExtension(platform));
}

void CompiledProgram::run() const {
    // Write a temporary EXE file
    auto pcode = serialize();
    auto nativePlatform = compiler::getNativeTargetPlatform();
    auto exeData = compiler::makeExeFile(pcode, nativePlatform);
    auto tempFilePath = shared::getTempFilePath(getTempExeFilename(nativePlatform));
    std::ofstream f{ tempFilePath, std::ios::out | std::ios::binary };
    f.write(reinterpret_cast<const char*>(exeData.data()), static_cast<std::streamsize>(exeData.size()));
    f.close();
#ifndef _WIN32
    chmod(tempFilePath.c_str(), 0777);
#endif

    // Execute it.
#ifdef _WIN32
    // Shell is always available in Windows, so just use std::system().
    auto args = fmt::format("\"{}\"", tempFilePath);
    std::system(args.c_str());
#else
    // Shell might NOT be available in Linux, so use fork/exec.
    // Consider a chroot environment containing tmbasic and nothing else.
    auto pid = fork();
    if (pid == 0) {
        // Child process. Use exec.
        execl(tempFilePath.c_str(), tempFilePath.c_str(), nullptr);

        // If it was successful, then execution will never reach this point.
        // If we're here, then it failed.
        std::cerr << "Failed to start the program: " << strerror(errno) << '\n';

        // ChatGPT says it's important to use _exit and not exit here.
        _exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process. Wait for the child process to exit.
        int status{};
        waitpid(pid, &status, 0);
    } else {
        std::cout << "Failed to fork a new process: " << strerror(errno) << '\n';
    }
#endif

    // Delete the temp file
    shared::deleteFile(tempFilePath);
}

}  // namespace compiler
