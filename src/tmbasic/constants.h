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

// app
const ushort kCmdHelpBasicReference = 100;
const ushort kCmdHelpDocumentation = 101;
const ushort kCmdHelpAbout = 102;
const ushort kCmdProgramContentsWindow = 103;
const ushort kCmdProgramAddSubroutine = 104;
const ushort kCmdProgramAddFunction = 105;
const ushort kCmdProgramAddGlobalVariable = 106;
const ushort kCmdProgramRun = 107;

// program window
const ushort kCmdProgramItemTypeSelected = 200;

}  // namespace tmbasic
