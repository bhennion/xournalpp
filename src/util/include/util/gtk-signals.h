#pragma once

#include <gtk/gtk.h>

namespace xoj::util::signal {
template <>
struct Signals<GtkAboutDialog, 0> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkAboutDialog*, char* uri, gpointer user_data);
};

template <>
struct Signals<GtkAdjustment, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkAdjustment*, gpointer user_data);
};
template <>
struct Signals<GtkAdjustment, 1> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkAdjustment*, gpointer user_data);
};

template <>
struct Signals<GtkAppChooserButton, 0> {
    static constexpr const char* name() { return "custom-item-activated"; }
    using callback_type = void(GtkAppChooserButton*, char* item_name, gpointer user_data);
};

template <>
struct Signals<GtkAppChooserWidget, 0> {
    static constexpr const char* name() { return "application-activated"; }
    using callback_type = void(GtkAppChooserWidget*, GAppInfo* application, gpointer user_data);
};
template <>
struct Signals<GtkAppChooserWidget, 1> {
    static constexpr const char* name() { return "application-selected"; }
    using callback_type = void(GtkAppChooserWidget*, GAppInfo* application, gpointer user_data);
};
template <>
struct Signals<GtkAppChooserWidget, 2> {
    static constexpr const char* name() { return "populate-popup"; }
    using callback_type = void(GtkAppChooserWidget*, GtkMenu* menu, GAppInfo* application, gpointer user_data);
};

template <>
struct Signals<GtkApplication, 0> {
    static constexpr const char* name() { return "query-end"; }
    using callback_type = void(GtkApplication*, gpointer user_data);
};
template <>
struct Signals<GtkApplication, 1> {
    static constexpr const char* name() { return "window-added"; }
    using callback_type = void(GtkApplication*, GtkWindow* window, gpointer user_data);
};
template <>
struct Signals<GtkApplication, 2> {
    static constexpr const char* name() { return "window-removed"; }
    using callback_type = void(GtkApplication*, GtkWindow* window, gpointer user_data);
};

template <>
struct Signals<GtkAssistant, 0> {
    static constexpr const char* name() { return "apply"; }
    using callback_type = void(GtkAssistant*, gpointer user_data);
};
template <>
struct Signals<GtkAssistant, 1> {
    static constexpr const char* name() { return "cancel"; }
    using callback_type = void(GtkAssistant*, gpointer user_data);
};
template <>
struct Signals<GtkAssistant, 2> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkAssistant*, gpointer user_data);
};
template <>
struct Signals<GtkAssistant, 3> {
    static constexpr const char* name() { return "escape"; }
    using callback_type = void(GtkAssistant*, gpointer user_data);
};
template <>
struct Signals<GtkAssistant, 4> {
    static constexpr const char* name() { return "prepare"; }
    using callback_type = void(GtkAssistant*, GtkWidget* page, gpointer user_data);
};

template <>
struct Signals<GtkButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};
template <>
struct Signals<GtkButton, 1> {
    static constexpr const char* name() { return "clicked"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};
template <>
struct Signals<GtkButton, 2> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};
template <>
struct Signals<GtkButton, 3> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};
template <>
struct Signals<GtkButton, 4> {
    static constexpr const char* name() { return "pressed"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};
template <>
struct Signals<GtkButton, 5> {
    static constexpr const char* name() { return "released"; }
    using callback_type = void(GtkButton*, gpointer user_data);
};

template <>
struct Signals<GtkCalendar, 0> {
    static constexpr const char* name() { return "day-selected"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 1> {
    static constexpr const char* name() { return "day-selected-double-click"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 2> {
    static constexpr const char* name() { return "month-changed"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 3> {
    static constexpr const char* name() { return "next-month"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 4> {
    static constexpr const char* name() { return "next-year"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 5> {
    static constexpr const char* name() { return "prev-month"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};
template <>
struct Signals<GtkCalendar, 6> {
    static constexpr const char* name() { return "prev-year"; }
    using callback_type = void(GtkCalendar*, gpointer user_data);
};

template <>
struct Signals<GtkCellArea, 0> {
    static constexpr const char* name() { return "add-editable"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer* renderer, GtkCellEditable* editable,
                               GdkRectangle* cell_area, char* path, gpointer user_data);
};
template <>
struct Signals<GtkCellArea, 1> {
    static constexpr const char* name() { return "apply-attributes"; }
    using callback_type = void(GtkCellArea*, GtkTreeModel* model, GtkTreeIter* iter, gboolean is_expander,
                               gboolean is_expanded, gpointer user_data);
};
template <>
struct Signals<GtkCellArea, 2> {
    static constexpr const char* name() { return "focus-changed"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer* renderer, char* path, gpointer user_data);
};
template <>
struct Signals<GtkCellArea, 3> {
    static constexpr const char* name() { return "remove-editable"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer* renderer, GtkCellEditable* editable, gpointer user_data);
};

template <>
struct Signals<GtkCellEditable, 0> {
    static constexpr const char* name() { return "editing-done"; }
    using callback_type = void(GtkCellEditable*, gpointer user_data);
};
template <>
struct Signals<GtkCellEditable, 1> {
    static constexpr const char* name() { return "remove-widget"; }
    using callback_type = void(GtkCellEditable*, gpointer user_data);
};

template <>
struct Signals<GtkCellRenderer, 0> {
    static constexpr const char* name() { return "editing-canceled"; }
    using callback_type = void(GtkCellRenderer*, gpointer user_data);
};
template <>
struct Signals<GtkCellRenderer, 1> {
    static constexpr const char* name() { return "editing-started"; }
    using callback_type = void(GtkCellRenderer*, GtkCellEditable* editable, char* path, gpointer user_data);
};

template <>
struct Signals<GtkCellRendererAccel, 0> {
    static constexpr const char* name() { return "accel-cleared"; }
    using callback_type = void(GtkCellRendererAccel*, char* path_string, gpointer user_data);
};
template <>
struct Signals<GtkCellRendererAccel, 1> {
    static constexpr const char* name() { return "accel-edited"; }
    using callback_type = void(GtkCellRendererAccel*, char* path_string, guint accel_key, GdkModifierType accel_mods,
                               guint hardware_keycode, gpointer user_data);
};

template <>
struct Signals<GtkCellRendererCombo, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkCellRendererCombo*, char* path_string, GtkTreeIter* new_iter, gpointer user_data);
};

template <>
struct Signals<GtkCellRendererText, 0> {
    static constexpr const char* name() { return "edited"; }
    using callback_type = void(GtkCellRendererText*, char* path, char* new_text, gpointer user_data);
};

template <>
struct Signals<GtkCellRendererToggle, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkCellRendererToggle*, char* path, gpointer user_data);
};

template <>
struct Signals<GtkCheckMenuItem, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkCheckMenuItem*, gpointer user_data);
};

template <>
struct Signals<GtkColorButton, 0> {
    static constexpr const char* name() { return "color-set"; }
    using callback_type = void(GtkColorButton*, gpointer user_data);
};

template <>
struct Signals<GtkColorChooser, 0> {
    static constexpr const char* name() { return "color-activated"; }
    using callback_type = void(GtkColorChooser*, GdkRGBA* color, gpointer user_data);
};

template <>
struct Signals<GtkComboBox, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkComboBox*, gpointer user_data);
};
template <>
struct Signals<GtkComboBox, 1> {
    static constexpr const char* name() { return "move-active"; }
    using callback_type = void(GtkComboBox*, GtkScrollType scroll_type, gpointer user_data);
};
template <>
struct Signals<GtkComboBox, 2> {
    static constexpr const char* name() { return "popdown"; }
    using callback_type = gboolean(GtkComboBox*, gpointer user_data);
};
template <>
struct Signals<GtkComboBox, 3> {
    static constexpr const char* name() { return "popup"; }
    using callback_type = void(GtkComboBox*, gpointer user_data);
};

template <>
struct Signals<GtkContainer, 0> {
    static constexpr const char* name() { return "add"; }
    using callback_type = void(GtkContainer*, GtkWidget* widget, gpointer user_data);
};
template <>
struct Signals<GtkContainer, 1> {
    static constexpr const char* name() { return "check-resize"; }
    using callback_type = void(GtkContainer*, gpointer user_data);
};
template <>
struct Signals<GtkContainer, 2> {
    static constexpr const char* name() { return "remove"; }
    using callback_type = void(GtkContainer*, GtkWidget* widget, gpointer user_data);
};
template <>
struct Signals<GtkContainer, 3> {
    static constexpr const char* name() { return "set-focus-child"; }
    using callback_type = void(GtkContainer*, GtkWidget* widget, gpointer user_data);
};

template <>
struct Signals<GtkCssProvider, 0> {
    static constexpr const char* name() { return "parsing-error"; }
    using callback_type = void(GtkCssProvider*, GtkCssSection* section, GError* error, gpointer user_data);
};

template <>
struct Signals<GtkDialog, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkDialog*, gpointer user_data);
};
template <>
struct Signals<GtkDialog, 1> {
    static constexpr const char* name() { return "response"; }
    using callback_type = void(GtkDialog*, int response_id, gpointer user_data);
};

template <>
struct Signals<GtkEditable, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkEditable*, gpointer user_data);
};
template <>
struct Signals<GtkEditable, 1> {
    static constexpr const char* name() { return "delete-text"; }
    using callback_type = void(GtkEditable*, int start_pos, int end_pos, gpointer user_data);
};
template <>
struct Signals<GtkEditable, 2> {
    static constexpr const char* name() { return "insert-text"; }
    using callback_type = void(GtkEditable*, char* new_text, int new_text_length, gpointer position,
                               gpointer user_data);
};

template <>
struct Signals<GtkEntry, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 1> {
    static constexpr const char* name() { return "backspace"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 2> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 3> {
    static constexpr const char* name() { return "cut-clipboard"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 4> {
    static constexpr const char* name() { return "delete-from-cursor"; }
    using callback_type = void(GtkEntry*, GtkDeleteType type, int count, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 5> {
    static constexpr const char* name() { return "icon-press"; }
    using callback_type = void(GtkEntry*, GtkEntryIconPosition icon_pos, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 6> {
    static constexpr const char* name() { return "icon-release"; }
    using callback_type = void(GtkEntry*, GtkEntryIconPosition icon_pos, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 7> {
    static constexpr const char* name() { return "insert-at-cursor"; }
    using callback_type = void(GtkEntry*, char* string, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 8> {
    static constexpr const char* name() { return "insert-emoji"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 9> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkEntry*, GtkMovementStep step, int count, gboolean extend_selection,
                               gpointer user_data);
};
template <>
struct Signals<GtkEntry, 10> {
    static constexpr const char* name() { return "paste-clipboard"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 11> {
    static constexpr const char* name() { return "populate-popup"; }
    using callback_type = void(GtkEntry*, GtkWidget* widget, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 12> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkEntry*, char* preedit, gpointer user_data);
};
template <>
struct Signals<GtkEntry, 13> {
    static constexpr const char* name() { return "toggle-overwrite"; }
    using callback_type = void(GtkEntry*, gpointer user_data);
};

template <>
struct Signals<GtkEntryBuffer, 0> {
    static constexpr const char* name() { return "deleted-text"; }
    using callback_type = void(GtkEntryBuffer*, guint position, guint n_chars, gpointer user_data);
};
template <>
struct Signals<GtkEntryBuffer, 1> {
    static constexpr const char* name() { return "inserted-text"; }
    using callback_type = void(GtkEntryBuffer*, guint position, char* chars, guint n_chars, gpointer user_data);
};

template <>
struct Signals<GtkEntryCompletion, 0> {
    static constexpr const char* name() { return "action-activated"; }
    using callback_type = void(GtkEntryCompletion*, int index, gpointer user_data);
};
template <>
struct Signals<GtkEntryCompletion, 1> {
    static constexpr const char* name() { return "cursor-on-match"; }
    using callback_type = gboolean(GtkEntryCompletion*, GtkTreeModel* model, GtkTreeIter* iter, gpointer user_data);
};
template <>
struct Signals<GtkEntryCompletion, 2> {
    static constexpr const char* name() { return "insert-prefix"; }
    using callback_type = gboolean(GtkEntryCompletion*, char* prefix, gpointer user_data);
};
template <>
struct Signals<GtkEntryCompletion, 3> {
    static constexpr const char* name() { return "match-selected"; }
    using callback_type = gboolean(GtkEntryCompletion*, GtkTreeModel* model, GtkTreeIter* iter, gpointer user_data);
};
template <>
struct Signals<GtkEntryCompletion, 4> {
    static constexpr const char* name() { return "no-matches"; }
    using callback_type = void(GtkEntryCompletion*, gpointer user_data);
};

template <>
struct Signals<GtkEventControllerKey, 0> {
    static constexpr const char* name() { return "focus-in"; }
    using callback_type = void(GtkEventControllerKey*, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerKey, 1> {
    static constexpr const char* name() { return "focus-out"; }
    using callback_type = void(GtkEventControllerKey*, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerKey, 2> {
    static constexpr const char* name() { return "im-update"; }
    using callback_type = void(GtkEventControllerKey*, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerKey, 3> {
    static constexpr const char* name() { return "key-pressed"; }
    using callback_type = gboolean(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state,
                                   gpointer user_data);
};
template <>
struct Signals<GtkEventControllerKey, 4> {
    static constexpr const char* name() { return "key-released"; }
    using callback_type = void(GtkEventControllerKey*, guint keyval, guint keycode, GdkModifierType state,
                               gpointer user_data);
};
template <>
struct Signals<GtkEventControllerKey, 5> {
    static constexpr const char* name() { return "modifiers"; }
    using callback_type = gboolean(GtkEventControllerKey*, GdkModifierType arg1, gpointer user_data);
};

template <>
struct Signals<GtkEventControllerMotion, 0> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = void(GtkEventControllerMotion*, double x, double y, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerMotion, 1> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkEventControllerMotion*, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerMotion, 2> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = void(GtkEventControllerMotion*, double x, double y, gpointer user_data);
};

template <>
struct Signals<GtkEventControllerScroll, 0> {
    static constexpr const char* name() { return "decelerate"; }
    using callback_type = void(GtkEventControllerScroll*, double vel_x, double vel_y, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerScroll, 1> {
    static constexpr const char* name() { return "scroll"; }
    using callback_type = void(GtkEventControllerScroll*, double dx, double dy, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerScroll, 2> {
    static constexpr const char* name() { return "scroll-begin"; }
    using callback_type = void(GtkEventControllerScroll*, gpointer user_data);
};
template <>
struct Signals<GtkEventControllerScroll, 3> {
    static constexpr const char* name() { return "scroll-end"; }
    using callback_type = void(GtkEventControllerScroll*, gpointer user_data);
};

template <>
struct Signals<GtkExpander, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkExpander*, gpointer user_data);
};

template <>
struct Signals<GtkFileChooser, 0> {
    static constexpr const char* name() { return "current-folder-changed"; }
    using callback_type = void(GtkFileChooser*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooser, 1> {
    static constexpr const char* name() { return "file-activated"; }
    using callback_type = void(GtkFileChooser*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooser, 2> {
    static constexpr const char* name() { return "selection-changed"; }
    using callback_type = void(GtkFileChooser*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooser, 3> {
    static constexpr const char* name() { return "update-preview"; }
    using callback_type = void(GtkFileChooser*, gpointer user_data);
};

template <>
struct Signals<GtkFileChooserButton, 0> {
    static constexpr const char* name() { return "file-set"; }
    using callback_type = void(GtkFileChooserButton*, gpointer user_data);
};

template <>
struct Signals<GtkFileChooserWidget, 0> {
    static constexpr const char* name() { return "desktop-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 1> {
    static constexpr const char* name() { return "down-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 2> {
    static constexpr const char* name() { return "home-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 3> {
    static constexpr const char* name() { return "location-popup"; }
    using callback_type = void(GtkFileChooserWidget*, char* path, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 4> {
    static constexpr const char* name() { return "location-popup-on-paste"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 5> {
    static constexpr const char* name() { return "location-toggle-popup"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 6> {
    static constexpr const char* name() { return "places-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 7> {
    static constexpr const char* name() { return "quick-bookmark"; }
    using callback_type = void(GtkFileChooserWidget*, int bookmark_index, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 8> {
    static constexpr const char* name() { return "recent-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 9> {
    static constexpr const char* name() { return "search-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 10> {
    static constexpr const char* name() { return "show-hidden"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};
template <>
struct Signals<GtkFileChooserWidget, 11> {
    static constexpr const char* name() { return "up-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer user_data);
};

template <>
struct Signals<GtkFlowBox, 0> {
    static constexpr const char* name() { return "activate-cursor-child"; }
    using callback_type = void(GtkFlowBox*, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 1> {
    static constexpr const char* name() { return "child-activated"; }
    using callback_type = void(GtkFlowBox*, GtkFlowBoxChild* child, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 2> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkFlowBox*, GtkMovementStep step, int count, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 3> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkFlowBox*, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 4> {
    static constexpr const char* name() { return "selected-children-changed"; }
    using callback_type = void(GtkFlowBox*, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 5> {
    static constexpr const char* name() { return "toggle-cursor-child"; }
    using callback_type = void(GtkFlowBox*, gpointer user_data);
};
template <>
struct Signals<GtkFlowBox, 6> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkFlowBox*, gpointer user_data);
};

template <>
struct Signals<GtkFlowBoxChild, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkFlowBoxChild*, gpointer user_data);
};

template <>
struct Signals<GtkFontButton, 0> {
    static constexpr const char* name() { return "font-set"; }
    using callback_type = void(GtkFontButton*, gpointer user_data);
};

template <>
struct Signals<GtkFontChooser, 0> {
    static constexpr const char* name() { return "font-activated"; }
    using callback_type = void(GtkFontChooser*, char* fontname, gpointer user_data);
};

template <>
struct Signals<GtkGLArea, 0> {
    static constexpr const char* name() { return "render"; }
    using callback_type = gboolean(GtkGLArea*, GdkGLContext* context, gpointer user_data);
};
template <>
struct Signals<GtkGLArea, 1> {
    static constexpr const char* name() { return "resize"; }
    using callback_type = void(GtkGLArea*, int width, int height, gpointer user_data);
};

template <>
struct Signals<GtkGesture, 0> {
    static constexpr const char* name() { return "begin"; }
    using callback_type = void(GtkGesture*, GdkEventSequence* sequence, gpointer user_data);
};
template <>
struct Signals<GtkGesture, 1> {
    static constexpr const char* name() { return "cancel"; }
    using callback_type = void(GtkGesture*, GdkEventSequence* sequence, gpointer user_data);
};
template <>
struct Signals<GtkGesture, 2> {
    static constexpr const char* name() { return "end"; }
    using callback_type = void(GtkGesture*, GdkEventSequence* sequence, gpointer user_data);
};
template <>
struct Signals<GtkGesture, 3> {
    static constexpr const char* name() { return "sequence-state-changed"; }
    using callback_type = void(GtkGesture*, GdkEventSequence* sequence, GtkEventSequenceState state,
                               gpointer user_data);
};
template <>
struct Signals<GtkGesture, 4> {
    static constexpr const char* name() { return "update"; }
    using callback_type = void(GtkGesture*, GdkEventSequence* sequence, gpointer user_data);
};

template <>
struct Signals<GtkGestureDrag, 0> {
    static constexpr const char* name() { return "drag-begin"; }
    using callback_type = void(GtkGestureDrag*, double start_x, double start_y, gpointer user_data);
};
template <>
struct Signals<GtkGestureDrag, 1> {
    static constexpr const char* name() { return "drag-end"; }
    using callback_type = void(GtkGestureDrag*, double offset_x, double offset_y, gpointer user_data);
};
template <>
struct Signals<GtkGestureDrag, 2> {
    static constexpr const char* name() { return "drag-update"; }
    using callback_type = void(GtkGestureDrag*, double offset_x, double offset_y, gpointer user_data);
};

template <>
struct Signals<GtkGestureLongPress, 0> {
    static constexpr const char* name() { return "cancelled"; }
    using callback_type = void(GtkGestureLongPress*, gpointer user_data);
};
template <>
struct Signals<GtkGestureLongPress, 1> {
    static constexpr const char* name() { return "pressed"; }
    using callback_type = void(GtkGestureLongPress*, double x, double y, gpointer user_data);
};

template <>
struct Signals<GtkGestureMultiPress, 0> {
    static constexpr const char* name() { return "pressed"; }
    using callback_type = void(GtkGestureMultiPress*, int n_press, double x, double y, gpointer user_data);
};
template <>
struct Signals<GtkGestureMultiPress, 1> {
    static constexpr const char* name() { return "released"; }
    using callback_type = void(GtkGestureMultiPress*, int n_press, double x, double y, gpointer user_data);
};
template <>
struct Signals<GtkGestureMultiPress, 2> {
    static constexpr const char* name() { return "stopped"; }
    using callback_type = void(GtkGestureMultiPress*, gpointer user_data);
};

template <>
struct Signals<GtkGesturePan, 0> {
    static constexpr const char* name() { return "pan"; }
    using callback_type = void(GtkGesturePan*, GtkPanDirection direction, double offset, gpointer user_data);
};

template <>
struct Signals<GtkGestureRotate, 0> {
    static constexpr const char* name() { return "angle-changed"; }
    using callback_type = void(GtkGestureRotate*, double angle, double angle_delta, gpointer user_data);
};

template <>
struct Signals<GtkGestureStylus, 0> {
    static constexpr const char* name() { return "down"; }
    using callback_type = void(GtkGestureStylus*, double arg1, double arg2, gpointer user_data);
};
template <>
struct Signals<GtkGestureStylus, 1> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = void(GtkGestureStylus*, double arg1, double arg2, gpointer user_data);
};
template <>
struct Signals<GtkGestureStylus, 2> {
    static constexpr const char* name() { return "proximity"; }
    using callback_type = void(GtkGestureStylus*, double arg1, double arg2, gpointer user_data);
};
template <>
struct Signals<GtkGestureStylus, 3> {
    static constexpr const char* name() { return "up"; }
    using callback_type = void(GtkGestureStylus*, double arg1, double arg2, gpointer user_data);
};

template <>
struct Signals<GtkGestureSwipe, 0> {
    static constexpr const char* name() { return "swipe"; }
    using callback_type = void(GtkGestureSwipe*, double velocity_x, double velocity_y, gpointer user_data);
};

template <>
struct Signals<GtkGestureZoom, 0> {
    static constexpr const char* name() { return "scale-changed"; }
    using callback_type = void(GtkGestureZoom*, double scale, gpointer user_data);
};

template <>
struct Signals<GtkIMContext, 0> {
    static constexpr const char* name() { return "commit"; }
    using callback_type = void(GtkIMContext*, char* str, gpointer user_data);
};
template <>
struct Signals<GtkIMContext, 1> {
    static constexpr const char* name() { return "delete-surrounding"; }
    using callback_type = gboolean(GtkIMContext*, int offset, int n_chars, gpointer user_data);
};
template <>
struct Signals<GtkIMContext, 2> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkIMContext*, gpointer user_data);
};
template <>
struct Signals<GtkIMContext, 3> {
    static constexpr const char* name() { return "preedit-end"; }
    using callback_type = void(GtkIMContext*, gpointer user_data);
};
template <>
struct Signals<GtkIMContext, 4> {
    static constexpr const char* name() { return "preedit-start"; }
    using callback_type = void(GtkIMContext*, gpointer user_data);
};
template <>
struct Signals<GtkIMContext, 5> {
    static constexpr const char* name() { return "retrieve-surrounding"; }
    using callback_type = gboolean(GtkIMContext*, gpointer user_data);
};

template <>
struct Signals<GtkIconTheme, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkIconTheme*, gpointer user_data);
};

template <>
struct Signals<GtkIconView, 0> {
    static constexpr const char* name() { return "activate-cursor-item"; }
    using callback_type = gboolean(GtkIconView*, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 1> {
    static constexpr const char* name() { return "item-activated"; }
    using callback_type = void(GtkIconView*, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 2> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkIconView*, GtkMovementStep step, int count, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 3> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkIconView*, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 4> {
    static constexpr const char* name() { return "select-cursor-item"; }
    using callback_type = void(GtkIconView*, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 5> {
    static constexpr const char* name() { return "selection-changed"; }
    using callback_type = void(GtkIconView*, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 6> {
    static constexpr const char* name() { return "toggle-cursor-item"; }
    using callback_type = void(GtkIconView*, gpointer user_data);
};
template <>
struct Signals<GtkIconView, 7> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkIconView*, gpointer user_data);
};

template <>
struct Signals<GtkInfoBar, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkInfoBar*, gpointer user_data);
};
template <>
struct Signals<GtkInfoBar, 1> {
    static constexpr const char* name() { return "response"; }
    using callback_type = void(GtkInfoBar*, int response_id, gpointer user_data);
};

template <>
struct Signals<GtkLabel, 0> {
    static constexpr const char* name() { return "activate-current-link"; }
    using callback_type = void(GtkLabel*, gpointer user_data);
};
template <>
struct Signals<GtkLabel, 1> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkLabel*, char* uri, gpointer user_data);
};
template <>
struct Signals<GtkLabel, 2> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkLabel*, gpointer user_data);
};
template <>
struct Signals<GtkLabel, 3> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkLabel*, GtkMovementStep step, int count, gboolean extend_selection,
                               gpointer user_data);
};
template <>
struct Signals<GtkLabel, 4> {
    static constexpr const char* name() { return "populate-popup"; }
    using callback_type = void(GtkLabel*, GtkMenu* menu, gpointer user_data);
};

template <>
struct Signals<GtkLevelBar, 0> {
    static constexpr const char* name() { return "offset-changed"; }
    using callback_type = void(GtkLevelBar*, char* name, gpointer user_data);
};

template <>
struct Signals<GtkLinkButton, 0> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkLinkButton*, gpointer user_data);
};

template <>
struct Signals<GtkListBox, 0> {
    static constexpr const char* name() { return "activate-cursor-row"; }
    using callback_type = void(GtkListBox*, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 1> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkListBox*, GtkMovementStep arg1, int arg2, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 2> {
    static constexpr const char* name() { return "row-activated"; }
    using callback_type = void(GtkListBox*, GtkListBoxRow* row, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 3> {
    static constexpr const char* name() { return "row-selected"; }
    using callback_type = void(GtkListBox*, GtkListBoxRow* row, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 4> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkListBox*, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 5> {
    static constexpr const char* name() { return "selected-rows-changed"; }
    using callback_type = void(GtkListBox*, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 6> {
    static constexpr const char* name() { return "toggle-cursor-row"; }
    using callback_type = void(GtkListBox*, gpointer user_data);
};
template <>
struct Signals<GtkListBox, 7> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkListBox*, gpointer user_data);
};

template <>
struct Signals<GtkListBoxRow, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkListBoxRow*, gpointer user_data);
};

template <>
struct Signals<GtkMenu, 0> {
    static constexpr const char* name() { return "move-scroll"; }
    using callback_type = void(GtkMenu*, GtkScrollType scroll_type, gpointer user_data);
};
template <>
struct Signals<GtkMenu, 1> {
    static constexpr const char* name() { return "popped-up"; }
    using callback_type = void(GtkMenu*, gpointer flipped_rect, gpointer final_rect, gboolean flipped_x,
                               gboolean flipped_y, gpointer user_data);
};

template <>
struct Signals<GtkMenuItem, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkMenuItem*, gpointer user_data);
};
template <>
struct Signals<GtkMenuItem, 1> {
    static constexpr const char* name() { return "activate-item"; }
    using callback_type = void(GtkMenuItem*, gpointer user_data);
};
template <>
struct Signals<GtkMenuItem, 2> {
    static constexpr const char* name() { return "deselect"; }
    using callback_type = void(GtkMenuItem*, gpointer user_data);
};
template <>
struct Signals<GtkMenuItem, 3> {
    static constexpr const char* name() { return "select"; }
    using callback_type = void(GtkMenuItem*, gpointer user_data);
};
template <>
struct Signals<GtkMenuItem, 4> {
    static constexpr const char* name() { return "toggle-size-allocate"; }
    using callback_type = void(GtkMenuItem*, int arg1, gpointer user_data);
};
template <>
struct Signals<GtkMenuItem, 5> {
    static constexpr const char* name() { return "toggle-size-request"; }
    using callback_type = void(GtkMenuItem*, gpointer arg1, gpointer user_data);
};

template <>
struct Signals<GtkMenuShell, 0> {
    static constexpr const char* name() { return "activate-current"; }
    using callback_type = void(GtkMenuShell*, gboolean force_hide, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 1> {
    static constexpr const char* name() { return "cancel"; }
    using callback_type = void(GtkMenuShell*, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 2> {
    static constexpr const char* name() { return "cycle-focus"; }
    using callback_type = void(GtkMenuShell*, GtkDirectionType direction, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 3> {
    static constexpr const char* name() { return "deactivate"; }
    using callback_type = void(GtkMenuShell*, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 4> {
    static constexpr const char* name() { return "insert"; }
    using callback_type = void(GtkMenuShell*, GtkWidget* child, int position, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 5> {
    static constexpr const char* name() { return "move-current"; }
    using callback_type = void(GtkMenuShell*, GtkMenuDirectionType direction, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 6> {
    static constexpr const char* name() { return "move-selected"; }
    using callback_type = gboolean(GtkMenuShell*, int distance, gpointer user_data);
};
template <>
struct Signals<GtkMenuShell, 7> {
    static constexpr const char* name() { return "selection-done"; }
    using callback_type = void(GtkMenuShell*, gpointer user_data);
};

template <>
struct Signals<GtkMenuToolButton, 0> {
    static constexpr const char* name() { return "show-menu"; }
    using callback_type = void(GtkMenuToolButton*, gpointer user_data);
};

template <>
struct Signals<GtkNotebook, 0> {
    static constexpr const char* name() { return "change-current-page"; }
    using callback_type = gboolean(GtkNotebook*, int arg1, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 1> {
    static constexpr const char* name() { return "focus-tab"; }
    using callback_type = gboolean(GtkNotebook*, GtkNotebookTab arg1, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 2> {
    static constexpr const char* name() { return "move-focus-out"; }
    using callback_type = void(GtkNotebook*, GtkDirectionType arg1, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 3> {
    static constexpr const char* name() { return "page-added"; }
    using callback_type = void(GtkNotebook*, GtkWidget* child, guint page_num, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 4> {
    static constexpr const char* name() { return "page-removed"; }
    using callback_type = void(GtkNotebook*, GtkWidget* child, guint page_num, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 5> {
    static constexpr const char* name() { return "page-reordered"; }
    using callback_type = void(GtkNotebook*, GtkWidget* child, guint page_num, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 6> {
    static constexpr const char* name() { return "reorder-tab"; }
    using callback_type = gboolean(GtkNotebook*, GtkDirectionType arg1, gboolean arg2, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 7> {
    static constexpr const char* name() { return "select-page"; }
    using callback_type = gboolean(GtkNotebook*, gboolean arg1, gpointer user_data);
};
template <>
struct Signals<GtkNotebook, 8> {
    static constexpr const char* name() { return "switch-page"; }
    using callback_type = void(GtkNotebook*, GtkWidget* page, guint page_num, gpointer user_data);
};

template <>
struct Signals<GtkOverlay, 0> {
    static constexpr const char* name() { return "get-child-position"; }
    using callback_type = gboolean(GtkOverlay*, GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);
};

template <>
struct Signals<GtkPaned, 0> {
    static constexpr const char* name() { return "accept-position"; }
    using callback_type = gboolean(GtkPaned*, gpointer user_data);
};
template <>
struct Signals<GtkPaned, 1> {
    static constexpr const char* name() { return "cancel-position"; }
    using callback_type = gboolean(GtkPaned*, gpointer user_data);
};
template <>
struct Signals<GtkPaned, 2> {
    static constexpr const char* name() { return "cycle-child-focus"; }
    using callback_type = gboolean(GtkPaned*, gboolean reversed, gpointer user_data);
};
template <>
struct Signals<GtkPaned, 3> {
    static constexpr const char* name() { return "cycle-handle-focus"; }
    using callback_type = gboolean(GtkPaned*, gboolean reversed, gpointer user_data);
};
template <>
struct Signals<GtkPaned, 4> {
    static constexpr const char* name() { return "move-handle"; }
    using callback_type = gboolean(GtkPaned*, GtkScrollType scroll_type, gpointer user_data);
};
template <>
struct Signals<GtkPaned, 5> {
    static constexpr const char* name() { return "toggle-handle-focus"; }
    using callback_type = gboolean(GtkPaned*, gpointer user_data);
};

template <>
struct Signals<GtkPlacesSidebar, 0> {
    static constexpr const char* name() { return "drag-action-ask"; }
    using callback_type = int(GtkPlacesSidebar*, int actions, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 1> {
    static constexpr const char* name() { return "drag-action-requested"; }
    using callback_type = int(GtkPlacesSidebar*, GdkDragContext* context, GObject* dest_file, gpointer source_file_list,
                              gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 2> {
    static constexpr const char* name() { return "drag-perform-drop"; }
    using callback_type = void(GtkPlacesSidebar*, GObject* dest_file, gpointer source_file_list, int action,
                               gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 3> {
    static constexpr const char* name() { return "mount"; }
    using callback_type = void(GtkPlacesSidebar*, GMountOperation* mount_operation, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 4> {
    static constexpr const char* name() { return "open-location"; }
    using callback_type = void(GtkPlacesSidebar*, GObject* location, GtkPlacesOpenFlags open_flags, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 5> {
    static constexpr const char* name() { return "populate-popup"; }
    using callback_type = void(GtkPlacesSidebar*, GtkWidget* container, GFile* selected_item, GVolume* selected_volume,
                               gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 6> {
    static constexpr const char* name() { return "show-connect-to-server"; }
    using callback_type = void(GtkPlacesSidebar*, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 7> {
    static constexpr const char* name() { return "show-enter-location"; }
    using callback_type = void(GtkPlacesSidebar*, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 8> {
    static constexpr const char* name() { return "show-error-message"; }
    using callback_type = void(GtkPlacesSidebar*, char* primary, char* secondary, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 9> {
    static constexpr const char* name() { return "show-other-locations"; }
    using callback_type = void(GtkPlacesSidebar*, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 10> {
    static constexpr const char* name() { return "show-other-locations-with-flags"; }
    using callback_type = void(GtkPlacesSidebar*, GtkPlacesOpenFlags open_flags, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 11> {
    static constexpr const char* name() { return "show-starred-location"; }
    using callback_type = void(GtkPlacesSidebar*, GtkPlacesOpenFlags open_flags, gpointer user_data);
};
template <>
struct Signals<GtkPlacesSidebar, 12> {
    static constexpr const char* name() { return "unmount"; }
    using callback_type = void(GtkPlacesSidebar*, GMountOperation* mount_operation, gpointer user_data);
};

template <>
struct Signals<GtkPopover, 0> {
    static constexpr const char* name() { return "closed"; }
    using callback_type = void(GtkPopover*, gpointer user_data);
};

template <>
struct Signals<GtkRadioButton, 0> {
    static constexpr const char* name() { return "group-changed"; }
    using callback_type = void(GtkRadioButton*, gpointer user_data);
};

template <>
struct Signals<GtkRadioMenuItem, 0> {
    static constexpr const char* name() { return "group-changed"; }
    using callback_type = void(GtkRadioMenuItem*, gpointer user_data);
};

template <>
struct Signals<GtkRange, 0> {
    static constexpr const char* name() { return "adjust-bounds"; }
    using callback_type = void(GtkRange*, double value, gpointer user_data);
};
template <>
struct Signals<GtkRange, 1> {
    static constexpr const char* name() { return "change-value"; }
    using callback_type = gboolean(GtkRange*, GtkScrollType scroll, double value, gpointer user_data);
};
template <>
struct Signals<GtkRange, 2> {
    static constexpr const char* name() { return "move-slider"; }
    using callback_type = void(GtkRange*, GtkScrollType step, gpointer user_data);
};
template <>
struct Signals<GtkRange, 3> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkRange*, gpointer user_data);
};

template <>
struct Signals<GtkRecentChooser, 0> {
    static constexpr const char* name() { return "item-activated"; }
    using callback_type = void(GtkRecentChooser*, gpointer user_data);
};
template <>
struct Signals<GtkRecentChooser, 1> {
    static constexpr const char* name() { return "selection-changed"; }
    using callback_type = void(GtkRecentChooser*, gpointer user_data);
};

template <>
struct Signals<GtkRecentManager, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkRecentManager*, gpointer user_data);
};

template <>
struct Signals<GtkScaleButton, 0> {
    static constexpr const char* name() { return "popdown"; }
    using callback_type = void(GtkScaleButton*, gpointer user_data);
};
template <>
struct Signals<GtkScaleButton, 1> {
    static constexpr const char* name() { return "popup"; }
    using callback_type = void(GtkScaleButton*, gpointer user_data);
};
template <>
struct Signals<GtkScaleButton, 2> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkScaleButton*, double value, gpointer user_data);
};

template <>
struct Signals<GtkScrolledWindow, 0> {
    static constexpr const char* name() { return "edge-overshot"; }
    using callback_type = void(GtkScrolledWindow*, GtkPositionType pos, gpointer user_data);
};
template <>
struct Signals<GtkScrolledWindow, 1> {
    static constexpr const char* name() { return "edge-reached"; }
    using callback_type = void(GtkScrolledWindow*, GtkPositionType pos, gpointer user_data);
};
template <>
struct Signals<GtkScrolledWindow, 2> {
    static constexpr const char* name() { return "move-focus-out"; }
    using callback_type = void(GtkScrolledWindow*, GtkDirectionType direction_type, gpointer user_data);
};
template <>
struct Signals<GtkScrolledWindow, 3> {
    static constexpr const char* name() { return "scroll-child"; }
    using callback_type = gboolean(GtkScrolledWindow*, GtkScrollType scroll, gboolean horizontal, gpointer user_data);
};

template <>
struct Signals<GtkSearchEntry, 0> {
    static constexpr const char* name() { return "next-match"; }
    using callback_type = void(GtkSearchEntry*, gpointer user_data);
};
template <>
struct Signals<GtkSearchEntry, 1> {
    static constexpr const char* name() { return "previous-match"; }
    using callback_type = void(GtkSearchEntry*, gpointer user_data);
};
template <>
struct Signals<GtkSearchEntry, 2> {
    static constexpr const char* name() { return "search-changed"; }
    using callback_type = void(GtkSearchEntry*, gpointer user_data);
};
template <>
struct Signals<GtkSearchEntry, 3> {
    static constexpr const char* name() { return "stop-search"; }
    using callback_type = void(GtkSearchEntry*, gpointer user_data);
};

template <>
struct Signals<GtkShortcutsSection, 0> {
    static constexpr const char* name() { return "change-current-page"; }
    using callback_type = gboolean(GtkShortcutsSection*, int arg1, gpointer user_data);
};

template <>
struct Signals<GtkShortcutsWindow, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkShortcutsWindow*, gpointer user_data);
};
template <>
struct Signals<GtkShortcutsWindow, 1> {
    static constexpr const char* name() { return "search"; }
    using callback_type = void(GtkShortcutsWindow*, gpointer user_data);
};


template <>
struct Signals<GtkSpinButton, 0> {
    static constexpr const char* name() { return "change-value"; }
    using callback_type = void(GtkSpinButton*, GtkScrollType scroll, gpointer user_data);
};
template <>
struct Signals<GtkSpinButton, 1> {
    static constexpr const char* name() { return "input"; }
    using callback_type = int(GtkSpinButton*, gpointer new_value, gpointer user_data);
};
template <>
struct Signals<GtkSpinButton, 2> {
    static constexpr const char* name() { return "output"; }
    using callback_type = gboolean(GtkSpinButton*, gpointer user_data);
};
template <>
struct Signals<GtkSpinButton, 3> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkSpinButton*, gpointer user_data);
};
template <>
struct Signals<GtkSpinButton, 4> {
    static constexpr const char* name() { return "wrapped"; }
    using callback_type = void(GtkSpinButton*, gpointer user_data);
};

template <>
struct Signals<GtkStatusbar, 0> {
    static constexpr const char* name() { return "text-popped"; }
    using callback_type = void(GtkStatusbar*, guint context_id, char* text, gpointer user_data);
};
template <>
struct Signals<GtkStatusbar, 1> {
    static constexpr const char* name() { return "text-pushed"; }
    using callback_type = void(GtkStatusbar*, guint context_id, char* text, gpointer user_data);
};

template <>
struct Signals<GtkStyle, 0> {
    static constexpr const char* name() { return "realize"; }
    using callback_type = void(GtkStyle*, gpointer user_data);
};
template <>
struct Signals<GtkStyle, 1> {
    static constexpr const char* name() { return "unrealize"; }
    using callback_type = void(GtkStyle*, gpointer user_data);
};

template <>
struct Signals<GtkStyleContext, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkStyleContext*, gpointer user_data);
};

template <>
struct Signals<GtkSwitch, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkSwitch*, gpointer user_data);
};
template <>
struct Signals<GtkSwitch, 1> {
    static constexpr const char* name() { return "state-set"; }
    using callback_type = gboolean(GtkSwitch*, gboolean state, gpointer user_data);
};

template <>
struct Signals<GtkTextBuffer, 0> {
    static constexpr const char* name() { return "apply-tag"; }
    using callback_type = void(GtkTextBuffer*, GtkTextTag* tag, GtkTextIter* start, GtkTextIter* end,
                               gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 1> {
    static constexpr const char* name() { return "begin-user-action"; }
    using callback_type = void(GtkTextBuffer*, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 2> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkTextBuffer*, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 3> {
    static constexpr const char* name() { return "delete-range"; }
    using callback_type = void(GtkTextBuffer*, GtkTextIter* start, GtkTextIter* end, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 4> {
    static constexpr const char* name() { return "end-user-action"; }
    using callback_type = void(GtkTextBuffer*, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 5> {
    static constexpr const char* name() { return "insert-child-anchor"; }
    using callback_type = void(GtkTextBuffer*, GtkTextIter* location, GtkTextChildAnchor* anchor, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 6> {
    static constexpr const char* name() { return "insert-pixbuf"; }
    using callback_type = void(GtkTextBuffer*, GtkTextIter* location, GdkPixbuf* pixbuf, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 7> {
    static constexpr const char* name() { return "insert-text"; }
    using callback_type = void(GtkTextBuffer*, GtkTextIter* location, char* text, int len, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 8> {
    static constexpr const char* name() { return "mark-deleted"; }
    using callback_type = void(GtkTextBuffer*, GtkTextMark* mark, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 9> {
    static constexpr const char* name() { return "mark-set"; }
    using callback_type = void(GtkTextBuffer*, GtkTextIter* location, GtkTextMark* mark, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 10> {
    static constexpr const char* name() { return "modified-changed"; }
    using callback_type = void(GtkTextBuffer*, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 11> {
    static constexpr const char* name() { return "paste-done"; }
    using callback_type = void(GtkTextBuffer*, GtkClipboard* clipboard, gpointer user_data);
};
template <>
struct Signals<GtkTextBuffer, 12> {
    static constexpr const char* name() { return "remove-tag"; }
    using callback_type = void(GtkTextBuffer*, GtkTextTag* tag, GtkTextIter* start, GtkTextIter* end,
                               gpointer user_data);
};

template <>
struct Signals<GtkTextTag, 0> {
    static constexpr const char* name() { return "event"; }
    using callback_type = gboolean(GtkTextTag*, GObject* object, GdkEvent* event, GtkTextIter* iter,
                                   gpointer user_data);
};

template <>
struct Signals<GtkTextTagTable, 0> {
    static constexpr const char* name() { return "tag-added"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag* tag, gpointer user_data);
};
template <>
struct Signals<GtkTextTagTable, 1> {
    static constexpr const char* name() { return "tag-changed"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag* tag, gboolean size_changed, gpointer user_data);
};
template <>
struct Signals<GtkTextTagTable, 2> {
    static constexpr const char* name() { return "tag-removed"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag* tag, gpointer user_data);
};

template <>
struct Signals<GtkTextView, 0> {
    static constexpr const char* name() { return "backspace"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 1> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 2> {
    static constexpr const char* name() { return "cut-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 3> {
    static constexpr const char* name() { return "delete-from-cursor"; }
    using callback_type = void(GtkTextView*, GtkDeleteType type, int count, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 4> {
    static constexpr const char* name() { return "extend-selection"; }
    using callback_type = gboolean(GtkTextView*, GtkTextExtendSelection granularity, GtkTextIter* location,
                                   GtkTextIter* start, GtkTextIter* end, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 5> {
    static constexpr const char* name() { return "insert-at-cursor"; }
    using callback_type = void(GtkTextView*, char* string, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 6> {
    static constexpr const char* name() { return "insert-emoji"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 7> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkTextView*, GtkMovementStep step, int count, gboolean extend_selection,
                               gpointer user_data);
};
template <>
struct Signals<GtkTextView, 8> {
    static constexpr const char* name() { return "move-viewport"; }
    using callback_type = void(GtkTextView*, GtkScrollStep step, int count, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 9> {
    static constexpr const char* name() { return "paste-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 10> {
    static constexpr const char* name() { return "populate-popup"; }
    using callback_type = void(GtkTextView*, GtkWidget* popup, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 11> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkTextView*, char* preedit, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 12> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkTextView*, gboolean select, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 13> {
    static constexpr const char* name() { return "set-anchor"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 14> {
    static constexpr const char* name() { return "toggle-cursor-visible"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};
template <>
struct Signals<GtkTextView, 15> {
    static constexpr const char* name() { return "toggle-overwrite"; }
    using callback_type = void(GtkTextView*, gpointer user_data);
};

template <>
struct Signals<GtkToggleButton, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkToggleButton*, gpointer user_data);
};

template <>
struct Signals<GtkToggleToolButton, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkToggleToolButton*, gpointer user_data);
};

template <>
struct Signals<GtkToolButton, 0> {
    static constexpr const char* name() { return "clicked"; }
    using callback_type = void(GtkToolButton*, gpointer user_data);
};

template <>
struct Signals<GtkToolItem, 0> {
    static constexpr const char* name() { return "create-menu-proxy"; }
    using callback_type = gboolean(GtkToolItem*, gpointer user_data);
};
template <>
struct Signals<GtkToolItem, 1> {
    static constexpr const char* name() { return "toolbar-reconfigured"; }
    using callback_type = void(GtkToolItem*, gpointer user_data);
};


template <>
struct Signals<GtkToolbar, 0> {
    static constexpr const char* name() { return "focus-home-or-end"; }
    using callback_type = gboolean(GtkToolbar*, gboolean focus_home, gpointer user_data);
};
template <>
struct Signals<GtkToolbar, 1> {
    static constexpr const char* name() { return "orientation-changed"; }
    using callback_type = void(GtkToolbar*, GtkOrientation orientation, gpointer user_data);
};
template <>
struct Signals<GtkToolbar, 2> {
    static constexpr const char* name() { return "popup-context-menu"; }
    using callback_type = gboolean(GtkToolbar*, int x, int y, int button, gpointer user_data);
};
template <>
struct Signals<GtkToolbar, 3> {
    static constexpr const char* name() { return "style-changed"; }
    using callback_type = void(GtkToolbar*, GtkToolbarStyle style, gpointer user_data);
};

template <>
struct Signals<GtkTreeModel, 0> {
    static constexpr const char* name() { return "row-changed"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath* path, GtkTreeIter* iter, gpointer user_data);
};
template <>
struct Signals<GtkTreeModel, 1> {
    static constexpr const char* name() { return "row-deleted"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkTreeModel, 2> {
    static constexpr const char* name() { return "row-has-child-toggled"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath* path, GtkTreeIter* iter, gpointer user_data);
};
template <>
struct Signals<GtkTreeModel, 3> {
    static constexpr const char* name() { return "row-inserted"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath* path, GtkTreeIter* iter, gpointer user_data);
};
template <>
struct Signals<GtkTreeModel, 4> {
    static constexpr const char* name() { return "rows-reordered"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath* path, GtkTreeIter* iter, gpointer new_order,
                               gpointer user_data);
};

template <>
struct Signals<GtkTreeSelection, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkTreeSelection*, gpointer user_data);
};

template <>
struct Signals<GtkTreeSortable, 0> {
    static constexpr const char* name() { return "sort-column-changed"; }
    using callback_type = void(GtkTreeSortable*, gpointer user_data);
};

template <>
struct Signals<GtkTreeView, 0> {
    static constexpr const char* name() { return "columns-changed"; }
    using callback_type = void(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 1> {
    static constexpr const char* name() { return "cursor-changed"; }
    using callback_type = void(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 2> {
    static constexpr const char* name() { return "expand-collapse-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gboolean arg1, gboolean arg2, gboolean arg3, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 3> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkTreeView*, GtkMovementStep step, int direction, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 4> {
    static constexpr const char* name() { return "row-activated"; }
    using callback_type = void(GtkTreeView*, GtkTreePath* path, GtkTreeViewColumn* column, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 5> {
    static constexpr const char* name() { return "row-collapsed"; }
    using callback_type = void(GtkTreeView*, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 6> {
    static constexpr const char* name() { return "row-expanded"; }
    using callback_type = void(GtkTreeView*, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 7> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = gboolean(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 8> {
    static constexpr const char* name() { return "select-cursor-parent"; }
    using callback_type = gboolean(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 9> {
    static constexpr const char* name() { return "select-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gboolean arg1, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 10> {
    static constexpr const char* name() { return "start-interactive-search"; }
    using callback_type = gboolean(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 11> {
    static constexpr const char* name() { return "test-collapse-row"; }
    using callback_type = gboolean(GtkTreeView*, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 12> {
    static constexpr const char* name() { return "test-expand-row"; }
    using callback_type = gboolean(GtkTreeView*, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 13> {
    static constexpr const char* name() { return "toggle-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gpointer user_data);
};
template <>
struct Signals<GtkTreeView, 14> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = gboolean(GtkTreeView*, gpointer user_data);
};

template <>
struct Signals<GtkTreeViewColumn, 0> {
    static constexpr const char* name() { return "clicked"; }
    using callback_type = void(GtkTreeViewColumn*, gpointer user_data);
};

template <>
struct Signals<GtkWidget, 0> {
    static constexpr const char* name() { return "accel-closures-changed"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 1> {
    static constexpr const char* name() { return "button-press-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 2> {
    static constexpr const char* name() { return "button-release-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 3> {
    static constexpr const char* name() { return "can-activate-accel"; }
    using callback_type = gboolean(GtkWidget*, guint signal_id, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 4> {
    static constexpr const char* name() { return "child-notify"; }
    using callback_type = void(GtkWidget*, GParamSpec* child_property, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 5> {
    static constexpr const char* name() { return "composited-changed"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 6> {
    static constexpr const char* name() { return "configure-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 7> {
    static constexpr const char* name() { return "damage-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 8> {
    static constexpr const char* name() { return "delete-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 9> {
    static constexpr const char* name() { return "destroy"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 10> {
    static constexpr const char* name() { return "destroy-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 11> {
    static constexpr const char* name() { return "direction-changed"; }
    using callback_type = void(GtkWidget*, GtkTextDirection previous_direction, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 12> {
    static constexpr const char* name() { return "drag-begin"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 13> {
    static constexpr const char* name() { return "drag-data-delete"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 14> {
    static constexpr const char* name() { return "drag-data-get"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, GtkSelectionData* data, guint info, guint time,
                               gpointer user_data);
};
template <>
struct Signals<GtkWidget, 15> {
    static constexpr const char* name() { return "drag-data-received"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, int x, int y, GtkSelectionData* data, guint info,
                               guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 16> {
    static constexpr const char* name() { return "drag-drop"; }
    using callback_type = gboolean(GtkWidget*, GdkDragContext* context, int x, int y, guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 17> {
    static constexpr const char* name() { return "drag-end"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 18> {
    static constexpr const char* name() { return "drag-failed"; }
    using callback_type = gboolean(GtkWidget*, GdkDragContext* context, GtkDragResult result, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 19> {
    static constexpr const char* name() { return "drag-leave"; }
    using callback_type = void(GtkWidget*, GdkDragContext* context, guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 20> {
    static constexpr const char* name() { return "drag-motion"; }
    using callback_type = gboolean(GtkWidget*, GdkDragContext* context, int x, int y, guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 21> {
    static constexpr const char* name() { return "draw"; }
    using callback_type = gboolean(GtkWidget*, cairo_t* cr, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 22> {
    static constexpr const char* name() { return "enter-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 23> {
    static constexpr const char* name() { return "event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 24> {
    static constexpr const char* name() { return "event-after"; }
    using callback_type = void(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 25> {
    static constexpr const char* name() { return "focus"; }
    using callback_type = gboolean(GtkWidget*, GtkDirectionType direction, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 26> {
    static constexpr const char* name() { return "focus-in-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 27> {
    static constexpr const char* name() { return "focus-out-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 28> {
    static constexpr const char* name() { return "grab-broken-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 29> {
    static constexpr const char* name() { return "grab-focus"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 30> {
    static constexpr const char* name() { return "grab-notify"; }
    using callback_type = void(GtkWidget*, gboolean was_grabbed, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 31> {
    static constexpr const char* name() { return "hide"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 32> {
    static constexpr const char* name() { return "hierarchy-changed"; }
    using callback_type = void(GtkWidget*, GtkWidget* previous_toplevel, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 33> {
    static constexpr const char* name() { return "key-press-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 34> {
    static constexpr const char* name() { return "key-release-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 35> {
    static constexpr const char* name() { return "keynav-failed"; }
    using callback_type = gboolean(GtkWidget*, GtkDirectionType direction, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 36> {
    static constexpr const char* name() { return "leave-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 37> {
    static constexpr const char* name() { return "map"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 38> {
    static constexpr const char* name() { return "map-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 39> {
    static constexpr const char* name() { return "mnemonic-activate"; }
    using callback_type = gboolean(GtkWidget*, gboolean group_cycling, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 40> {
    static constexpr const char* name() { return "motion-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 41> {
    static constexpr const char* name() { return "move-focus"; }
    using callback_type = void(GtkWidget*, GtkDirectionType direction, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 42> {
    static constexpr const char* name() { return "parent-set"; }
    using callback_type = void(GtkWidget*, GtkWidget* old_parent, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 43> {
    static constexpr const char* name() { return "popup-menu"; }
    using callback_type = gboolean(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 44> {
    static constexpr const char* name() { return "property-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 45> {
    static constexpr const char* name() { return "proximity-in-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 46> {
    static constexpr const char* name() { return "proximity-out-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 47> {
    static constexpr const char* name() { return "query-tooltip"; }
    using callback_type = gboolean(GtkWidget*, int x, int y, gboolean keyboard_mode, GtkTooltip* tooltip,
                                   gpointer user_data);
};
template <>
struct Signals<GtkWidget, 48> {
    static constexpr const char* name() { return "realize"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 49> {
    static constexpr const char* name() { return "screen-changed"; }
    using callback_type = void(GtkWidget*, GdkScreen* previous_screen, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 50> {
    static constexpr const char* name() { return "scroll-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 51> {
    static constexpr const char* name() { return "selection-clear-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 52> {
    static constexpr const char* name() { return "selection-get"; }
    using callback_type = void(GtkWidget*, GtkSelectionData* data, guint info, guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 53> {
    static constexpr const char* name() { return "selection-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 54> {
    static constexpr const char* name() { return "selection-received"; }
    using callback_type = void(GtkWidget*, GtkSelectionData* data, guint time, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 55> {
    static constexpr const char* name() { return "selection-request-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 56> {
    static constexpr const char* name() { return "show"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 57> {
    static constexpr const char* name() { return "show-help"; }
    using callback_type = gboolean(GtkWidget*, GtkWidgetHelpType help_type, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 58> {
    static constexpr const char* name() { return "size-allocate"; }
    using callback_type = void(GtkWidget*, GdkRectangle* allocation, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 59> {
    static constexpr const char* name() { return "state-changed"; }
    using callback_type = void(GtkWidget*, GtkStateType state, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 60> {
    static constexpr const char* name() { return "state-flags-changed"; }
    using callback_type = void(GtkWidget*, GtkStateFlags flags, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 61> {
    static constexpr const char* name() { return "style-set"; }
    using callback_type = void(GtkWidget*, GtkStyle* previous_style, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 62> {
    static constexpr const char* name() { return "style-updated"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 63> {
    static constexpr const char* name() { return "touch-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* arg1, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 64> {
    static constexpr const char* name() { return "unmap"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 65> {
    static constexpr const char* name() { return "unmap-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 66> {
    static constexpr const char* name() { return "unrealize"; }
    using callback_type = void(GtkWidget*, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 67> {
    static constexpr const char* name() { return "visibility-notify-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};
template <>
struct Signals<GtkWidget, 68> {
    static constexpr const char* name() { return "window-state-event"; }
    using callback_type = gboolean(GtkWidget*, GdkEvent* event, gpointer user_data);
};

template <>
struct Signals<GtkWindow, 0> {
    static constexpr const char* name() { return "activate-default"; }
    using callback_type = void(GtkWindow*, gpointer user_data);
};
template <>
struct Signals<GtkWindow, 1> {
    static constexpr const char* name() { return "activate-focus"; }
    using callback_type = void(GtkWindow*, gpointer user_data);
};
template <>
struct Signals<GtkWindow, 2> {
    static constexpr const char* name() { return "enable-debugging"; }
    using callback_type = gboolean(GtkWindow*, gboolean toggle, gpointer user_data);
};
template <>
struct Signals<GtkWindow, 3> {
    static constexpr const char* name() { return "keys-changed"; }
    using callback_type = void(GtkWindow*, gpointer user_data);
};
template <>
struct Signals<GtkWindow, 4> {
    static constexpr const char* name() { return "set-focus"; }
    using callback_type = void(GtkWindow*, GtkWidget* widget, gpointer user_data);
};
}  // namespace xoj::util::signal
