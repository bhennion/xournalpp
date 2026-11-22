/*
 * Xournal++
 *
 * The old Control
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gio/gio.h>  // for GApplication

#include "enums/Action.enum.h"

#include "DocumentControl.h"
#include "MainControl.h"

class Settings;

class Control: public MainControl, public DocumentControl {
public:
    Control(GApplication* gtkApp, GladeSearchpath* gladeSearchPath);
    Control(Control const&) = delete;
    Control(Control&&) = delete;
    auto operator=(Control const&) -> Control& = delete;
    auto operator=(Control&&) -> Control& = delete;
    ~Control() override;

    inline auto* getSettings() const { return MainControl::getSettings(); }
    inline auto* getToolHandler() const { return MainControl::getToolHandler(); }

public:

    template <Action a>
    friend struct ActionProperties;
};
