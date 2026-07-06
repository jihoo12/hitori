#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

#define BOARD_SIZE 220
#define BOARD_MARGIN 12

/* Path to the discovered battery, e.g. /sys/class/power_supply/BAT0.
 * Left empty on machines with no battery (desktops). */
static char battery_path[256] = {0};

/* Scan /sys/class/power_supply for the first BAT* entry.
 * Returns TRUE and fills battery_path if one is found. */
static gboolean find_battery(void) {
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

static gboolean read_int_file(const char *path, int *out) {
    FILE *f = fopen(path, "r");
    if (!f) return FALSE;
    gboolean ok = (fscanf(f, "%d", out) == 1);
    fclose(f);
    return ok;
}

static gboolean read_str_file(const char *path, char *out, size_t len) {
    FILE *f = fopen(path, "r");
    if (!f) return FALSE;
    if (!fgets(out, (int)len, f)) { fclose(f); return FALSE; }
    fclose(f);
    out[strcspn(out, "\n")] = '\0';
    return TRUE;
}

/* Timer callback: refresh the battery label. Returns G_SOURCE_REMOVE
 * to stop rescheduling itself if something goes wrong finding files. */
static gboolean update_battery(gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    if (battery_path[0] == '\0') return G_SOURCE_REMOVE;

    char capacity_file[300], status_file[300];
    snprintf(capacity_file, sizeof(capacity_file), "%s/capacity", battery_path);
    snprintf(status_file, sizeof(status_file), "%s/status", battery_path);

    int capacity = -1;
    char status[32] = "Unknown";
    read_int_file(capacity_file, &capacity);
    read_str_file(status_file, status, sizeof(status));

    const char *icon = "\xF0\x9F\x94\x8B"; /* battery icon */
    if (g_strcmp0(status, "Charging") == 0)
        icon = "\xE2\x9A\xA1"; /* charging bolt */
    else if (capacity >= 0 && capacity <= 15)
        icon = "\xF0\x9F\xAA\xAB"; /* low battery */

    char buf[64];
    if (capacity >= 0)
        snprintf(buf, sizeof(buf), "%s %d%%", icon, capacity);
    else
        snprintf(buf, sizeof(buf), "%s --", icon);

    gtk_label_set_text(GTK_LABEL(label), buf);
    return G_SOURCE_CONTINUE;
}

static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background-color: rgba(30, 30, 30, 0.95); border-radius: 12px; }"
        "box.panel-section { padding: 12px 16px; }"
        "label.panel-clock { color: #ffffff; font-weight: bold; font-size: 16px; }";
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void activate(GtkApplication *app, gpointer user_data) {
    load_css();

    GtkWidget *window = gtk_application_window_new(app);

    gtk_layer_init_for_window(GTK_WINDOW(window));

    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);

    /* Anchor to a single corner (top-right) instead of stretching
     * across the whole top edge, so the window stays a fixed square
     * rather than a bar. */
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, BOARD_MARGIN);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, BOARD_MARGIN);

    /* A floating square shouldn't reserve a strip of screen the way a
     * bar does, so no exclusive zone. */
    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), 0);

    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

    gtk_widget_set_size_request(window, BOARD_SIZE, BOARD_SIZE);

    /* Vertical stack fits a square much better than the old
     * start/center/end horizontal layout, which was built for a bar. */
    GtkWidget *board = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(board, "panel-section");
    gtk_widget_set_valign(board, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(board, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(board, TRUE);
    gtk_widget_set_hexpand(board, TRUE);
    gtk_window_set_child(GTK_WINDOW(window), board);

    GtkWidget *title_label = gtk_label_new("My Board");
    gtk_widget_add_css_class(title_label, "panel-clock");
    gtk_box_append(GTK_BOX(board), title_label);

    /* Battery indicator: only shown if a battery is actually present. */
    if (find_battery()) {
        GtkWidget *battery_label = gtk_label_new("");
        gtk_widget_add_css_class(battery_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), battery_label);
        update_battery(battery_label);
        g_timeout_add_seconds(5, update_battery, battery_label);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.gtk4layershelldemo",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
