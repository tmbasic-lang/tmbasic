#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class DesignerWindow;
class EditorWindow;
class PictureWindow;

enum : uint16_t {
    // top menu commands
    kCmdHelpBasicReference = 100,
    kCmdHelpDocumentation,
    kCmdHelpAbout,
    kCmdEditRedo,
    kCmdEditInsertSymbol,
    kCmdEditInsertColor,
    kCmdProgramContentsWindow,
    kCmdProgramAddSubroutine,
    kCmdProgramAddFunction,
    kCmdProgramAddGlobalVariable,
    kCmdProgramAddConstant,
    kCmdProgramAddType,
    kCmdProgramAddForm,
    kCmdProgramAddCustomControl,
    kCmdProgramAddPicture,
    kCmdProgramRun,
    kCmdDesignAddButton,
    kCmdDesignAddCheckBox,
    kCmdDesignAddGroupBox,
    kCmdDesignAddLabel,
    kCmdDesignAddListBox,
    kCmdDesignAddRadioButton,
    kCmdDesignAddScrollBar,
    kCmdDesignAddTextBox,
    kCmdDesignAddCustomControl,

    // designer window context menu
    kCmdDesignerWindowProperties,

    // broadcast in response to cmSave/cmSaveAs
    kCmdProgramSave,
    kCmdProgramSaveAs,

    // broadcast when the app is exiting. bool* infoPtr = whether to cancel
    kCmdAppExit,

    // broadcast to find a ProgramWindow. ProgramWindow** infoPtr
    kCmdFindProgramWindow,

    // broadcast to find an EditorWindow for the specified member. FindEditorWindowEventArgs* infoPtr
    kCmdFindEditorWindow,

    // broadcast to find a DesignerWindow for the specified member. FindDesignerWindowEventArgs* infoPtr
    kCmdFindDesignerWindow,

    // broadcast to find a PictureWindow for the specified member. FindPictureWindowEventArgs* infoPtr
    kCmdFindPictureWindow,

    // broadcast to close all program-related windows.
    kCmdCloseProgramRelatedWindows,

    // broadcast every 250msec or so
    kCmdTimerTick,
};

struct FindEditorWindowEventArgs {
    compiler::SourceMember* member;  // input
    EditorWindow* window;            // output
};

struct FindDesignerWindowEventArgs {
    compiler::SourceMember* member;  // input
    DesignerWindow* window;          // output
};

struct FindPictureWindowEventArgs {
    compiler::SourceMember* member;  // input
    PictureWindow* window;           // output
};

}  // namespace tmbasic
