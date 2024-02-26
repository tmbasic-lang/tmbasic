#pragma once

#include "../common.h"
#include "Interpreter.h"
#include "String.h"

namespace vm {

class BasicLabel : public TLabel {
   public:
    BasicLabel(int64_t id, Interpreter* interpreter);
    virtual ~BasicLabel();
    static int64_t newLabel(Interpreter* interpreter);
    boost::intrusive_ptr<String> getBasicText();
    void setText(const String& newText);

   private:
    int64_t _id;
    Interpreter* _interpreter;
};

}  // namespace vm
