/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gdk/gdk.h>                // for GdkEvent
#include <gtk/gtk.h>                // for GtkWidget, GtkToolItem, GtkMenuItem

#include "enums/ActionType.enum.h"  // for ActionType
#include "model/Font.h"             // for XojFont

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionHandler;


class FontButton: public AbstractToolItem {
public:
    FontButton(ActionHandler* handler, std::string id, ActionType type, std::string description,
               GtkWidget* menuitem = nullptr);
    ~FontButton() override;

public:
    void activated(GtkMenuItem* menuitem, GtkToolButton* toolbutton) override;
    void setFont(const XojFont& font);
    XojFont getFont() const;
    std::string getToolDisplayName() const override;
    void showFontDialog();

protected:
    GtkToolItem* createItem(bool horizontal) override;
    GtkToolItem* createTmpItem(bool horizontal) override;
    GtkToolItem* newItem() override;

    GtkWidget* getNewToolIcon() const override;

private:
    GtkWidget* fontButton = nullptr;
    std::string description;

    XojFont font;
};
