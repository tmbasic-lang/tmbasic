#include "filesystem.h"
#include "Error.h"
#include "../util/path.h"
#include "../util/win.h"

// std::filesystem support requires macOS 10.15, and we are targeting 10.13, so don't use it on macOS.
// instead we will fall back to standard POSIX stuff.
#ifdef __APPLE__
#include <libgen.h>
#include <sys/stat.h>
#endif

// Linux and macOS support opendir/readdir/closedir, but Windows does not.
#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace vm {

#ifndef _WIN32
struct DirClose {
    void operator()(DIR* dir) {
        if (dir != nullptr) {
            closedir(dir);
        }
    }
};
#endif

#ifdef _WIN32
class FindFileHandle {
   public:
    HANDLE handle;
    explicit FindFileHandle(HANDLE h) : handle(h) {}
    ~FindFileHandle() {
        if (handle != INVALID_HANDLE_VALUE) {
            FindClose(handle);
        }
    }
    HANDLE get() const { return handle; }
};
#endif

// wantDirsOrFiles: true = want directories, false = want files
static std::vector<std::string> listFilesOrDirectories(const std::string& path, bool wantDirsOrFiles) {
#ifdef _WIN32
    WIN32_FIND_DATAW findFileData{};
    auto wpath = util::winUtf8ToUtf16(path);

    FindFileHandle hFind{ FindFirstFileW(wpath.c_str(), &findFileData) };
    if (hFind.get() == INVALID_HANDLE_VALUE) {
        auto lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND) {
            return {};
        }
        throw Error::fromDirectoryWinError(lastError, path);
    }

    std::vector<std::string> result;
    do {
        if (wantDirsOrFiles == (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            result.push_back(util::pathCombine(path, util::winUtf16ToUtf8(findFileData.cFileName)));
        }
    } while (FindNextFileW(hFind.get(), &findFileData) != 0);

    auto lastError = GetLastError();
    if (lastError != ERROR_NO_MORE_FILES) {
        throw Error::fromDirectoryWinError(lastError, path);
    }

    return result;
#else
    std::vector<std::string> filePaths{};
    auto dir = std::unique_ptr<DIR, DirClose>(opendir(path.c_str()));
    if (dir == nullptr) {
        auto err = errno;
        throw Error::fromDirectoryErrno(err, path);
    }
    while (true) {
        // from the readdir man page:
        //    "If the end of the directory stream is reached, NULL is returned
        //    and errno is not changed.  If an error occurs, NULL is returned
        //    and errno is set to indicate the error.  To distinguish end of
        //    stream from an error, set errno to zero before calling readdir()
        //    and then check the value of errno if NULL is returned."
        // https://man7.org/linux/man-pages/man3/readdir.3.html
        errno = 0;
        const auto* entry = readdir(dir.get());
        if (entry == nullptr) {
            auto err = errno;
            if (err == 0) {
                break;
            } else {
                throw Error::fromDirectoryErrno(err, path);
            }
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        auto isDir = entry->d_type == DT_DIR;
        if (isDir == wantDirsOrFiles) {
            filePaths.push_back(util::pathCombine(path, entry->d_name));
        }
    }
    return filePaths;
#endif
}

std::vector<std::string> listFiles(const std::string& path) {
    return listFilesOrDirectories(path, false);
}

std::vector<std::string> listDirectories(const std::string& path) {
    return listFilesOrDirectories(path, true);
}

void createDirectory(const std::string& path) {
#ifdef _WIN32
    try {
        std::filesystem::create_directories(path);
    } catch (const std::runtime_error& e) {
        throw Error(ErrorCode::kIoFailure, fmt::format("Failed to create directory \"{}\": {}", path, e.what()));
    }
#else
    auto parentDir = util::getDirectoryName(path);
    if (!parentDir.empty()) {
        createDirectory(parentDir);
    }

    if (mkdir(path.c_str(), 0755) != 0) {
        auto err = errno;
        if (err == EEXIST) {
            // not an error
            return;
        }
        throw Error::fromDirectoryErrno(err, path);
    }
#endif
}

void deleteDirectory(const std::string& path, bool recursive) {
#ifdef _WIN32
    try {
        if (recursive) {
            std::filesystem::remove_all(path);
        } else {
            auto wpath = util::winUtf8ToUtf16(path);
            auto result = RemoveDirectoryW(wpath.c_str());
            if (result == 0) {
                auto lastError = GetLastError();
                throw Error::fromDirectoryWinError(lastError, path);
            }
        }
    } catch (const std::runtime_error& e) {
        throw Error(ErrorCode::kIoFailure, fmt::format("Failed to delete directory \"{}\": {}", path, e.what()));
    }
#else
    if (recursive) {
        auto filePaths = listFiles(path);
        for (const auto& filePath : filePaths) {
            if (unlink(filePath.c_str()) != 0) {
                auto err = errno;
                throw Error::fromFileErrno(err, filePath);
            }
        }

        auto dirPaths = listDirectories(path);
        for (const auto& filePath : dirPaths) {
            deleteDirectory(filePath, true);
        }
    }
    if (rmdir(path.c_str()) != 0) {
        auto err = errno;
        throw Error::fromDirectoryErrno(err, path);
    }
#endif
}

}  // namespace vm
