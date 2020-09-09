#pragma once

#include "Procedure.h"
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <vector>

namespace vm {

class Program {
    std::vector<boost::local_shared_ptr<Procedure>> procedures;
};

}  // namespace vm
