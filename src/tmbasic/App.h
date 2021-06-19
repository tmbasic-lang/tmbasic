#pragma once

#include "../common.h"
#include "tmbasic/PictureWindow.h"
#include "tmbasic/ProgramWindow.h"

namespace tmbasic {

enum class EditorType { kFunction, kSubroutine, kGlobalVariable, kConstant, kType };

enum class DesignerType { kForm, kCustomControl };

class App : public TApplication {
   public:
    static std::array<char, 9> helpWindowPalette;
    static TStatusItem* insertColorDialogHelpStatusItem;

    App(int argc, char** argv);
    virtual ~App();
    void idle() override;
    void handleEvent(TEvent& event) override;
    TPalette& getPalette() const override;
    void setPictureWindow(PictureWindow* pictureWindow);
    void openHelpTopic(uint16_t topic);

   private:
    gsl::owner<class AppPrivate*> _private;
};

}  // namespace tmbasic
