#pragma once

#include <gtk/gtk.h>

namespace xoj::util::signal {

template<>
struct Signals<GtkAboutDialog, 0> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkAboutDialog*, gchar*  uri, gpointer  user_data);
};
template<>
struct Signals<GtkAdjustment, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkAdjustment*, gpointer  user_data);
};
template<>
struct Signals<GtkAdjustment, 1> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkAdjustment*, gpointer  user_data);
};
template<>
struct Signals<GtkAppChooserButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkAppChooserButton*, gpointer  user_data);
};
template<>
struct Signals<GtkAppChooserButton, 1> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkAppChooserButton*, gpointer  user_data);
};
template<>
struct Signals<GtkAppChooserButton, 2> {
    static constexpr const char* name() { return "custom-item-activated"; }
    using callback_type = void(GtkAppChooserButton*, gchar*  item_name, gpointer  user_data);
};
template<>
struct Signals<GtkAppChooserWidget, 0> {
    static constexpr const char* name() { return "application-activated"; }
    using callback_type = void(GtkAppChooserWidget*, GAppInfo*  application, gpointer  user_data);
};
template<>
struct Signals<GtkAppChooserWidget, 1> {
    static constexpr const char* name() { return "application-selected"; }
    using callback_type = void(GtkAppChooserWidget*, GAppInfo*  application, gpointer  user_data);
};
template<>
struct Signals<GtkApplication, 0> {
    static constexpr const char* name() { return "query-end"; }
    using callback_type = void(GtkApplication*, gpointer  user_data);
};
template<>
struct Signals<GtkApplication, 1> {
    static constexpr const char* name() { return "window-added"; }
    using callback_type = void(GtkApplication*, GtkWindow*  window, gpointer  user_data);
};
template<>
struct Signals<GtkApplication, 2> {
    static constexpr const char* name() { return "window-removed"; }
    using callback_type = void(GtkApplication*, GtkWindow*  window, gpointer  user_data);
};
template<>
struct Signals<GtkAssistant, 0> {
    static constexpr const char* name() { return "apply"; }
    using callback_type = void(GtkAssistant*, gpointer  user_data);
};
template<>
struct Signals<GtkAssistant, 1> {
    static constexpr const char* name() { return "cancel"; }
    using callback_type = void(GtkAssistant*, gpointer  user_data);
};
template<>
struct Signals<GtkAssistant, 2> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkAssistant*, gpointer  user_data);
};
template<>
struct Signals<GtkAssistant, 3> {
    static constexpr const char* name() { return "escape"; }
    using callback_type = void(GtkAssistant*, gpointer  user_data);
};
template<>
struct Signals<GtkAssistant, 4> {
    static constexpr const char* name() { return "prepare"; }
    using callback_type = void(GtkAssistant*, GtkWidget*  page, gpointer  user_data);
};
template<>
struct Signals<GtkATContext, 0> {
    static constexpr const char* name() { return "state-change"; }
    using callback_type = void(GtkATContext*, gpointer  user_data);
};
template<>
struct Signals<GtkButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkButton*, gpointer  user_data);
};
template<>
struct Signals<GtkButton, 1> {
    static constexpr const char* name() { return "clicked"; }
    using callback_type = void(GtkButton*, gpointer  user_data);
};
template<>
struct Signals<GtkCalendar, 0> {
    static constexpr const char* name() { return "day-selected"; }
    using callback_type = void(GtkCalendar*, gpointer  user_data);
};
template<>
struct Signals<GtkCalendar, 1> {
    static constexpr const char* name() { return "next-month"; }
    using callback_type = void(GtkCalendar*, gpointer  user_data);
};
template<>
struct Signals<GtkCalendar, 2> {
    static constexpr const char* name() { return "next-year"; }
    using callback_type = void(GtkCalendar*, gpointer  user_data);
};
template<>
struct Signals<GtkCalendar, 3> {
    static constexpr const char* name() { return "prev-month"; }
    using callback_type = void(GtkCalendar*, gpointer  user_data);
};
template<>
struct Signals<GtkCalendar, 4> {
    static constexpr const char* name() { return "prev-year"; }
    using callback_type = void(GtkCalendar*, gpointer  user_data);
};
template<>
struct Signals<GtkCellArea, 0> {
    static constexpr const char* name() { return "add-editable"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer*  renderer, GtkCellEditable*  editable, GdkRectangle*  cell_area, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkCellArea, 1> {
    static constexpr const char* name() { return "apply-attributes"; }
    using callback_type = void(GtkCellArea*, GtkTreeModel*  model, GtkTreeIter*  iter, gboolean  is_expander, gboolean  is_expanded, gpointer  user_data);
};
template<>
struct Signals<GtkCellArea, 2> {
    static constexpr const char* name() { return "focus-changed"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer*  renderer, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkCellArea, 3> {
    static constexpr const char* name() { return "remove-editable"; }
    using callback_type = void(GtkCellArea*, GtkCellRenderer*  renderer, GtkCellEditable*  editable, gpointer  user_data);
};
template<>
struct Signals<GtkCellEditable, 0> {
    static constexpr const char* name() { return "editing-done"; }
    using callback_type = void(GtkCellEditable*, gpointer  user_data);
};
template<>
struct Signals<GtkCellEditable, 1> {
    static constexpr const char* name() { return "remove-widget"; }
    using callback_type = void(GtkCellEditable*, gpointer  user_data);
};
template<>
struct Signals<GtkCellRenderer, 0> {
    static constexpr const char* name() { return "editing-canceled"; }
    using callback_type = void(GtkCellRenderer*, gpointer  user_data);
};
template<>
struct Signals<GtkCellRenderer, 1> {
    static constexpr const char* name() { return "editing-started"; }
    using callback_type = void(GtkCellRenderer*, GtkCellEditable*  editable, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkCellRendererAccel, 0> {
    static constexpr const char* name() { return "accel-cleared"; }
    using callback_type = void(GtkCellRendererAccel*, gchar*  path_string, gpointer  user_data);
};
template<>
struct Signals<GtkCellRendererAccel, 1> {
    static constexpr const char* name() { return "accel-edited"; }
    using callback_type = void(GtkCellRendererAccel*, gchar*  path_string, guint  accel_key, GdkModifierType  accel_mods, guint  hardware_keycode, gpointer  user_data);
};
template<>
struct Signals<GtkCellRendererCombo, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkCellRendererCombo*, gchar*  path_string, GtkTreeIter*  new_iter, gpointer  user_data);
};
template<>
struct Signals<GtkCellRendererText, 0> {
    static constexpr const char* name() { return "edited"; }
    using callback_type = void(GtkCellRendererText*, gchar*  path, gchar*  new_text, gpointer  user_data);
};
template<>
struct Signals<GtkCellRendererToggle, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkCellRendererToggle*, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkCheckButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkCheckButton*, gpointer  user_data);
};
template<>
struct Signals<GtkCheckButton, 1> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkCheckButton*, gpointer  user_data);
};
template<>
struct Signals<GtkColorButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkColorButton*, gpointer  user_data);
};
template<>
struct Signals<GtkColorButton, 1> {
    static constexpr const char* name() { return "color-set"; }
    using callback_type = void(GtkColorButton*, gpointer  user_data);
};
template<>
struct Signals<GtkColorChooser, 0> {
    static constexpr const char* name() { return "color-activated"; }
    using callback_type = void(GtkColorChooser*, GdkRGBA*  color, gpointer  user_data);
};
template<>
struct Signals<GtkColorDialogButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkColorDialogButton*, gpointer  user_data);
};
template<>
struct Signals<GtkColumnView, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkColumnView*, guint  position, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkComboBox*, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 1> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkComboBox*, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 2> {
    static constexpr const char* name() { return "format-entry-text"; }
    using callback_type = gchar*(GtkComboBox*, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 3> {
    static constexpr const char* name() { return "move-active"; }
    using callback_type = void(GtkComboBox*, GtkScrollType*  scroll_type, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 4> {
    static constexpr const char* name() { return "popdown"; }
    using callback_type = gboolean(GtkComboBox*, gpointer  user_data);
};
template<>
struct Signals<GtkComboBox, 5> {
    static constexpr const char* name() { return "popup"; }
    using callback_type = void(GtkComboBox*, gpointer  user_data);
};
template<>
struct Signals<GtkCssProvider, 0> {
    static constexpr const char* name() { return "parsing-error"; }
    using callback_type = void(GtkCssProvider*, GtkCssSection*  section, GError*  error, gpointer  user_data);
};
template<>
struct Signals<GtkDialog, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkDialog*, gpointer  user_data);
};
template<>
struct Signals<GtkDialog, 1> {
    static constexpr const char* name() { return "response"; }
    using callback_type = void(GtkDialog*, gint  response_id, gpointer  user_data);
};
template<>
struct Signals<GtkDragSource, 0> {
    static constexpr const char* name() { return "drag-begin"; }
    using callback_type = void(GtkDragSource*, GdkDrag*  drag, gpointer  user_data);
};
template<>
struct Signals<GtkDragSource, 1> {
    static constexpr const char* name() { return "drag-cancel"; }
    using callback_type = gboolean(GtkDragSource*, GdkDrag*  drag, GdkDragCancelReason*  reason, gpointer  user_data);
};
template<>
struct Signals<GtkDragSource, 2> {
    static constexpr const char* name() { return "drag-end"; }
    using callback_type = void(GtkDragSource*, GdkDrag*  drag, gboolean  delete_data, gpointer  user_data);
};
template<>
struct Signals<GtkDragSource, 3> {
    static constexpr const char* name() { return "prepare"; }
    using callback_type = GdkContentProvider*(GtkDragSource*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDrawingArea, 0> {
    static constexpr const char* name() { return "resize"; }
    using callback_type = void(GtkDrawingArea*, gint  width, gint  height, gpointer  user_data);
};
template<>
struct Signals<GtkDropControllerMotion, 0> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = void(GtkDropControllerMotion*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropControllerMotion, 1> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkDropControllerMotion*, gpointer  user_data);
};
template<>
struct Signals<GtkDropControllerMotion, 2> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = void(GtkDropControllerMotion*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropDown, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkDropDown*, gpointer  user_data);
};
template<>
struct Signals<GtkDropTarget, 0> {
    static constexpr const char* name() { return "accept"; }
    using callback_type = gboolean(GtkDropTarget*, GdkDrop*  drop, gpointer  user_data);
};
template<>
struct Signals<GtkDropTarget, 1> {
    static constexpr const char* name() { return "drop"; }
    using callback_type = gboolean(GtkDropTarget*,const GValue*  value, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropTarget, 2> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = GdkDragAction(GtkDropTarget*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropTarget, 3> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkDropTarget*, gpointer  user_data);
};
template<>
struct Signals<GtkDropTarget, 4> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = GdkDragAction(GtkDropTarget*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropTargetAsync, 0> {
    static constexpr const char* name() { return "accept"; }
    using callback_type = gboolean(GtkDropTargetAsync*, GdkDrop*  drop, gpointer  user_data);
};
template<>
struct Signals<GtkDropTargetAsync, 1> {
    static constexpr const char* name() { return "drag-enter"; }
    using callback_type = GdkDragAction(GtkDropTargetAsync*, GdkDrop*  drop, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropTargetAsync, 2> {
    static constexpr const char* name() { return "drag-leave"; }
    using callback_type = void(GtkDropTargetAsync*, GdkDrop*  drop, gpointer  user_data);
};
template<>
struct Signals<GtkDropTargetAsync, 3> {
    static constexpr const char* name() { return "drag-motion"; }
    using callback_type = GdkDragAction(GtkDropTargetAsync*, GdkDrop*  drop, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkDropTargetAsync, 4> {
    static constexpr const char* name() { return "drop"; }
    using callback_type = gboolean(GtkDropTargetAsync*, GdkDrop*  drop, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkEditable, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkEditable*, gpointer  user_data);
};
template<>
struct Signals<GtkEditable, 1> {
    static constexpr const char* name() { return "delete-text"; }
    using callback_type = void(GtkEditable*, gint  start_pos, gint  end_pos, gpointer  user_data);
};
template<>
struct Signals<GtkEditable, 2> {
    static constexpr const char* name() { return "insert-text"; }
    using callback_type = void(GtkEditable*, gchar*  text, gint  length, gint*  position, gpointer  user_data);
};
template<>
struct Signals<GtkEmojiChooser, 0> {
    static constexpr const char* name() { return "emoji-picked"; }
    using callback_type = void(GtkEmojiChooser*, gchar*  text, gpointer  user_data);
};
template<>
struct Signals<GtkEntry, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkEntry, 1> {
    static constexpr const char* name() { return "icon-press"; }
    using callback_type = void(GtkEntry*, GtkEntryIconPosition  icon_pos, gpointer  user_data);
};
template<>
struct Signals<GtkEntry, 2> {
    static constexpr const char* name() { return "icon-release"; }
    using callback_type = void(GtkEntry*, GtkEntryIconPosition  icon_pos, gpointer  user_data);
};
template<>
struct Signals<GtkEntryBuffer, 0> {
    static constexpr const char* name() { return "deleted-text"; }
    using callback_type = void(GtkEntryBuffer*, guint  position, guint  n_chars, gpointer  user_data);
};
template<>
struct Signals<GtkEntryBuffer, 1> {
    static constexpr const char* name() { return "inserted-text"; }
    using callback_type = void(GtkEntryBuffer*, guint  position, gchar*  chars, guint  n_chars, gpointer  user_data);
};
template<>
struct Signals<GtkEntryCompletion, 0> {
    static constexpr const char* name() { return "cursor-on-match"; }
    using callback_type = gboolean(GtkEntryCompletion*, GtkTreeModel*  model, GtkTreeIter*  iter, gpointer  user_data);
};
template<>
struct Signals<GtkEntryCompletion, 1> {
    static constexpr const char* name() { return "insert-prefix"; }
    using callback_type = gboolean(GtkEntryCompletion*, gchar*  prefix, gpointer  user_data);
};
template<>
struct Signals<GtkEntryCompletion, 2> {
    static constexpr const char* name() { return "match-selected"; }
    using callback_type = gboolean(GtkEntryCompletion*, GtkTreeModel*  model, GtkTreeIter*  iter, gpointer  user_data);
};
template<>
struct Signals<GtkEntryCompletion, 3> {
    static constexpr const char* name() { return "no-matches"; }
    using callback_type = void(GtkEntryCompletion*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerFocus, 0> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = void(GtkEventControllerFocus*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerFocus, 1> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkEventControllerFocus*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerKey, 0> {
    static constexpr const char* name() { return "im-update"; }
    using callback_type = void(GtkEventControllerKey*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerKey, 1> {
    static constexpr const char* name() { return "key-pressed"; }
    using callback_type = gboolean(GtkEventControllerKey*, guint  keyval, guint  keycode, GdkModifierType  state, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerKey, 2> {
    static constexpr const char* name() { return "key-released"; }
    using callback_type = void(GtkEventControllerKey*, guint  keyval, guint  keycode, GdkModifierType  state, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerKey, 3> {
    static constexpr const char* name() { return "modifiers"; }
    using callback_type = gboolean(GtkEventControllerKey*, GdkModifierType  state, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerLegacy, 0> {
    static constexpr const char* name() { return "event"; }
    using callback_type = gboolean(GtkEventControllerLegacy*, GdkEvent*  event, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerMotion, 0> {
    static constexpr const char* name() { return "enter"; }
    using callback_type = void(GtkEventControllerMotion*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerMotion, 1> {
    static constexpr const char* name() { return "leave"; }
    using callback_type = void(GtkEventControllerMotion*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerMotion, 2> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = void(GtkEventControllerMotion*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerScroll, 0> {
    static constexpr const char* name() { return "decelerate"; }
    using callback_type = void(GtkEventControllerScroll*, gdouble  vel_x, gdouble  vel_y, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerScroll, 1> {
    static constexpr const char* name() { return "scroll"; }
    using callback_type = gboolean(GtkEventControllerScroll*, gdouble  dx, gdouble  dy, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerScroll, 2> {
    static constexpr const char* name() { return "scroll-begin"; }
    using callback_type = void(GtkEventControllerScroll*, gpointer  user_data);
};
template<>
struct Signals<GtkEventControllerScroll, 3> {
    static constexpr const char* name() { return "scroll-end"; }
    using callback_type = void(GtkEventControllerScroll*, gpointer  user_data);
};
template<>
struct Signals<GtkExpander, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkExpander*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 0> {
    static constexpr const char* name() { return "desktop-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 1> {
    static constexpr const char* name() { return "down-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 2> {
    static constexpr const char* name() { return "home-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 3> {
    static constexpr const char* name() { return "location-popup"; }
    using callback_type = void(GtkFileChooserWidget*, gchar*  path, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 4> {
    static constexpr const char* name() { return "location-popup-on-paste"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 5> {
    static constexpr const char* name() { return "location-toggle-popup"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 6> {
    static constexpr const char* name() { return "places-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 7> {
    static constexpr const char* name() { return "quick-bookmark"; }
    using callback_type = void(GtkFileChooserWidget*, gint  bookmark_index, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 8> {
    static constexpr const char* name() { return "recent-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 9> {
    static constexpr const char* name() { return "search-shortcut"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 10> {
    static constexpr const char* name() { return "show-hidden"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFileChooserWidget, 11> {
    static constexpr const char* name() { return "up-folder"; }
    using callback_type = void(GtkFileChooserWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkFilter, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkFilter*, GtkFilterChange  change, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 0> {
    static constexpr const char* name() { return "activate-cursor-child"; }
    using callback_type = void(GtkFlowBox*, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 1> {
    static constexpr const char* name() { return "child-activated"; }
    using callback_type = void(GtkFlowBox*, GtkFlowBoxChild*  child, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 2> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkFlowBox*, GtkMovementStep*  step, gint  count, gboolean  extend, gboolean  modify, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 3> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkFlowBox*, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 4> {
    static constexpr const char* name() { return "selected-children-changed"; }
    using callback_type = void(GtkFlowBox*, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 5> {
    static constexpr const char* name() { return "toggle-cursor-child"; }
    using callback_type = void(GtkFlowBox*, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBox, 6> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkFlowBox*, gpointer  user_data);
};
template<>
struct Signals<GtkFlowBoxChild, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkFlowBoxChild*, gpointer  user_data);
};
template<>
struct Signals<GtkFontButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkFontButton*, gpointer  user_data);
};
template<>
struct Signals<GtkFontButton, 1> {
    static constexpr const char* name() { return "font-set"; }
    using callback_type = void(GtkFontButton*, gpointer  user_data);
};
template<>
struct Signals<GtkFontChooser, 0> {
    static constexpr const char* name() { return "font-activated"; }
    using callback_type = void(GtkFontChooser*, gchar*  fontname, gpointer  user_data);
};
template<>
struct Signals<GtkFontDialogButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkFontDialogButton*, gpointer  user_data);
};
template<>
struct Signals<GtkGesture, 0> {
    static constexpr const char* name() { return "begin"; }
    using callback_type = void(GtkGesture*, GdkEventSequence*  sequence, gpointer  user_data);
};
template<>
struct Signals<GtkGesture, 1> {
    static constexpr const char* name() { return "cancel"; }
    using callback_type = void(GtkGesture*, GdkEventSequence*  sequence, gpointer  user_data);
};
template<>
struct Signals<GtkGesture, 2> {
    static constexpr const char* name() { return "end"; }
    using callback_type = void(GtkGesture*, GdkEventSequence*  sequence, gpointer  user_data);
};
template<>
struct Signals<GtkGesture, 3> {
    static constexpr const char* name() { return "sequence-state-changed"; }
    using callback_type = void(GtkGesture*, GdkEventSequence*  sequence, GtkEventSequenceState  state, gpointer  user_data);
};
template<>
struct Signals<GtkGesture, 4> {
    static constexpr const char* name() { return "update"; }
    using callback_type = void(GtkGesture*, GdkEventSequence*  sequence, gpointer  user_data);
};
template<>
struct Signals<GtkGestureClick, 0> {
    static constexpr const char* name() { return "pressed"; }
    using callback_type = void(GtkGestureClick*, gint  n_press, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureClick, 1> {
    static constexpr const char* name() { return "released"; }
    using callback_type = void(GtkGestureClick*, gint  n_press, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureClick, 2> {
    static constexpr const char* name() { return "stopped"; }
    using callback_type = void(GtkGestureClick*, gpointer  user_data);
};
template<>
struct Signals<GtkGestureClick, 3> {
    static constexpr const char* name() { return "unpaired-release"; }
    using callback_type = void(GtkGestureClick*, gdouble  x, gdouble  y, guint  button, GdkEventSequence*  sequence, gpointer  user_data);
};
template<>
struct Signals<GtkGestureDrag, 0> {
    static constexpr const char* name() { return "drag-begin"; }
    using callback_type = void(GtkGestureDrag*, gdouble  start_x, gdouble  start_y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureDrag, 1> {
    static constexpr const char* name() { return "drag-end"; }
    using callback_type = void(GtkGestureDrag*, gdouble  offset_x, gdouble  offset_y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureDrag, 2> {
    static constexpr const char* name() { return "drag-update"; }
    using callback_type = void(GtkGestureDrag*, gdouble  offset_x, gdouble  offset_y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureLongPress, 0> {
    static constexpr const char* name() { return "cancelled"; }
    using callback_type = void(GtkGestureLongPress*, gpointer  user_data);
};
template<>
struct Signals<GtkGestureLongPress, 1> {
    static constexpr const char* name() { return "pressed"; }
    using callback_type = void(GtkGestureLongPress*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGesturePan, 0> {
    static constexpr const char* name() { return "pan"; }
    using callback_type = void(GtkGesturePan*, GtkPanDirection*  direction, gdouble  offset, gpointer  user_data);
};
template<>
struct Signals<GtkGestureRotate, 0> {
    static constexpr const char* name() { return "angle-changed"; }
    using callback_type = void(GtkGestureRotate*, gdouble  angle, gdouble  angle_delta, gpointer  user_data);
};
template<>
struct Signals<GtkGestureStylus, 0> {
    static constexpr const char* name() { return "down"; }
    using callback_type = void(GtkGestureStylus*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureStylus, 1> {
    static constexpr const char* name() { return "motion"; }
    using callback_type = void(GtkGestureStylus*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureStylus, 2> {
    static constexpr const char* name() { return "proximity"; }
    using callback_type = void(GtkGestureStylus*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureStylus, 3> {
    static constexpr const char* name() { return "up"; }
    using callback_type = void(GtkGestureStylus*, gdouble  x, gdouble  y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureSwipe, 0> {
    static constexpr const char* name() { return "swipe"; }
    using callback_type = void(GtkGestureSwipe*, gdouble  velocity_x, gdouble  velocity_y, gpointer  user_data);
};
template<>
struct Signals<GtkGestureZoom, 0> {
    static constexpr const char* name() { return "scale-changed"; }
    using callback_type = void(GtkGestureZoom*, gdouble  scale, gpointer  user_data);
};
template<>
struct Signals<GtkGLArea, 0> {
    static constexpr const char* name() { return "create-context"; }
    using callback_type = GdkGLContext*(GtkGLArea*, gpointer  user_data);
};
template<>
struct Signals<GtkGLArea, 1> {
    static constexpr const char* name() { return "render"; }
    using callback_type = gboolean(GtkGLArea*, GdkGLContext*  context, gpointer  user_data);
};
template<>
struct Signals<GtkGLArea, 2> {
    static constexpr const char* name() { return "resize"; }
    using callback_type = void(GtkGLArea*, gint  width, gint  height, gpointer  user_data);
};
template<>
struct Signals<GtkGridView, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkGridView*, guint  position, gpointer  user_data);
};
template<>
struct Signals<GtkIconTheme, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkIconTheme*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 0> {
    static constexpr const char* name() { return "activate-cursor-item"; }
    using callback_type = gboolean(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 1> {
    static constexpr const char* name() { return "item-activated"; }
    using callback_type = void(GtkIconView*, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 2> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkIconView*, GtkMovementStep*  step, gint  count, gboolean  extend, gboolean  modify, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 3> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 4> {
    static constexpr const char* name() { return "select-cursor-item"; }
    using callback_type = void(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 5> {
    static constexpr const char* name() { return "selection-changed"; }
    using callback_type = void(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 6> {
    static constexpr const char* name() { return "toggle-cursor-item"; }
    using callback_type = void(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIconView, 7> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkIconView*, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 0> {
    static constexpr const char* name() { return "commit"; }
    using callback_type = void(GtkIMContext*, gchar*  str, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 1> {
    static constexpr const char* name() { return "delete-surrounding"; }
    using callback_type = gboolean(GtkIMContext*, gint  offset, gint  n_chars, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 2> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkIMContext*, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 3> {
    static constexpr const char* name() { return "preedit-end"; }
    using callback_type = void(GtkIMContext*, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 4> {
    static constexpr const char* name() { return "preedit-start"; }
    using callback_type = void(GtkIMContext*, gpointer  user_data);
};
template<>
struct Signals<GtkIMContext, 5> {
    static constexpr const char* name() { return "retrieve-surrounding"; }
    using callback_type = gboolean(GtkIMContext*, gpointer  user_data);
};
template<>
struct Signals<GtkInfoBar, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkInfoBar*, gpointer  user_data);
};
template<>
struct Signals<GtkInfoBar, 1> {
    static constexpr const char* name() { return "response"; }
    using callback_type = void(GtkInfoBar*, gint  response_id, gpointer  user_data);
};
template<>
struct Signals<GtkLabel, 0> {
    static constexpr const char* name() { return "activate-current-link"; }
    using callback_type = void(GtkLabel*, gpointer  user_data);
};
template<>
struct Signals<GtkLabel, 1> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkLabel*, gchar*  uri, gpointer  user_data);
};
template<>
struct Signals<GtkLabel, 2> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkLabel*, gpointer  user_data);
};
template<>
struct Signals<GtkLabel, 3> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkLabel*, GtkMovementStep*  step, gint  count, gboolean  extend_selection, gpointer  user_data);
};
template<>
struct Signals<GtkLevelBar, 0> {
    static constexpr const char* name() { return "offset-changed"; }
    using callback_type = void(GtkLevelBar*, gchar*  name, gpointer  user_data);
};
template<>
struct Signals<GtkLinkButton, 0> {
    static constexpr const char* name() { return "activate-link"; }
    using callback_type = gboolean(GtkLinkButton*, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 0> {
    static constexpr const char* name() { return "activate-cursor-row"; }
    using callback_type = void(GtkListBox*, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 1> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkListBox*, GtkMovementStep*  step, gint  count, gboolean  extend, gboolean  modify, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 2> {
    static constexpr const char* name() { return "row-activated"; }
    using callback_type = void(GtkListBox*, GtkListBoxRow*  row, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 3> {
    static constexpr const char* name() { return "row-selected"; }
    using callback_type = void(GtkListBox*, GtkListBoxRow*  row, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 4> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkListBox*, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 5> {
    static constexpr const char* name() { return "selected-rows-changed"; }
    using callback_type = void(GtkListBox*, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 6> {
    static constexpr const char* name() { return "toggle-cursor-row"; }
    using callback_type = void(GtkListBox*, gpointer  user_data);
};
template<>
struct Signals<GtkListBox, 7> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = void(GtkListBox*, gpointer  user_data);
};
template<>
struct Signals<GtkListBoxRow, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkListBoxRow*, gpointer  user_data);
};
template<>
struct Signals<GtkListView, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkListView*, guint  position, gpointer  user_data);
};
template<>
struct Signals<GtkMenuButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkMenuButton*, gpointer  user_data);
};
template<>
struct Signals<GtkNativeDialog, 0> {
    static constexpr const char* name() { return "response"; }
    using callback_type = void(GtkNativeDialog*, gint  response_id, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 0> {
    static constexpr const char* name() { return "change-current-page"; }
    using callback_type = gboolean(GtkNotebook*, gint  page, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 1> {
    static constexpr const char* name() { return "create-window"; }
    using callback_type = GtkNotebook*(GtkNotebook*, GtkWidget*  page, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 2> {
    static constexpr const char* name() { return "focus-tab"; }
    using callback_type = gboolean(GtkNotebook*, GtkNotebookTab*  tab, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 3> {
    static constexpr const char* name() { return "move-focus-out"; }
    using callback_type = void(GtkNotebook*, GtkDirectionType  direction, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 4> {
    static constexpr const char* name() { return "page-added"; }
    using callback_type = void(GtkNotebook*, GtkWidget*  child, guint  page_num, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 5> {
    static constexpr const char* name() { return "page-removed"; }
    using callback_type = void(GtkNotebook*, GtkWidget*  child, guint  page_num, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 6> {
    static constexpr const char* name() { return "page-reordered"; }
    using callback_type = void(GtkNotebook*, GtkWidget*  child, guint  page_num, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 7> {
    static constexpr const char* name() { return "reorder-tab"; }
    using callback_type = gboolean(GtkNotebook*, GtkDirectionType  direction, gboolean  move_to_last, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 8> {
    static constexpr const char* name() { return "select-page"; }
    using callback_type = gboolean(GtkNotebook*, gboolean  move_focus, gpointer  user_data);
};
template<>
struct Signals<GtkNotebook, 9> {
    static constexpr const char* name() { return "switch-page"; }
    using callback_type = void(GtkNotebook*, GtkWidget*  page, guint  page_num, gpointer  user_data);
};
template<>
struct Signals<GtkOverlay, 0> {
    static constexpr const char* name() { return "get-child-position"; }
    using callback_type = gboolean(GtkOverlay*, GtkWidget*  widget, GdkRectangle*  allocation, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 0> {
    static constexpr const char* name() { return "accept-position"; }
    using callback_type = gboolean(GtkPaned*, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 1> {
    static constexpr const char* name() { return "cancel-position"; }
    using callback_type = gboolean(GtkPaned*, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 2> {
    static constexpr const char* name() { return "cycle-child-focus"; }
    using callback_type = gboolean(GtkPaned*, gboolean  reversed, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 3> {
    static constexpr const char* name() { return "cycle-handle-focus"; }
    using callback_type = gboolean(GtkPaned*, gboolean  reversed, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 4> {
    static constexpr const char* name() { return "move-handle"; }
    using callback_type = gboolean(GtkPaned*, GtkScrollType*  scroll_type, gpointer  user_data);
};
template<>
struct Signals<GtkPaned, 5> {
    static constexpr const char* name() { return "toggle-handle-focus"; }
    using callback_type = gboolean(GtkPaned*, gpointer  user_data);
};
template<>
struct Signals<GtkPasswordEntry, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkPasswordEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkPopover, 0> {
    static constexpr const char* name() { return "activate-default"; }
    using callback_type = void(GtkPopover*, gpointer  user_data);
};
template<>
struct Signals<GtkPopover, 1> {
    static constexpr const char* name() { return "closed"; }
    using callback_type = void(GtkPopover*, gpointer  user_data);
};
template<>
struct Signals<GtkPrinter, 0> {
    static constexpr const char* name() { return "details-acquired"; }
    using callback_type = void(GtkPrinter*, gboolean  success, gpointer  user_data);
};
template<>
struct Signals<GtkPrintJob, 0> {
    static constexpr const char* name() { return "status-changed"; }
    using callback_type = void(GtkPrintJob*, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 0> {
    static constexpr const char* name() { return "begin-print"; }
    using callback_type = void(GtkPrintOperation*, GtkPrintContext*  context, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 1> {
    static constexpr const char* name() { return "create-custom-widget"; }
    using callback_type = GObject*(GtkPrintOperation*, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 2> {
    static constexpr const char* name() { return "custom-widget-apply"; }
    using callback_type = void(GtkPrintOperation*, GtkWidget*  widget, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 3> {
    static constexpr const char* name() { return "done"; }
    using callback_type = void(GtkPrintOperation*, GtkPrintOperationResult  result, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 4> {
    static constexpr const char* name() { return "draw-page"; }
    using callback_type = void(GtkPrintOperation*, GtkPrintContext*  context, gint  page_nr, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 5> {
    static constexpr const char* name() { return "end-print"; }
    using callback_type = void(GtkPrintOperation*, GtkPrintContext*  context, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 6> {
    static constexpr const char* name() { return "paginate"; }
    using callback_type = gboolean(GtkPrintOperation*, GtkPrintContext*  context, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 7> {
    static constexpr const char* name() { return "preview"; }
    using callback_type = gboolean(GtkPrintOperation*, GtkPrintOperationPreview*  preview, GtkPrintContext*  context, GtkWindow*  parent, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 8> {
    static constexpr const char* name() { return "request-page-setup"; }
    using callback_type = void(GtkPrintOperation*, GtkPrintContext*  context, gint  page_nr, GtkPageSetup*  setup, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 9> {
    static constexpr const char* name() { return "status-changed"; }
    using callback_type = void(GtkPrintOperation*, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperation, 10> {
    static constexpr const char* name() { return "update-custom-widget"; }
    using callback_type = void(GtkPrintOperation*, GtkWidget*  widget, GtkPageSetup*  setup, GtkPrintSettings*  settings, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperationPreview, 0> {
    static constexpr const char* name() { return "got-page-size"; }
    using callback_type = void(GtkPrintOperationPreview*, GtkPrintContext*  context, GtkPageSetup*  page_setup, gpointer  user_data);
};
template<>
struct Signals<GtkPrintOperationPreview, 1> {
    static constexpr const char* name() { return "ready"; }
    using callback_type = void(GtkPrintOperationPreview*, GtkPrintContext*  context, gpointer  user_data);
};
template<>
struct Signals<GtkRange, 0> {
    static constexpr const char* name() { return "adjust-bounds"; }
    using callback_type = void(GtkRange*, gdouble  value, gpointer  user_data);
};
template<>
struct Signals<GtkRange, 1> {
    static constexpr const char* name() { return "change-value"; }
    using callback_type = gboolean(GtkRange*, GtkScrollType*  scroll, gdouble  value, gpointer  user_data);
};
template<>
struct Signals<GtkRange, 2> {
    static constexpr const char* name() { return "move-slider"; }
    using callback_type = void(GtkRange*, GtkScrollType*  step, gpointer  user_data);
};
template<>
struct Signals<GtkRange, 3> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkRange*, gpointer  user_data);
};
template<>
struct Signals<GtkRecentManager, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkRecentManager*, gpointer  user_data);
};
template<>
struct Signals<GtkScaleButton, 0> {
    static constexpr const char* name() { return "popdown"; }
    using callback_type = void(GtkScaleButton*, gpointer  user_data);
};
template<>
struct Signals<GtkScaleButton, 1> {
    static constexpr const char* name() { return "popup"; }
    using callback_type = void(GtkScaleButton*, gpointer  user_data);
};
template<>
struct Signals<GtkScaleButton, 2> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkScaleButton*, gdouble  value, gpointer  user_data);
};
template<>
struct Signals<GtkScrolledWindow, 0> {
    static constexpr const char* name() { return "edge-overshot"; }
    using callback_type = void(GtkScrolledWindow*, GtkPositionType  pos, gpointer  user_data);
};
template<>
struct Signals<GtkScrolledWindow, 1> {
    static constexpr const char* name() { return "edge-reached"; }
    using callback_type = void(GtkScrolledWindow*, GtkPositionType  pos, gpointer  user_data);
};
template<>
struct Signals<GtkScrolledWindow, 2> {
    static constexpr const char* name() { return "move-focus-out"; }
    using callback_type = void(GtkScrolledWindow*, GtkDirectionType  direction_type, gpointer  user_data);
};
template<>
struct Signals<GtkScrolledWindow, 3> {
    static constexpr const char* name() { return "scroll-child"; }
    using callback_type = gboolean(GtkScrolledWindow*, GtkScrollType*  scroll, gboolean  horizontal, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 1> {
    static constexpr const char* name() { return "next-match"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 2> {
    static constexpr const char* name() { return "previous-match"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 3> {
    static constexpr const char* name() { return "search-changed"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 4> {
    static constexpr const char* name() { return "search-started"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSearchEntry, 5> {
    static constexpr const char* name() { return "stop-search"; }
    using callback_type = void(GtkSearchEntry*, gpointer  user_data);
};
template<>
struct Signals<GtkSectionModel, 0> {
    static constexpr const char* name() { return "sections-changed"; }
    using callback_type = void(GtkSectionModel*, guint  position, guint  n_items, gpointer  user_data);
};
template<>
struct Signals<GtkSelectionModel, 0> {
    static constexpr const char* name() { return "selection-changed"; }
    using callback_type = void(GtkSelectionModel*, guint  position, guint  n_items, gpointer  user_data);
};
template<>
struct Signals<GtkShortcutsSection, 0> {
    static constexpr const char* name() { return "change-current-page"; }
    using callback_type = gboolean(GtkShortcutsSection*, gint  offset, gpointer  user_data);
};
template<>
struct Signals<GtkShortcutsWindow, 0> {
    static constexpr const char* name() { return "close"; }
    using callback_type = void(GtkShortcutsWindow*, gpointer  user_data);
};
template<>
struct Signals<GtkShortcutsWindow, 1> {
    static constexpr const char* name() { return "search"; }
    using callback_type = void(GtkShortcutsWindow*, gpointer  user_data);
};
template<>
struct Signals<GtkSignalListItemFactory, 0> {
    static constexpr const char* name() { return "bind"; }
    using callback_type = void(GtkSignalListItemFactory*, GObject*  object, gpointer  user_data);
};
template<>
struct Signals<GtkSignalListItemFactory, 1> {
    static constexpr const char* name() { return "setup"; }
    using callback_type = void(GtkSignalListItemFactory*, GObject*  object, gpointer  user_data);
};
template<>
struct Signals<GtkSignalListItemFactory, 2> {
    static constexpr const char* name() { return "teardown"; }
    using callback_type = void(GtkSignalListItemFactory*, GObject*  object, gpointer  user_data);
};
template<>
struct Signals<GtkSignalListItemFactory, 3> {
    static constexpr const char* name() { return "unbind"; }
    using callback_type = void(GtkSignalListItemFactory*, GObject*  object, gpointer  user_data);
};
template<>
struct Signals<GtkSorter, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkSorter*, GtkSorterChange  change, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkSpinButton*, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 1> {
    static constexpr const char* name() { return "change-value"; }
    using callback_type = void(GtkSpinButton*, GtkScrollType*  scroll, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 2> {
    static constexpr const char* name() { return "input"; }
    using callback_type = gint(GtkSpinButton*, gdouble*  new_value, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 3> {
    static constexpr const char* name() { return "output"; }
    using callback_type = gboolean(GtkSpinButton*, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 4> {
    static constexpr const char* name() { return "value-changed"; }
    using callback_type = void(GtkSpinButton*, gpointer  user_data);
};
template<>
struct Signals<GtkSpinButton, 5> {
    static constexpr const char* name() { return "wrapped"; }
    using callback_type = void(GtkSpinButton*, gpointer  user_data);
};
template<>
struct Signals<GtkStatusbar, 0> {
    static constexpr const char* name() { return "text-popped"; }
    using callback_type = void(GtkStatusbar*, guint  context_id, gchar*  text, gpointer  user_data);
};
template<>
struct Signals<GtkStatusbar, 1> {
    static constexpr const char* name() { return "text-pushed"; }
    using callback_type = void(GtkStatusbar*, guint  context_id, gchar*  text, gpointer  user_data);
};
template<>
struct Signals<GtkSwitch, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkSwitch*, gpointer  user_data);
};
template<>
struct Signals<GtkSwitch, 1> {
    static constexpr const char* name() { return "state-set"; }
    using callback_type = gboolean(GtkSwitch*, gboolean  state, gpointer  user_data);
};
template<>
struct Signals<GtkText, 0> {
    static constexpr const char* name() { return "activate"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 1> {
    static constexpr const char* name() { return "backspace"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 2> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 3> {
    static constexpr const char* name() { return "cut-clipboard"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 4> {
    static constexpr const char* name() { return "delete-from-cursor"; }
    using callback_type = void(GtkText*, GtkDeleteType*  type, gint  count, gpointer  user_data);
};
template<>
struct Signals<GtkText, 5> {
    static constexpr const char* name() { return "insert-at-cursor"; }
    using callback_type = void(GtkText*, gchar*  string, gpointer  user_data);
};
template<>
struct Signals<GtkText, 6> {
    static constexpr const char* name() { return "insert-emoji"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 7> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkText*, GtkMovementStep*  step, gint  count, gboolean  extend, gpointer  user_data);
};
template<>
struct Signals<GtkText, 8> {
    static constexpr const char* name() { return "paste-clipboard"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkText, 9> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkText*, gchar*  preedit, gpointer  user_data);
};
template<>
struct Signals<GtkText, 10> {
    static constexpr const char* name() { return "toggle-overwrite"; }
    using callback_type = void(GtkText*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 0> {
    static constexpr const char* name() { return "apply-tag"; }
    using callback_type = void(GtkTextBuffer*, GtkTextTag*  tag, const GtkTextIter*  start, const GtkTextIter*  end, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 1> {
    static constexpr const char* name() { return "begin-user-action"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 2> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 3> {
    static constexpr const char* name() { return "delete-range"; }
    using callback_type = void(GtkTextBuffer*,const GtkTextIter*  start, const GtkTextIter*  end, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 4> {
    static constexpr const char* name() { return "end-user-action"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 5> {
    static constexpr const char* name() { return "insert-child-anchor"; }
    using callback_type = void(GtkTextBuffer*,const GtkTextIter*  location, GtkTextChildAnchor*  anchor, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 6> {
    static constexpr const char* name() { return "insert-paintable"; }
    using callback_type = void(GtkTextBuffer*,const GtkTextIter*  location, GdkPaintable*  paintable, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 7> {
    static constexpr const char* name() { return "insert-text"; }
    using callback_type = void(GtkTextBuffer*,const GtkTextIter*  location, gchar*  text, gint  len, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 8> {
    static constexpr const char* name() { return "mark-deleted"; }
    using callback_type = void(GtkTextBuffer*, GtkTextMark*  mark, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 9> {
    static constexpr const char* name() { return "mark-set"; }
    using callback_type = void(GtkTextBuffer*,const GtkTextIter*  location, GtkTextMark*  mark, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 10> {
    static constexpr const char* name() { return "modified-changed"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 11> {
    static constexpr const char* name() { return "paste-done"; }
    using callback_type = void(GtkTextBuffer*, GdkClipboard*  clipboard, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 12> {
    static constexpr const char* name() { return "redo"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 13> {
    static constexpr const char* name() { return "remove-tag"; }
    using callback_type = void(GtkTextBuffer*, GtkTextTag*  tag, const GtkTextIter*  start, const GtkTextIter*  end, gpointer  user_data);
};
template<>
struct Signals<GtkTextBuffer, 14> {
    static constexpr const char* name() { return "undo"; }
    using callback_type = void(GtkTextBuffer*, gpointer  user_data);
};
template<>
struct Signals<GtkTextTagTable, 0> {
    static constexpr const char* name() { return "tag-added"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag*  tag, gpointer  user_data);
};
template<>
struct Signals<GtkTextTagTable, 1> {
    static constexpr const char* name() { return "tag-changed"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag*  tag, gboolean  size_changed, gpointer  user_data);
};
template<>
struct Signals<GtkTextTagTable, 2> {
    static constexpr const char* name() { return "tag-removed"; }
    using callback_type = void(GtkTextTagTable*, GtkTextTag*  tag, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 0> {
    static constexpr const char* name() { return "backspace"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 1> {
    static constexpr const char* name() { return "copy-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 2> {
    static constexpr const char* name() { return "cut-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 3> {
    static constexpr const char* name() { return "delete-from-cursor"; }
    using callback_type = void(GtkTextView*, GtkDeleteType*  type, gint  count, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 4> {
    static constexpr const char* name() { return "extend-selection"; }
    using callback_type = gboolean(GtkTextView*, GtkTextExtendSelection  granularity, const GtkTextIter*  location, const GtkTextIter*  start, const GtkTextIter*  end, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 5> {
    static constexpr const char* name() { return "insert-at-cursor"; }
    using callback_type = void(GtkTextView*, gchar*  string, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 6> {
    static constexpr const char* name() { return "insert-emoji"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 7> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = void(GtkTextView*, GtkMovementStep*  step, gint  count, gboolean  extend_selection, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 8> {
    static constexpr const char* name() { return "move-viewport"; }
    using callback_type = void(GtkTextView*, GtkScrollStep*  step, gint  count, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 9> {
    static constexpr const char* name() { return "paste-clipboard"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 10> {
    static constexpr const char* name() { return "preedit-changed"; }
    using callback_type = void(GtkTextView*, gchar*  preedit, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 11> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = void(GtkTextView*, gboolean  select, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 12> {
    static constexpr const char* name() { return "set-anchor"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 13> {
    static constexpr const char* name() { return "toggle-cursor-visible"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkTextView, 14> {
    static constexpr const char* name() { return "toggle-overwrite"; }
    using callback_type = void(GtkTextView*, gpointer  user_data);
};
template<>
struct Signals<GtkToggleButton, 0> {
    static constexpr const char* name() { return "toggled"; }
    using callback_type = void(GtkToggleButton*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeModel, 0> {
    static constexpr const char* name() { return "row-changed"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath*  path, GtkTreeIter*  iter, gpointer  user_data);
};
template<>
struct Signals<GtkTreeModel, 1> {
    static constexpr const char* name() { return "row-deleted"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkTreeModel, 2> {
    static constexpr const char* name() { return "row-has-child-toggled"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath*  path, GtkTreeIter*  iter, gpointer  user_data);
};
template<>
struct Signals<GtkTreeModel, 3> {
    static constexpr const char* name() { return "row-inserted"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath*  path, GtkTreeIter*  iter, gpointer  user_data);
};
template<>
struct Signals<GtkTreeModel, 4> {
    static constexpr const char* name() { return "rows-reordered"; }
    using callback_type = void(GtkTreeModel*, GtkTreePath*  path, GtkTreeIter*  iter, gpointer  new_order, gpointer  user_data);
};
template<>
struct Signals<GtkTreeSelection, 0> {
    static constexpr const char* name() { return "changed"; }
    using callback_type = void(GtkTreeSelection*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeSortable, 0> {
    static constexpr const char* name() { return "sort-column-changed"; }
    using callback_type = void(GtkTreeSortable*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 0> {
    static constexpr const char* name() { return "columns-changed"; }
    using callback_type = void(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 1> {
    static constexpr const char* name() { return "cursor-changed"; }
    using callback_type = void(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 2> {
    static constexpr const char* name() { return "expand-collapse-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gboolean  object, gboolean p0, gboolean p1, gpointer user_data);
};
template<>
struct Signals<GtkTreeView, 3> {
    static constexpr const char* name() { return "move-cursor"; }
    using callback_type = gboolean(GtkTreeView*, GtkMovementStep*  step, gint  direction, gboolean  extend, gboolean  modify, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 4> {
    static constexpr const char* name() { return "row-activated"; }
    using callback_type = void(GtkTreeView*, GtkTreePath*  path, GtkTreeViewColumn*  column, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 5> {
    static constexpr const char* name() { return "row-collapsed"; }
    using callback_type = void(GtkTreeView*, GtkTreeIter*  iter, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 6> {
    static constexpr const char* name() { return "row-expanded"; }
    using callback_type = void(GtkTreeView*, GtkTreeIter*  iter, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 7> {
    static constexpr const char* name() { return "select-all"; }
    using callback_type = gboolean(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 8> {
    static constexpr const char* name() { return "select-cursor-parent"; }
    using callback_type = gboolean(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 9> {
    static constexpr const char* name() { return "select-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gboolean  object, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 10> {
    static constexpr const char* name() { return "start-interactive-search"; }
    using callback_type = gboolean(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 11> {
    static constexpr const char* name() { return "test-collapse-row"; }
    using callback_type = gboolean(GtkTreeView*, GtkTreeIter*  iter, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 12> {
    static constexpr const char* name() { return "test-expand-row"; }
    using callback_type = gboolean(GtkTreeView*, GtkTreeIter*  iter, GtkTreePath*  path, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 13> {
    static constexpr const char* name() { return "toggle-cursor-row"; }
    using callback_type = gboolean(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeView, 14> {
    static constexpr const char* name() { return "unselect-all"; }
    using callback_type = gboolean(GtkTreeView*, gpointer  user_data);
};
template<>
struct Signals<GtkTreeViewColumn, 0> {
    static constexpr const char* name() { return "clicked"; }
    using callback_type = void(GtkTreeViewColumn*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 0> {
    static constexpr const char* name() { return "destroy"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 1> {
    static constexpr const char* name() { return "direction-changed"; }
    using callback_type = void(GtkWidget*, GtkTextDirection  previous_direction, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 2> {
    static constexpr const char* name() { return "hide"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 3> {
    static constexpr const char* name() { return "keynav-failed"; }
    using callback_type = gboolean(GtkWidget*, GtkDirectionType  direction, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 4> {
    static constexpr const char* name() { return "map"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 5> {
    static constexpr const char* name() { return "mnemonic-activate"; }
    using callback_type = gboolean(GtkWidget*, gboolean  group_cycling, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 6> {
    static constexpr const char* name() { return "move-focus"; }
    using callback_type = void(GtkWidget*, GtkDirectionType  direction, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 7> {
    static constexpr const char* name() { return "query-tooltip"; }
    using callback_type = gboolean(GtkWidget*, gint  x, gint  y, gboolean  keyboard_mode, GtkTooltip*  tooltip, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 8> {
    static constexpr const char* name() { return "realize"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 9> {
    static constexpr const char* name() { return "show"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 10> {
    static constexpr const char* name() { return "state-flags-changed"; }
    using callback_type = void(GtkWidget*, GtkStateFlags  flags, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 11> {
    static constexpr const char* name() { return "unmap"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWidget, 12> {
    static constexpr const char* name() { return "unrealize"; }
    using callback_type = void(GtkWidget*, gpointer  user_data);
};
template<>
struct Signals<GtkWindow, 0> {
    static constexpr const char* name() { return "activate-default"; }
    using callback_type = void(GtkWindow*, gpointer  user_data);
};
template<>
struct Signals<GtkWindow, 1> {
    static constexpr const char* name() { return "activate-focus"; }
    using callback_type = void(GtkWindow*, gpointer  user_data);
};
template<>
struct Signals<GtkWindow, 2> {
    static constexpr const char* name() { return "close-request"; }
    using callback_type = gboolean(GtkWindow*, gpointer  user_data);
};
template<>
struct Signals<GtkWindow, 3> {
    static constexpr const char* name() { return "enable-debugging"; }
    using callback_type = gboolean(GtkWindow*, gboolean  toggle, gpointer  user_data);
};
template<>
struct Signals<GtkWindow, 4> {
    static constexpr const char* name() { return "keys-changed"; }
    using callback_type = void(GtkWindow*, gpointer  user_data);
};

}  // namespace xoj::util::signal
