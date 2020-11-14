#pragma once

#include "common.h"

namespace tmbasic {

#define kWindowPaletteFramePassive "\x01"
#define kWindowPaletteFrameActive "\x02"
#define kWindowPaletteFrameIcon "\x03"
#define kWindowPaletteScrollBarPage "\x04"
#define kWindowPaletteScrollBarControls "\x05"
#define kWindowPaletteScrollerNormalText "\x06"
#define kWindowPaletteScrollerSelectedText "\x07"
#define kWindowPaletteReserved "\x08"

enum : ushort {
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

    // broadcast to close all program-related windows.
    kCmdCloseProgramRelatedWindows,
};

}  // namespace tmbasic
