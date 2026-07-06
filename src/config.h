#pragma once

#include <gtk/gtk.h>

typedef struct {
    char *label;
    char *command;
} CustomButtonEntry;

typedef struct {
    gboolean clock;
    gboolean battery;
    gboolean power_save;
    gboolean charge_limit;
    gboolean brightness;
    gboolean volume;
    gboolean suspend;
    GPtrArray *custom_buttons;
} HitoriConfig;

HitoriConfig *config_load(void);
void config_free(HitoriConfig *cfg);
