#pragma once

#include "../common.h"
#include "Interpreter.h"

namespace vm {

class BasicForm : public TDialog {
   public:
    BasicForm(int64_t id, Interpreter* interpreter);
    virtual ~BasicForm();
    static int64_t newForm(Interpreter* interpreter);

   private:
    int64_t _id;
    Interpreter* _interpreter = nullptr;
};

}  // namespace vm
