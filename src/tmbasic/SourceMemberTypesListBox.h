#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"
#include "util/ListViewer.h"

namespace tmbasic {

typedef std::function<void()> SourceMemberTypeSelectedFunc;

class SourceMemberTypesListBox : public util::ListViewer {
   public:
    SourceMemberTypesListBox(const TRect& bounds, uint16_t numCols, SourceMemberTypeSelectedFunc onSelectedFunc);
    void getText(char* dest, int16_t item, int16_t maxLen) override;
    void focusItem(int16_t item) override;
    compiler::SourceMemberType getSelectedType() const;

   private:
    SourceMemberTypeSelectedFunc _onSelectedFunc;
    compiler::SourceMemberType _selectedType;
};

}  // namespace tmbasic
