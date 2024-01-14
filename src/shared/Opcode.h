#pragma once

#include "../common.h"

namespace shared {

enum class Opcode {
    // data types:
    // i64 - 8b signed integer
    // u64 - 8b unsigned integer
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
    kPushImmediateInt64,   // <i64 operand> pushv operand;
    kPushImmediateDec128,  // <u8 tag, u8 sign, u64 hi, u64 lo, i64 exp> pushv operand;
    kPushImmediateUtf8,    // <u32 length, utf8 string> pusho string
    kPopValue,             // popv;
    kPopObject,            // popo;
    kDuplicateValue,       //
    kDuplicateValues,      // <u8 count>
    kDuplicateObject,      //
    kDuplicateObjects,     // <u8 count>
    kSwapValues,           // swap top two values
    kSwapObjects,          // swap top two objects
    kCopyValue,            // <u8 abs(index)> push a value from the stack
    kCopyObject,           // <u8 abs(index)> push an object from the stack

    // variables
    kInitLocals,          // <u16 numVals, u16 numObjs>
    kPushArgumentValue,   // <u8 index> pushv index;
    kPushArgumentObject,  // <u8 index> pusho index;
    kSetArgumentValue,    // <u8 index>
    kSetArgumentObject,   // <u8 index>
    kPushGlobalValue,     // <globalVal* src> pushv *src
    kPushGlobalObject,    // <globalObj* src> pusho *src
    kSetGlobalValue,      // <globalVal* dst> *dst = vsp[-1]; popv;
    kSetGlobalObject,     // <globalObj* dst> *dst = ospTop; popo;
    kPushLocalValue,      // <localVal* src> pushv *src
    kPushLocalObject,     // <localObj* src> pusho *src
    kSetLocalValue,       // <localVal* dst> *dst = vsp[-1]; popv;
    kSetLocalObject,      // <localObj* dst> *dst = ospTop; popo;
    kClearLocalObject,    // <localObj* dst> *dst = null;

    // flow control
    kJump,           // <inst* index> goto byte index in the current procedure
    kBranchIfTrue,   // <inst* index> if (vsp[-1]) {goto index in current proc} popv;
    kBranchIfFalse,  // <inst* index> if (!vsp[-1]) {goto index in current proc} popv;

    // procedures
    kCall,          // <proc* procIndex, u8 numVals, u8 numObjs>
    kCallV,         // <proc* procIndex, u8 numVals, u8 numObjs>
    kCallO,         // <proc* procIndex, u8 numVals, u8 numObjs>
    kSystemCall,    // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallV,   // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallO,   // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kSystemCallVO,  // <u16 syscallIndex, u8 numVals, u8 numObjs>
    kReturn,        // restores osp/vsp
    kReturnValue,   // restores osp/vsp and pushes the return value vsp[-1]
    kReturnObject,  // restores osp/vsp and pushes the return object ospTop

    // error handling
    kSetError,       // set error flag from code vsp[-1] and message osp[-1]; popv; popo;
    kClearError,     // clear error flag, don't clear message or code
    kBubbleError,    // set error flag
    kReturnIfError,  // if error is set, return
    kBranchIfError,  // <inst* index> if error, then jump

    // records
    kRecordNew,        // <u16 numVals, u16 numObjs> pushes record with values taken from stack
    kRecordGetValue,   // <u16 valIndex>
    kRecordGetObject,  // <u16 objIndex>
    kRecordSetValue,   // <u16 valIndex> record and new value are on the stack
    kRecordSetObject,  // <u16 objIndex> record and new object are on the stack

    // lists
    kValueListNew,   // <u16 numVals> pushes value list with items taken from stack
    kObjectListNew,  // <u16 numObjs> pushes object list with items taken from stack

    // maps
    kObjectToObjectMapTryGet,  // in: map obj, key obj. out: result obj, success bool.
    kObjectToValueMapTryGet,   // in: map obj, key obj. out: result val, success bool.
    kValueToObjectMapTryGet,   // in: map obj, key val. out: result obj, success bool.
    kValueToValueMapTryGet,    // in: map obj, key val. out: result val, success bool.
    kSetErrorMapKeyNotFound,   // SetError with preset code and message

    // A variable-length representation of the dotted expression follows the fixed bits specified next to the opcode
    // below. The calling code pushes the value/object to be assigned, then the base of the target dotted expression.
    // Any index/key objects and values are pushed next, as described below. From left-to-right:
    // - for a dotted name:
    //      u8: 0x01 for value field, 0x02 for object field
    //      u16: field index
    // - for a list index / map key that is a value:
    //      u8: 0x03 for value element, 0x04 for object element
    //      index/key is on the value stack -- the count of these is in numKeyValuesOnStack
    // - for a map key that is an object:
    //      u8: 0x05 for value element, 0x06 for object element
    //      key is on the object stack -- the count of these is in numKeyObjectsOnStack
    // The updated base object is pushed on the stack.
    kDottedExpressionSetValue,   // <u8 numSuffixes, u8 numKeyValuesOnStack, u8 numKeyObjectsOnStack, ...>
    kDottedExpressionSetObject,  // <u8 numSuffixes, u8 numKeyValuesOnStack, u8 numKeyObjectsOnStack, ...>

    // minimal support for callbacks
    kSetEventHandler,  // <4b procedureIndex, 2b event> in: target obj

    // Not a real opcode. Used to get the number of opcodes.
    kMaxOpcode,
};

}  // namespace shared
