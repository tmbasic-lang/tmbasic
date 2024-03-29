#pragma once

#include "../common.h"

namespace shared {

struct membuf : std::streambuf {
    membuf(char* begin, char* end);
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override;
    pos_type seekpos(pos_type sp, std::ios_base::openmode which) override;
};

class MemoryIopstream : public iopstream {
   public:
    MemoryIopstream(char* begin, char* end);
    inline _Cdecl ~MemoryIopstream() override = default;

   private:
    std::unique_ptr<membuf> _membuf = nullptr;
};

}  // namespace shared
