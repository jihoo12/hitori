#include "brightness.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

static guint brightness_debounce_id = 0;

static gboolean apply_brightness(gpointer user_data) {
    GtkRange *range = GTK_RANGE(user_data);
    int value = (int)gtk_range_get_value(range);

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "brightnessctl set %d%%", value);

    GError *error = NULL;
    if (!g_spawn_command_line_async(cmd, &error)) {
        g_warning("brightness slider: failed to run '%s': %s", cmd, error->message);
        g_error_free(error);
    }

    brightness_debounce_id = 0;
    return G_SOURCE_REMOVE;
}

void on_brightness_changed(GtkRange *range, gpointer user_data) {
    (void)user_data;
    if (brightness_debounce_id != 0)
        g_source_remove(brightness_debounce_id);
    brightness_debounce_id = g_timeout_add(150, apply_brightness, range);
}

gboolean find_backlight(char *out, size_t len) {
    const char *base = "/sys/class/backlight";
    DIR *d = opendir(base);
    if (!d) return FALSE;

    struct dirent *entry;
    gboolean found = FALSE;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        snprintf(out, len, "%s/%s", base, entry->d_name);
        found = TRUE;
        break;
    }
    closedir(d);
    return found;
}
