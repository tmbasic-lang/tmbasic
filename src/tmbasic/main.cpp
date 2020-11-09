#include "App.h"

int main(int argc, char** argv) {
    tmbasic::App app(argc, argv);
    app.run();
    app.shutDown();
    return 0;
}
