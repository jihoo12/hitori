#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "clock.h"
#include "battery.h"
#include "power.h"
#include "brightness.h"
#include "volume.h"
#include "suspend.h"
#include "util.h"

#define BOARD_SIZE 600
#define BOARD_MARGIN 12

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
    (void)user_data;
    load_css();

    GtkWidget *window = gtk_application_window_new(app);

    gtk_layer_init_for_window(GTK_WINDOW(window));

    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);

    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, BOARD_MARGIN);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, BOARD_MARGIN);

    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), 0);

    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

    gtk_widget_set_size_request(window, BOARD_SIZE, BOARD_SIZE);

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

    GtkWidget *clock_label = gtk_label_new("");
    gtk_widget_add_css_class(clock_label, "panel-clock");
    gtk_box_append(GTK_BOX(board), clock_label);
    update_clock(clock_label);
    g_timeout_add_seconds(1, update_clock, clock_label);

    gboolean have_battery = find_battery();

    if (have_battery) {
        GtkWidget *battery_label = gtk_label_new("");
        gtk_widget_add_css_class(battery_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), battery_label);
        update_battery(battery_label);
        g_timeout_add_seconds(5, update_battery, battery_label);
    }

    GtkWidget *power_save_toggle = gtk_toggle_button_new_with_label("Power Save: Off");
    g_signal_connect(power_save_toggle, "toggled", G_CALLBACK(on_power_save_toggled), NULL);
    gtk_widget_set_halign(power_save_toggle, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(board), power_save_toggle);

    if (have_battery && has_charge_threshold()) {
        GtkWidget *charge_limit_toggle = gtk_toggle_button_new_with_label("Charge Limit: Off");
        g_signal_connect(charge_limit_toggle, "toggled", G_CALLBACK(on_charge_limit_toggled), NULL);
        gtk_widget_set_halign(charge_limit_toggle, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), charge_limit_toggle);
    }

    GtkWidget *suspend_button = gtk_button_new_with_label("Suspend");
    g_signal_connect(suspend_button, "clicked", G_CALLBACK(on_suspend_clicked), NULL);
    gtk_widget_set_halign(suspend_button, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(board), suspend_button);

    char backlight_path[256] = {0};
    if (find_backlight(backlight_path, sizeof(backlight_path))) {
        char cur_file[300], max_file[300];
        snprintf(cur_file, sizeof(cur_file), "%s/brightness", backlight_path);
        snprintf(max_file, sizeof(max_file), "%s/max_brightness", backlight_path);

        int cur = 0, max = 1;
        read_int_file(cur_file, &cur);
        read_int_file(max_file, &max);
        int initial_percent = (max > 0) ? (cur * 100) / max : 50;

        GtkWidget *brightness_label = gtk_label_new("Brightness");
        gtk_widget_add_css_class(brightness_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), brightness_label);

        GtkWidget *brightness_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
        gtk_range_set_value(GTK_RANGE(brightness_scale), initial_percent);
        gtk_scale_set_draw_value(GTK_SCALE(brightness_scale), TRUE);
        gtk_widget_set_size_request(brightness_scale, 180, -1);
        g_signal_connect(brightness_scale, "value-changed", G_CALLBACK(on_brightness_changed), NULL);

        if (!g_find_program_in_path("brightnessctl")) {
            gtk_widget_set_sensitive(brightness_scale, FALSE);
            gtk_widget_set_tooltip_text(brightness_scale, "brightnessctl not found in PATH");
        }

        gtk_box_append(GTK_BOX(board), brightness_scale);
    }

    {
        int initial_volume = 50;
        gboolean have_wpctl = (g_find_program_in_path("wpctl") != NULL);

        GtkWidget *volume_label = gtk_label_new("Volume");
        gtk_widget_add_css_class(volume_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), volume_label);

        GtkWidget *volume_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
        gtk_range_set_value(GTK_RANGE(volume_scale), initial_volume);
        gtk_scale_set_draw_value(GTK_SCALE(volume_scale), TRUE);
        gtk_widget_set_size_request(volume_scale, 180, -1);
        g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_changed), NULL);

        if (!have_wpctl) {
            gtk_widget_set_sensitive(volume_scale, FALSE);
            gtk_widget_set_tooltip_text(volume_scale, "wpctl not found in PATH");
        } else {
            find_initial_volume(&initial_volume);
            gtk_range_set_value(GTK_RANGE(volume_scale), initial_volume);
        }

        gtk_box_append(GTK_BOX(board), volume_scale);
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
