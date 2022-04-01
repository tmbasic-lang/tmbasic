#include "systemCall.h"
#include "Error.h"
#include "List.h"
#include "String.h"
#include "filesystem.h"
#include "util/path.h"

namespace vm {

void initSystemCallsFiles() {
    initSystemCall(SystemCall::kCreateDirectory, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        vm::createDirectory(path);
    });

    initSystemCall(SystemCall::kDeleteDirectory1, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        vm::deleteDirectory(path, false);
    });

    initSystemCall(SystemCall::kDeleteDirectory2, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto recursive = input.getValue(-1).getBoolean();
        vm::deleteDirectory(path, recursive);
    });

    initSystemCall(SystemCall::kDeleteFile, [](const auto& input, auto* /*result*/) {
        const auto& path = dynamic_cast<const String&>(input.getObject(-1));
        auto pathStr = path.toUtf8();
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
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedValue.setBoolean(access(path.c_str(), F_OK) == 0);
    });

    initSystemCall(SystemCall::kPathCombine, [](const auto& input, auto* result) {
        const auto& list = dynamic_cast<const ObjectList&>(input.getObject(-1));
        if (list.items.empty()) {
            result->returnedObject = boost::make_local_shared<String>("", 0);
            return;
        }
        auto path = dynamic_cast<const String&>(*list.items.at(0)).toUtf8();
        for (size_t i = 1; i < list.items.size(); i++) {
            auto component = dynamic_cast<const String&>(*list.items.at(i)).toUtf8();
            path = util::pathCombine(path, component);
        }
        result->returnedObject = boost::make_local_shared<String>(path);
    });

    initSystemCall(SystemCall::kPathDirectoryName, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getDirectoryName(path));
    });

    initSystemCall(SystemCall::kPathExtension, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getExtension(path));
    });

    initSystemCall(SystemCall::kPathFileName, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        result->returnedObject = boost::make_local_shared<String>(util::getFileName(path));
    });

    initSystemCall(SystemCall::kPathFileNameWithoutExtension, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
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
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto vec = listDirectories(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });

    initSystemCall(SystemCall::kListFiles, [](const auto& input, auto* result) {
        auto path = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
        auto vec = listFiles(path);
        ObjectListBuilder builder;
        for (const auto& s : vec) {
            builder.items.push_back(boost::make_local_shared<String>(s));
        }
        result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
    });

    initSystemCall(SystemCall::kReadFileBytes, [](const auto& input, auto* result) {
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
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
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
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
        auto filePath = dynamic_cast<const String&>(input.getObject(-1)).toUtf8();
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
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& bytesValueList = dynamic_cast<const ValueList&>(input.getObject(-1));
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
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& lines = dynamic_cast<const ObjectList&>(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        for (const auto& line : lines.items) {
            stream << dynamic_cast<const String&>(*line).toUtf8() << kNewLine;
            if (stream.fail()) {
                throw Error::fromFileErrno(errno, filePath);
            }
        }
    });

    initSystemCall(SystemCall::kWriteFileText, [](const auto& input, auto* /*result*/) {
        const auto& filePath = dynamic_cast<const String&>(input.getObject(-2)).toUtf8();
        const auto& text = dynamic_cast<const String&>(input.getObject(-1));
        std::ofstream stream{ filePath };
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
        stream << text.toUtf8();
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
    });
}

}  // namespace vm
