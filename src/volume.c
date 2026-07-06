#include "volume.h"
#include <stdio.h>

static guint volume_debounce_id = 0;

static gboolean apply_volume(gpointer user_data) {
    GtkRange *range = GTK_RANGE(user_data);
    int value = (int)gtk_range_get_value(range);

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "wpctl set-volume @DEFAULT_AUDIO_SINK@ %d%%", value);

    GError *error = NULL;
    if (!g_spawn_command_line_async(cmd, &error)) {
        g_warning("volume slider: failed to run '%s': %s", cmd, error->message);
        g_error_free(error);
    }

    volume_debounce_id = 0;
    return G_SOURCE_REMOVE;
}

void on_volume_changed(GtkRange *range, gpointer user_data) {
    (void)user_data;
    if (volume_debounce_id != 0)
        g_source_remove(volume_debounce_id);
    volume_debounce_id = g_timeout_add(150, apply_volume, range);
}

gboolean find_initial_volume(int *out_percent) {
    char *output = NULL;
    if (!g_spawn_command_line_sync("wpctl get-volume @DEFAULT_AUDIO_SINK@",
                                    &output, NULL, NULL, NULL))
        return FALSE;
    if (!output) return FALSE;

    float vol = 0.0;
    int ok = (sscanf(output, "Volume: %f", &vol) == 1);
    g_free(output);
    if (!ok) return FALSE;

    *out_percent = (int)(vol * 100 + 0.5f);
    if (*out_percent < 0) *out_percent = 0;
    if (*out_percent > 100) *out_percent = 100;
    return TRUE;
}
