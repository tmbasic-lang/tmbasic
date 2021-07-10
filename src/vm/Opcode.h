#pragma once

#include "../common.h"

namespace vm {

enum class Opcode {
    // data types:
    // i64 - 8b signed integer
    // u8 - 1b unsigned integer
    // u16 - 2b unsigned integer
    // u32 - 4b unsigned integer
    // utf8 - null-terminated string
    // localVal* - 2b index of local value
    // localObj* - 2b index of local object
    // globalVal* - 2b index of global value
    // globalObj* - 2b index of global object
    // inst* - 4b instruction index
    // proc* - 4b procedure index

    // stack pointers point to the first unused slot and grow from 0 to bigger numbers
    // vsp - value stack pointer
    // osp - object stack pointer

    kExit,  // ends the program

    // stack manipulation
    kPushImmediateInt64,  // <i64 operand> pushv operand;
    kPushImmediateUtf8,   // <u32 length, utf8 string> pusho string
    kPopValue,            // popv;
    kPopObject,           // popo;

    // variables
    kInitLocals,          // <u16 numVals, u16 numObjs>
    kPushArgumentValue,   // <u8 index> pushv index;
    kPushArgumentObject,  // <u8 index> pusho index;
    kPushGlobalValue,     // <globalVal* src> pushv *src
    kPushGlobalObject,    // <globalObj* src> pusho *src
    kSetGlobalValue,      // <globalVal* dst> *dst = vsp[-1]; popv;
    kSetGlobalObject,     // <globalObj* dst> *dst = ospTop; popo;
    kPushLocalValue,      // <localVal* src> pushv *src
    kPushLocalObject,     // <localObj* src> pusho *src
    kSetLocalValue,       // <localVal* dst> *dst = vsp[-1]; popv;
    kSetLocalObject,      // <localObj* dst> *dst = ospTop; popo;

    // flow control
    kJump,           // <inst* index> goto byte index in the current procedure
    kBranchIfTrue,   // <inst* index> if (vsp[-1]) goto index in current proc
    kBranchIfFalse,  // <inst* index> if (!vsp[-1]) goto index in current proc

    // procedures. flags: 1=return value | 2=return object
    kCallSub,                        // <proc* procIndex, u8 numVals, u8 numObjs>
    kCallFunctionValue,              // <proc* procIndex, u8 numVals, u8 numObjs>
    kCallFunctionObject,             // <proc* procIndex, u8 numVals, u8 numObjs>
    kSystemCallSub,                  // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallFunctionValue,        // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallFunctionObject,       // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallFunctionValueObject,  // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kReturn,                         // restores osp/vsp
    kReturnValue,                    // restores osp/vsp and pushes the return value vsp[-1]
    kReturnObject,                   // restores osp/vsp and pushes the return object ospTop

    // error handling
    kSetError,          // set error flag from code vsp[-1] and message ospTop; popv; popo;
    kClearError,        // clear error flag, don't clear message or code
    kBubbleError,       // set error flag
    kReturnIfError,     // if error is set, return
    kBranchIfNotError,  // <inst* index> if no error, then jump
    kPushErrorMessage,  // pusho message
    kPushErrorCode,     // pushv code

    // objects
    kRecordNew,        // <u16 numVals, u16 numObjs> pushes record with values taken from stack
    kRecordGetValue,   // <u16 valIndex>
    kRecordGetObject,  // <u16 objIndex>
    kValueListNew,     // <u16 numVals> pushes value list with items taken from stack
    kObjectListNew,    // <u16 numObjs> pushes object list with items taken from stack
};

}  // namespace vm
