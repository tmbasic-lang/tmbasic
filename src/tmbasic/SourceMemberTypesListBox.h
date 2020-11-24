#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

typedef std::function<void()> SourceMemberTypeSelectedFunc;

class SourceMemberTypesListBox : public TListViewer {
   public:
    SourceMemberTypesListBox(const TRect& bounds, uint16_t numCols, SourceMemberTypeSelectedFunc onSelectedFunc);
    virtual ~SourceMemberTypesListBox();
    void getText(char* dest, int16_t item, int16_t maxLen) override;
    void focusItem(int16_t item) override;
    TPalette& getPalette() const override;
    compiler::SourceMemberType getSelectedType() const;

   private:
    SourceMemberTypeSelectedFunc _onSelectedFunc;
    compiler::SourceMemberType _selectedType;
};

}  // namespace tmbasic
