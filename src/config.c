#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static gboolean get_bool_default(GKeyFile *kf, const char *section,
                                  const char *key, gboolean default_val) {
    GError *error = NULL;
    gboolean val = g_key_file_get_boolean(kf, section, key, &error);
    if (error) {
        g_error_free(error);
        return default_val;
    }
    return val;
}

static void custom_button_entry_free(gpointer data) {
    CustomButtonEntry *entry = (CustomButtonEntry *)data;
    g_free(entry->label);
    g_free(entry->command);
    g_free(entry);
}

HitoriConfig *config_load(const char *config_path) {
    HitoriConfig *cfg = g_new0(HitoriConfig, 1);

    cfg->clock = TRUE;
    cfg->battery = TRUE;
    cfg->power_save = TRUE;
    cfg->charge_limit = TRUE;
    cfg->brightness = TRUE;
    cfg->volume = TRUE;
    cfg->suspend = TRUE;
    cfg->poweroff = TRUE;
    cfg->css_path = NULL;
    cfg->custom_buttons = g_ptr_array_new_with_free_func(custom_button_entry_free);

    char *path = NULL;
    if (config_path) {
        path = g_strdup(config_path);
    } else {
        char *config_dir = g_build_filename(g_get_user_config_dir(), "hitori", NULL);
        path = g_build_filename(config_dir, "config.ini", NULL);
        g_free(config_dir);
    }

    GKeyFile *kf = g_key_file_new();
    if (!g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, NULL)) {
        g_free(path);
        g_key_file_free(kf);
        return cfg;
    }
    g_free(path);

    cfg->clock = get_bool_default(kf, "widgets", "clock", TRUE);
    cfg->battery = get_bool_default(kf, "widgets", "battery", TRUE);
    cfg->power_save = get_bool_default(kf, "widgets", "power_save", TRUE);
    cfg->charge_limit = get_bool_default(kf, "widgets", "charge_limit", TRUE);
    cfg->brightness = get_bool_default(kf, "widgets", "brightness", TRUE);
    cfg->volume = get_bool_default(kf, "widgets", "volume", TRUE);
    cfg->suspend = get_bool_default(kf, "widgets", "suspend", TRUE);
    cfg->poweroff = get_bool_default(kf, "widgets", "poweroff", TRUE);

    cfg->css_path = g_key_file_get_string(kf, "style", "css_path", NULL);

    gsize nkeys = 0;
    gchar **keys = g_key_file_get_keys(kf, "custom_buttons", &nkeys, NULL);
    if (keys) {
        for (gsize i = 0; i < nkeys; i++) {
            char *command = g_key_file_get_value(kf, "custom_buttons", keys[i], NULL);
            if (command && command[0] != '\0') {
                CustomButtonEntry *entry = g_new(CustomButtonEntry, 1);
                entry->label = g_strdup(keys[i]);
                entry->command = command;
                g_ptr_array_add(cfg->custom_buttons, entry);
            } else {
                g_free(command);
            }
        }
        g_strfreev(keys);
    }

    g_key_file_free(kf);
    return cfg;
}

void config_free(HitoriConfig *cfg) {
    if (!cfg) return;
    g_free(cfg->css_path);
    g_ptr_array_unref(cfg->custom_buttons);
    g_free(cfg);
}
