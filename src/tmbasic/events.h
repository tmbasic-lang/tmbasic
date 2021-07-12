#pragma once

#include "../common.h"

namespace compiler {
class SourceMember;
}

namespace tmbasic {

enum : uint16_t {
    // top menu commands
    kCmdHelpBasicReference = 100,
    kCmdHelpDocumentation,
    kCmdHelpLicense,
    kCmdHelpAbout,
    kCmdEditRedo,
    kCmdEditInsertSymbol,
    kCmdEditInsertColor,
    kCmdProgramAddItem,
    kCmdProgramImportItem,
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
    kCmdProgramCheckForErrors,
    kCmdProgramPublish,
    kCmdDesignAddButton,
    kCmdDesignAddCheckBox,
    kCmdDesignAddGroupBox,
    kCmdDesignAddLabel,
    kCmdDesignAddListBox,
    kCmdDesignAddRadioButton,
    kCmdDesignAddScrollBar,
    kCmdDesignAddTextBox,
    kCmdDesignAddCustomControl,
    kCmdPictureCharacter,
    kCmdPictureFg,
    kCmdPictureBg,
    kCmdPictureSelect,
    kCmdPictureDraw,
    kCmdPicturePick,
    kCmdPictureType,
    kCmdPictureMask,
    kCmdPictureOptions,
    kCmdPicturePasteOk,
    kCmdPicturePasteCancel,
    kCmdPictureMove,
    kCmdPictureMoveOk,
    kCmdPictureMoveCancel,

    // designer window context menu
    kCmdDesignerWindowProperties,

    // broadcast in response to cmSave/cmSaveAs to tell windows to update their SourceMember records
    kCmdProgramSave,

    // broadcast when the app is exiting. bool* infoPtr = whether to cancel
    kCmdAppExit,

    // broadcast to find a ProgramWindow. ProgramWindow** infoPtr
    kCmdFindProgramWindow,

    // broadcast to find a CodeEditorWindow for the specified member. FindEditorWindowEventArgs* infoPtr
    kCmdFindEditorWindow,

    // broadcast to find a DesignerWindow for the specified member. FindDesignerWindowEventArgs* infoPtr
    kCmdFindDesignerWindow,

    // broadcast to find a PictureWindow for the specified member. FindPictureWindowEventArgs* infoPtr
    kCmdFindPictureWindow,

    // broadcast to close all program-related windows.
    kCmdCloseProgramRelatedWindows,

    // broadcast every 250msec or so
    kCmdTimerTick,

    // sent from PictureView to PictureWindow
    kCmdPictureViewMouse,
};

struct FindEditorWindowEventArgs {
    compiler::SourceMember* member;  // input
    class CodeEditorWindow* window;  // output
};

struct FindDesignerWindowEventArgs {
    compiler::SourceMember* member;  // input
    class DesignerWindow* window;    // output
};

struct FindPictureWindowEventArgs {
    compiler::SourceMember* member;  // input
    class PictureWindow* window;     // output
};

}  // namespace tmbasic
