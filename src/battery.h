#pragma once

#include <gtk/gtk.h>

extern char battery_path[256];

gboolean find_battery(void);
gboolean has_charge_threshold(void);
gboolean update_battery(gpointer user_data);
void on_charge_limit_toggled(GtkToggleButton *toggle, gpointer user_data);
