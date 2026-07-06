#include "clock.h"

gboolean update_clock(gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    GDateTime *now = g_date_time_new_now_local();
    char *text = g_date_time_format(now, "%H:%M:%S");
    gtk_label_set_text(GTK_LABEL(label), text);
    g_free(text);
    g_date_time_unref(now);
    return G_SOURCE_CONTINUE;
}
