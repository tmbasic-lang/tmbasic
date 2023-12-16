#include "vm/systemCall.h"
#include "vm/Error.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/filesystem.h"
#include "vm/castObject.h"
#include "util/path.h"

namespace vm {

void initSystemCallsFiles() {
    initSystemCall(SystemCall::kCreateDirectory, [](const auto& input, auto* /*result*/) {
        const auto& path = castString(input.getObject(-1)).value;
        vm::createDirectory(path);
    });

    initSystemCall(SystemCall::kDeleteDirectory1, [](const auto& input, auto* /*result*/) {
        const auto& path = castString(input.getObject(-1)).value;
        vm::deleteDirectory(path, false);
    });

    initSystemCall(SystemCall::kDeleteDirectory2, [](const auto& input, auto* /*result*/) {
        const auto& path = castString(input.getObject(-1)).value;
        auto recursive = input.getValue(-1).getBoolean();
        vm::deleteDirectory(path, recursive);
    });

    initSystemCall(SystemCall::kDeleteFile, [](const auto& input, auto* /*result*/) {
        const auto& path = castString(input.getObject(-1));
        auto pathStr = path.value;
        if (unlink(pathStr.c_str()) != 0) {
            auto err = errno;
            if (err == ENOENT) {
                // not an error
                return;
            }
            throw Error::fromFileErrno(err, pathStr);
        }
    });

    initSystemCall(SystemCall::kFileExists, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        result->returnedValue.setBoolean(access(path.c_str(), F_OK) == 0);
    });

    initSystemCall(SystemCall::kPathCombine, [](const auto& input, auto* result) {
        const auto& list = castObjectList(input.getObject(-1));
        if (list.items.empty()) {
            result->returnedObject = boost::make_local_shared<String>("", 0);
            return;
        }
        auto path = castString(*list.items.at(0)).value;
        for (size_t i = 1; i < list.items.size(); i++) {
            auto component = castString(*list.items.at(i)).value;
            path = util::pathCombine(path, component);
        }
        result->returnedObject = boost::make_local_shared<String>(path);
    });

    initSystemCall(SystemCall::kPathDirectoryName, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        result->returnedObject = boost::make_local_shared<String>(util::getDirectoryName(path));
    });

    initSystemCall(SystemCall::kPathExtension, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        result->returnedObject = boost::make_local_shared<String>(util::getExtension(path));
    });

    initSystemCall(SystemCall::kPathFileName, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        result->returnedObject = boost::make_local_shared<String>(util::getFileName(path));
    });

    initSystemCall(SystemCall::kPathFileNameWithoutExtension, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        result->returnedObject = boost::make_local_shared<String>(util::getFileNameWithoutExtension(path));
    });

    initSystemCall(SystemCall::kPathSeparator, [](const auto& /*input*/, auto* result) {
#ifdef _WIN32
        result->returnedObject = boost::make_local_shared<String>("\\", 1);
#else
        result->returnedObject = boost::make_local_shared<String>("/", 1);
#endif
    });

    initSystemCall(SystemCall::kListDirectories, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        auto vec = listDirectories(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });

    initSystemCall(SystemCall::kListFiles, [](const auto& input, auto* result) {
        auto path = castString(input.getObject(-1)).value;
        auto vec = listFiles(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });

    initSystemCall(SystemCall::kReadFileBytes, [](const auto& input, auto* result) {
        auto filePath = castString(input.getObject(-1)).value;
        std::ifstream stream{ filePath };
        stream.seekg(0, std::ios::end);
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }

        std::vector<char> bytes(stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read(bytes.data(), bytes.size());
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }

        ValueListBuilder vlb{};
        for (auto& byte : bytes) {
            vlb.items.push_back(Value{ byte });
        }
        result->returnedObject = boost::make_local_shared<ValueList>(&vlb);
    });

    initSystemCall(SystemCall::kReadFileLines, [](const auto& input, auto* result) {
        auto filePath = castString(input.getObject(-1)).value;
        std::ifstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        ObjectListBuilder builder{};
        std::string line;
        while (std::getline(stream, line)) {
            if (stream.fail()) {
                throw Error::fromFileErrno(errno, filePath);
            }
            builder.items.push_back(boost::make_local_shared<String>(line));
        }
        if (stream.fail() && !stream.eof()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });

    initSystemCall(SystemCall::kReadFileText, [](const auto& input, auto* result) {
        auto filePath = castString(input.getObject(-1)).value;
        std::ifstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        std::ostringstream ss;
        ss << stream.rdbuf();
        if (stream.fail() && !stream.eof()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        result->returnedObject = boost::make_local_shared<String>(ss.str());
    });

    initSystemCall(SystemCall::kWriteFileBytes, [](const auto& input, auto* /*result*/) {
        const auto& filePath = castString(input.getObject(-2)).value;
        const auto& bytesValueList = castValueList(input.getObject(-1));
        std::vector<char> bytes;
        bytes.reserve(bytesValueList.items.size());
        for (const auto& value : bytesValueList.items) {
            bytes.push_back(static_cast<char>(value.getInt32()));
        }
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        stream.write(bytes.data(), bytes.size());
    });

    initSystemCall(SystemCall::kWriteFileLines, [](const auto& input, auto* /*result*/) {
        const auto& filePath = castString(input.getObject(-2)).value;
        const auto& lines = castObjectList(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        for (const auto& line : lines.items) {
            stream << castString(*line).value << kNewLine;
            if (stream.fail()) {
                throw Error::fromFileErrno(errno, filePath);
            }
        }
    });

    initSystemCall(SystemCall::kWriteFileText, [](const auto& input, auto* /*result*/) {
        const auto& filePath = castString(input.getObject(-2)).value;
        const auto& text = castString(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        stream << text.value;
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
    });
}

}  // namespace vm
