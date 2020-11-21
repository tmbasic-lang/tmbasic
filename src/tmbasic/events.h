#pragma once

#include "../common.h"
#include "tmbasic/EditorWindow.h"
#include "tmbasic/SourceProgram.h"

namespace tmbasic {

enum : uint16_t {
    // menu commands
    kCmdHelpBasicReference = 100,
    kCmdHelpDocumentation,
    kCmdHelpAbout,
    kCmdProgramContentsWindow,
    kCmdProgramAddSubroutine,
    kCmdProgramAddFunction,
    kCmdProgramAddGlobalVariable,
    kCmdProgramAddConstant,
    kCmdProgramAddType,
    kCmdProgramRun,

    // broadcast in response to cmSave/cmSaveAs
    kCmdProgramSave,
    kCmdProgramSaveAs,

    // broadcast when the app is exiting. bool* infoPtr = whether to cancel
    kCmdAppExit,

    // broadcast to find a ProgramWindow. ProgramWindow** infoPtr
    kCmdFindProgramWindow,

    // broadcast to find an EditorWindow for the specified member. FindEditorWindowEventArgs* infoPtr
    kCmdFindEditorWindow,

    // broadcast to close all program-related windows.
    kCmdCloseProgramRelatedWindows,

    // broadcast every 100msec or so
    kCmdTimerTick,
};

struct FindEditorWindowEventArgs {
    SourceMember* member;  // input
    EditorWindow* window;  // output
};

}  // namespace tmbasic
