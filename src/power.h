#pragma once

#include <gtk/gtk.h>

void on_power_save_toggled(GtkToggleButton *toggle, gpointer user_data);
gboolean get_power_save_active(void);
