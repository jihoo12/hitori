#pragma once

#include <gtk/gtk.h>

gboolean find_initial_volume(int *out_percent);
void on_volume_changed(GtkRange *range, gpointer user_data);
