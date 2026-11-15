#include "Control.h"

Control::Control(GApplication* gtkApp, GladeSearchpath* gladeSearchPath):
        MainControl(gtkApp, gladeSearchPath), DocumentControl(this) {
    documentControl = this;
}

Control::~Control() = default;
