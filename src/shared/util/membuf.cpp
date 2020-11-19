#include "shared/util/membuf.h"

namespace util {

membuf::membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
}

std::streambuf::pos_type membuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) {
    if (dir == std::ios_base::cur) {
        gbump(off);
    } else if (dir == std::ios_base::end) {
        setg(eback(), egptr() + off, egptr());
    } else if (dir == std::ios_base::beg) {
        setg(eback(), eback() + off, egptr());
    }
    return gptr() - eback();
}

std::streambuf::pos_type membuf::seekpos(pos_type sp, std::ios_base::openmode which) {
    return seekoff(sp - pos_type(off_type(0)), std::ios_base::beg, which);
}

}  // namespace util
