#include "suspend.h"
#include <gtk/gtk.h>

void on_suspend_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;

    GError *error = NULL;
    if (!g_spawn_command_line_async("systemctl suspend", &error)) {
        g_warning("suspend: failed to run 'systemctl suspend': %s", error->message);
        g_error_free(error);
        return;
    }

    GtkWidget *toplevel = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW);
    if (toplevel) {
        GtkApplication *app = gtk_window_get_application(GTK_WINDOW(toplevel));
        if (app) g_application_quit(G_APPLICATION(app));
    }
}
