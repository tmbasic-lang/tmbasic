#include "ui/App.h"

int main(int argc, char** argv) {
    ui::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
