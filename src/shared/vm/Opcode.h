#pragma once

#include "../../common.h"

namespace vm {

enum class Opcode {
    kExit,  // return;

    kLoadConstantA,        // <8b operand> A = (int64)operand;
    kLoadConstantB,        // <8b operand> B = (int64)operand;
    kLoadConstantStringX,  // <4b length, string> X = new string
    kLoadConstantStringY,  // <4b length, string> Y = new string

    kStoreA,  // <2b> VSP[operand] = A;
    kStoreB,  // <2b> VSP[operand] = B;
    kStoreX,  // <2b> OSP[operand] = X;
    kStoreY,  // <2b> OSP[operand] = Y;
    kLoadA,   // <2b> A = VSP[operand];
    kLoadB,   // <2b> B = VSP[operand];
    kLoadX,   // <2b> X = OSP[operand];
    kLoadY,   // <2b> Y = OSP[operand];

    kPushA,        // --VSP; VSP[0] = A;
    kPushB,        // --VSP; VSP[0] = B;
    kPushX,        // --OSP; OSP[0] = X;
    kPushY,        // --OSP; OSP[0] = Y;
    kPopA,         // A = VSP[0]; ++VSP;
    kPopB,         // B = VSP[0]; ++VSP;
    kPopX,         // X = OSP[0]; ++OSP;
    kPopY,         // Y = OSP[0]; ++OSP;
    kPushValues,   // <2b> VSP -= operand
    kPushObjects,  // <2b> OSP -= operand
    kPopValues,    // <2b> VSP += operand
    kPopObjects,   // <2b> OSP += operand

    kClearX,  // X = NULL;
    kClearY,  // Y = NULL;

    kSetAFromB,  // A = B;
    kSetBFromA,  // B = A;
    kSetXFromY,  // X = Y;
    kSetYFromX,  // Y = X;

    kAOrB,                 // A = A || B;
    kAAndB,                // A = A && B;
    kAEqualsB,             // A = A == B;
    kANotEqualsB,          // A = A <> B
    kALessThanB,           // A = A < B
    kALessThanEqualsB,     // A = A <= B
    kAGreaterThanB,        // A = A > B
    kAGreaterThanEqualsB,  // A = A >= B
    kAAddB,                // A += B
    kASubtractB,           // A -= B
    kAMultiplyB,           // A *= B
    kADivideB,             // A /= B
    kAModuloB,             // A %= B
    kAEqualsConstant,      // <8b> A = A == (int64)operand;
    kBEqualsConstant,      // <8b> A = B == (int64)operand;
    kStringXEqualsY,       // A = strcmp(X, Y) == 0;
    kStringXConcatenateY,  // X = strcat(x, y)

    kJump,          // <4b> goto byte index in the current procedure
    kBranchIfA,     // <4b> if (A) goto byte index in the current procedure
    kBranchIfNotA,  // <4b> if (!A) goto byte index in the current procedure

    kCall,               // <2b: proc index>
    kReturn,             // restores OSP/VSP to where the caller left them, implicitly returns A and X
    kSetError,           // X=message, A=code, set error flag
    kClearError,         // clear error flag, don't clear message or code
    kBubbleError,        // set error flag
    kReturnIfError,      // if error is set, return
    kPopBranchIfError,   // <4b num vals, 4b num objs, 4b target> if error, then pop and jump
    kBranchIfNotError,   // <4b> if no error, then jump
    kLoadErrorMessageX,  // x = error string
    kLoadErrorCodeA,     // a = error code

    kRecordBuilderBegin,   // <2b: num vals; 2b: num objs> push a new buffer on the RB stack
    kRecordBuilderStoreA,  // <2b: value index> RB.Values[operand] = A
    kRecordBuilderStoreX,  // <2b: object index> RB.Objects[operand] = X
    kRecordBuilderEnd,     // X = new tuple created from RB popped from RB stack
    kRecordLoadA,          // <2b: index> A = value read from operand index in tuple X
    kRecordLoadX,          // <2b: index> X = object read from operand index in tuple X
    kRecordStoreA,         // <2b: index> X = copy of X with operand value index set to A
    kRecordStoreY,         // <2b: index> X = copy of X with operand object  index set to Y

    kValueListBuilderBegin,  // push new ValueListBuilder to VLB stack
    kValueListBuilderAddA,   // append A to topmost ValueListBuilder
    kValueListBuilderEnd,    // X = new ValueList, ValueListBuilder is popped
    kValueListGet,           // A = X[A]
    kValueListSet,           // X[A] = B
    kValueListCount,         // A = X.count
    kValueListInsert,        // insert A into index B in list X, store new list in X
    kValueListRemove,        // remove index A from list X

    kObjectListBuilderBegin,  // push new ObjectListBuilder to VLB stack
    kObjectListBuilderAddX,   // append X to topmost ObjectListBuilder
    kObjectListBuilderEnd,    // X = new ObjectList, ObjectListBuilder is popped
    kObjectListGet,           // X = X[A]
    kObjectListSet,           // X[A] = Y
    kObjectListCount,         // A = X.count
    kObjectListInsert,        // insert Y into index B in list X, store new list in X
    kObjectListRemove,        // remove index A from list X

    kValueToValueMapNew,     // X = new map
    kValueToValueMapTryGet,  // if X[A] exists, A=X[A] and B=1. if not, B=0
    kValueToValueMapCount,   // A = X.count
    kValueToValueMapSet,     // X = copy X and set A -> B
    kValueToValueMapRemove,  // X = copy X with key A removed
    kValueToValueMapKeys,    // X = ValueList formed from keys of map X
    kValueToValueMapValues,  // X = ValueList formed from values of map X

    kValueToObjectMapNew,     // X = new map
    kValueToObjectMapTryGet,  // if X[A] exists, X=X[A] and B=1. if not, B=0
    kValueToObjectMapCount,   // A = X.count
    kValueToObjectMapSet,     // X = copy X and set A -> Y
    kValueToObjectMapRemove,  // X = copy X with key A removed
    kValueToObjectMapKeys,    // X = ValueList formed from keys of map X
    kValueToObjectMapValues,  // X = ObjectList formed from values of map X

    kObjectToValueMapNew,     // X = new map
    kObjectToValueMapTryGet,  // if X[Y] exists, A=X[Y] and B=1. if not, B=0
    kObjectToValueMapCount,   // A = X.count
    kObjectToValueMapSet,     // X = copy X and set Y -> A
    kObjectToValueMapRemove,  // X = copy X with key Y removed
    kObjectToValueMapKeys,    // X = ObjectList formed from keys of map X
    kObjectToValueMapValues,  // X = ValueList formed from values of map X

    kObjectToObjectMapNew,     // X = new map
    kObjectToObjectMapTryGet,  // if X[Y] exists, X=X[Y] and B=1. if not, B=0
    kObjectToObjectMapCount,   // A = X.count
    kObjectToObjectMapSet,     // X = copy X and set Y -> Z
    kObjectToObjectMapRemove,  // X = copy X with key Y removed
    kObjectToObjectMapKeys,    // X = ObjectList formed from keys of map X
    kObjectToObjectMapValues,  // X = ObjectList formed from values of map X

    kStringMid,        // X=input, A=startIndex, B=length. store "" or substring in X
    kStringIndexOf,    // X=haystack, Y=needle, A=startIndex. store -1 or index in A
    kStringChr,        // A=ascii code, only LSB is used. store string in X
    kStringAsc,        // X=string, A=index. store -1 or X[A] into A
    kStringPrint,      // X=string (does not print newline)
    kStringInputLine,  // stores string in X
    kNumberToString,   // convert number in A to string in X
    kStringToNumber,   // convert string in X to number in A, with success (0/1) in B
};

}  // namespace vm
