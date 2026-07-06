#include "battery.h"
#include "util.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

char battery_path[300] = {0};

gboolean find_battery(void) {
    const char *base = "/sys/class/power_supply";
    DIR *d = opendir(base);
    if (!d) return FALSE;

    struct dirent *entry;
    gboolean found = FALSE;
    while ((entry = readdir(d)) != NULL) {
        if (strncmp(entry->d_name, "BAT", 3) == 0) {
            snprintf(battery_path, sizeof(battery_path), "%s/%s", base, entry->d_name);
            found = TRUE;
            break;
        }
    }
    closedir(d);
    return found;
}

gboolean has_charge_threshold(void) {
    if (battery_path[0] == '\0') return FALSE;
    char path[300];
    snprintf(path, sizeof(path), "%s/charge_control_end_threshold", battery_path);
    return g_file_test(path, G_FILE_TEST_EXISTS);
}

gboolean update_battery(gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    if (battery_path[0] == '\0') return G_SOURCE_REMOVE;

    char capacity_file[300], status_file[300];
    snprintf(capacity_file, sizeof(capacity_file), "%s/capacity", battery_path);
    snprintf(status_file, sizeof(status_file), "%s/status", battery_path);

    int capacity = -1;
    char status[32] = "Unknown";
    read_int_file(capacity_file, &capacity);
    read_str_file(status_file, status, sizeof(status));

    const char *icon = "\xF0\x9F\x94\x8B";
    if (g_strcmp0(status, "Charging") == 0)
        icon = "\xE2\x9A\xA1";
    else if (capacity >= 0 && capacity <= 15)
        icon = "\xF0\x9F\xAA\xAB";

    char buf[64];
    if (capacity >= 0)
        snprintf(buf, sizeof(buf), "%s %d%%", icon, capacity);
    else
        snprintf(buf, sizeof(buf), "%s --", icon);

    gtk_label_set_text(GTK_LABEL(label), buf);
    return G_SOURCE_CONTINUE;
}

void on_charge_limit_toggled(GtkToggleButton *toggle, gpointer user_data) {
    (void)user_data;
    gboolean active = gtk_toggle_button_get_active(toggle);
    if (battery_path[0] == '\0') return;

    char threshold_file[300];
    snprintf(threshold_file, sizeof(threshold_file), "%s/charge_control_end_threshold", battery_path);

    int threshold = active ? 80 : 100;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "pkexec sh -c \"echo %d > '%s'\"", threshold, threshold_file);

    GError *error = NULL;
    if (!g_spawn_command_line_async(cmd, &error)) {
        g_warning("charge limit toggle: failed to run '%s': %s", cmd, error->message);
        g_error_free(error);
        return;
    }

    gtk_button_set_label(GTK_BUTTON(toggle), active ? "Charge Limit: 80%" : "Charge Limit: Off");
}

int read_charge_threshold(void) {
    if (battery_path[0] == '\0') return 0;
    char path[300];
    snprintf(path, sizeof(path), "%s/charge_control_end_threshold", battery_path);
    int val = 0;
    if (!read_int_file(path, &val)) return 0;
    return val;
}
