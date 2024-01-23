#include "filesystem.h"
#include "shared/Error.h"
#include "shared/path.h"
#include "shared/win.h"

// std::filesystem support requires macOS 10.15, and we are targeting 10.13, so don't use it on macOS.
// instead we will fall back to standard POSIX stuff.
#ifdef __APPLE__
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#endif

// Linux and macOS support opendir/readdir/closedir, but Windows does not.
#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>  // NOLINT(build/include)
#include <sys/types.h>
#endif

namespace shared {

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
    size_t trimmedPathLength = path.size();
    while (trimmedPathLength > 0 && path.at(trimmedPathLength - 1) == '\\') {
        trimmedPathLength--;
    }
    auto trimmedPath = path.substr(0, trimmedPathLength) + "\\*";

    WIN32_FIND_DATAW findFileData{};
    auto wpath = shared::winUtf8ToUtf16(trimmedPath);

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
        auto isSingleDot = wcscmp(findFileData.cFileName, L".") == 0;
        auto isDoubleDot = wcscmp(findFileData.cFileName, L"..") == 0;
        if (!isSingleDot && !isDoubleDot) {
            std::wstring filenameUtf16{ findFileData.cFileName };
            auto filenameUtf8 = shared::winUtf16ToUtf8(filenameUtf16);
            auto isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            if (wantDirsOrFiles == isDir) {
                result.push_back(shared::pathCombine(path, filenameUtf8));
            }
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
            }
            throw Error::fromDirectoryErrno(err, path);
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        auto isDir = entry->d_type == DT_DIR;
        if (isDir == wantDirsOrFiles) {
            filePaths.push_back(shared::pathCombine(path, entry->d_name));
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
    auto parentDir = shared::getDirectoryName(path);
    if (!parentDir.empty() && parentDir != path) {
        createDirectory(parentDir);
    }

#ifdef _WIN32
    if (mkdir(path.c_str()) != 0) {
#else
    if (mkdir(path.c_str(), 0755) != 0) {
#endif
        auto err = errno;
        if (err == EEXIST) {
            // not an error
            return;
        }
        throw Error::fromDirectoryErrno(err, path);
    }
}

void deleteDirectory(const std::string& path, bool recursive) {
    if (recursive) {
        for (const auto& filePath : listFiles(path)) {
            if (unlink(filePath.c_str()) != 0) {
                auto err = errno;
                throw Error::fromFileErrno(err, filePath);
            }
        }

        for (const auto& subdirPath : listDirectories(path)) {
            deleteDirectory(subdirPath, true);
        }
    }
    if (rmdir(path.c_str()) != 0) {
        auto err = errno;
        throw Error::fromDirectoryErrno(err, path);
    }
}

std::string getTempFilePath(const std::string& filename) {
#ifdef __APPLE__
    const char* temp_path = getenv("TMPDIR");
    std::ostringstream ss{};
    if (temp_path != nullptr) {
        ss << temp_path << "/" << filename;
        return ss.str();
    } else {
        // Default to "/tmp" if TMPDIR is not set
        // If even /tmp doesn't exist, just return filename.
        struct stat sb {};
        if (stat("/tmp", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
            return filename;
        }
        ss << "/tmp/" << filename;
        return ss.str();
    }
#else
    try {
        auto tempPath = std::filesystem::temp_directory_path();
        return (tempPath / filename).string();
    } catch (const std::filesystem::filesystem_error& ex) {
        // It's possible that the temp directory doesn't exist.
        // Best to fall back to the current directory with a relative path.
        return filename;
    }
#endif
}

void deleteFile(const std::string& path) {
#ifdef __APPLE__
    unlink(path.c_str());
#else
    std::filesystem::remove(path);
#endif
}

}  // namespace shared
