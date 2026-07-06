#include "power.h"
#include <string.h>

gboolean get_power_save_active(void) {
    char *output = NULL;
    GError *error = NULL;
    if (!g_spawn_command_line_sync("powerprofilesctl get", &output, NULL, NULL, &error)) {
        g_warning("power save check: %s", error->message);
        g_error_free(error);
        return FALSE;
    }
    if (!output) return FALSE;
    g_strstrip(output);
    gboolean active = (strcmp(output, "power-saver") == 0);
    g_free(output);
    return active;
}

void on_power_save_toggled(GtkToggleButton *toggle, gpointer user_data) {
    (void)user_data;
    gboolean active = gtk_toggle_button_get_active(toggle);
    const char *cmd = active ? "powerprofilesctl set power-saver"
                              : "powerprofilesctl set balanced";

    GError *error = NULL;
    if (!g_spawn_command_line_async(cmd, &error)) {
        g_warning("power save toggle: failed to run '%s': %s", cmd, error->message);
        g_error_free(error);
    }

    gtk_button_set_label(GTK_BUTTON(toggle), active ? "Power Save: On" : "Power Save: Off");
}
