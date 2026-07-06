#pragma once

#include <gtk/gtk.h>

gboolean find_backlight(char *out, size_t len);
void on_brightness_changed(GtkRange *range, gpointer user_data);
