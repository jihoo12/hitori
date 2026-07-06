#include "brightness.h"
#include "util.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

static char backlight_path[256] = {0};
static int max_brightness = 1;

void on_brightness_apply(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkRange *range = GTK_RANGE(user_data);
    int percent = (int)gtk_range_get_value(range);

    if (backlight_path[0] == '\0') return;

    char brightness_file[300];
    snprintf(brightness_file, sizeof(brightness_file), "%s/brightness", backlight_path);

    int value = (percent * max_brightness) / 100;
    if (value < 0) value = 0;
    if (value > max_brightness) value = max_brightness;

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "pkexec sh -c \"echo %d > '%s'\"", value, brightness_file);

    GError *error = NULL;
    if (!g_spawn_command_line_async(cmd, &error)) {
        g_warning("brightness apply: failed to run '%s': %s", cmd, error->message);
        g_error_free(error);
    }
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
        strncpy(backlight_path, out, sizeof(backlight_path) - 1);
        backlight_path[sizeof(backlight_path) - 1] = '\0';
        char max_file[300];
        snprintf(max_file, sizeof(max_file), "%s/max_brightness", backlight_path);
        max_brightness = 1;
        read_int_file(max_file, &max_brightness);
        if (max_brightness <= 0) max_brightness = 1;
        found = TRUE;
        break;
    }
    closedir(d);
    return found;
}
