#include "vm/systemCall.h"
#include "vm/Error.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/filesystem.h"
#include "vm/castObject.h"
#include "util/path.h"

namespace vm {

// (path as String) as Boolean
void systemCallCreateDirectory(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& path = castString(input.getObject(-1))->value;
    vm::createDirectory(path);
}

// (path as String)
void systemCallDeleteDirectory1(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& path = castString(input.getObject(-1))->value;
    vm::deleteDirectory(path, false);
}

// (path as String, recursive as Boolean)
void systemCallDeleteDirectory2(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& path = castString(input.getObject(-1))->value;
    auto recursive = input.getValue(-1).getBoolean();
    vm::deleteDirectory(path, recursive);
}

// (filePath as String)
void systemCallDeleteFile(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& path = castString(input.getObject(-1));
    auto pathStr = path->value;
    if (unlink(pathStr.c_str()) != 0) {
        auto err = errno;
        if (err == ENOENT) {
            // not an error
            return;
        }
        throw Error::fromFileErrno(err, pathStr);
    }
}

// (filePath as String) as Boolean
void systemCallFileExists(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    result->returnedValue.setBoolean(access(path.c_str(), F_OK) == 0);
}

// (parts as List of String) as String
void systemCallPathCombine(const SystemCallInput& input, SystemCallResult* result) {
    const auto& list = *castObjectList(input.getObject(-1));
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
}

// (path as String) as String
void systemCallPathDirectoryName(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    result->returnedObject = boost::make_local_shared<String>(util::getDirectoryName(path));
}

// (path as String) as String
void systemCallPathExtension(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    result->returnedObject = boost::make_local_shared<String>(util::getExtension(path));
}

// (path as String) as String
void systemCallPathFileName(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    result->returnedObject = boost::make_local_shared<String>(util::getFileName(path));
}

// (path as String) as String
void systemCallPathFileNameWithoutExtension(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    result->returnedObject = boost::make_local_shared<String>(util::getFileNameWithoutExtension(path));
}

// () as String
void systemCallPathSeparator(const SystemCallInput& /*input*/, SystemCallResult* result) {
#ifdef _WIN32
    result->returnedObject = boost::make_local_shared<String>("\\", 1);
#else
    result->returnedObject = boost::make_local_shared<String>("/", 1);
#endif
}

// (path as String) as List of String
void systemCallListDirectories(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    auto vec = listDirectories(path);
    ObjectListBuilder builder;
    for (const auto& s : vec) {
        builder.items.push_back(boost::make_local_shared<String>(s));
    }
    result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
}

// (path as String) as List of String
void systemCallListFiles(const SystemCallInput& input, SystemCallResult* result) {
    auto path = castString(input.getObject(-1))->value;
    auto vec = listFiles(path);
    ObjectListBuilder builder;
    for (const auto& s : vec) {
        builder.items.push_back(boost::make_local_shared<String>(s));
    }
    result->returnedObject = boost::make_local_shared<ObjectList>(&builder);
}

// (filePath as String) as List of Number
void systemCallReadFileBytes(const SystemCallInput& input, SystemCallResult* result) {
    auto filePath = castString(input.getObject(-1))->value;
    std::ifstream stream{ filePath, std::ios::binary };
    if (stream.fail()) {
        throw Error::fromFileErrno(errno, filePath);
    }

    constexpr std::size_t chunkSize = 16 * 1024;  // 16KB
    std::vector<char> buffer(chunkSize);
    std::vector<char> bytes;
    while (!stream.eof()) {
        stream.read(buffer.data(), chunkSize);
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + stream.gcount());
    }
    if (!stream.eof()) {
        throw Error::fromFileErrno(errno, filePath);
    }

    ValueListBuilder vlb{};
    for (auto& byte : bytes) {
        vlb.items.push_back(Value{ byte });
    }
    result->returnedObject = boost::make_local_shared<ValueList>(&vlb);
}

// (filePath as String) as List of String
void systemCallReadFileLines(const SystemCallInput& input, SystemCallResult* result) {
    auto filePath = castString(input.getObject(-1))->value;
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
}

// (filePath as String) as String
void systemCallReadFileText(const SystemCallInput& input, SystemCallResult* result) {
    auto filePath = castString(input.getObject(-1))->value;
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
}

// (filePath as String, bytes as List of Number)
void systemCallWriteFileBytes(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& filePath = castString(input.getObject(-2))->value;
    const auto& bytesValueList = *castValueList(input.getObject(-1));
    std::vector<char> bytes;
    bytes.reserve(bytesValueList.items.size());
    for (const auto& value : bytesValueList.items) {
        bytes.push_back(static_cast<char>(value.getInt32()));
    }
    std::ofstream stream{ filePath };
    if (stream.fail()) {
        throw Error::fromFileErrno(errno, filePath);
    }
    constexpr size_t kChunkSize = 16 * 1024;  // 16KB
    for (size_t i = 0; i < bytes.size(); i += kChunkSize) {
        size_t remainingBytes = bytes.size() - i;
        size_t chunkBytes = std::min(kChunkSize, remainingBytes);
        stream.write(&bytes.at(i), static_cast<std::streamsize>(chunkBytes));
        if (stream.fail()) {
            throw Error::fromFileErrno(errno, filePath);
        }
    }
}

// (filePath as String, lines as List of String)
void systemCallWriteFileLines(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& filePath = castString(input.getObject(-2))->value;
    const auto& lines = *castObjectList(input.getObject(-1));
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
}

// (filePath as String, text as String)
void systemCallWriteFileText(const SystemCallInput& input, SystemCallResult* /*result*/) {
    const auto& filePath = castString(input.getObject(-2))->value;
    const auto& text = castString(input.getObject(-1));
    std::ofstream stream{ filePath };
    if (stream.fail()) {
        throw Error::fromFileErrno(errno, filePath);
    }
    stream << text->value;
    if (stream.fail()) {
        throw Error::fromFileErrno(errno, filePath);
    }
}

}  // namespace vm
