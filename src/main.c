#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include "config.h"
#include "clock.h"
#include "battery.h"
#include "power.h"
#include "brightness.h"
#include "volume.h"
#include "suspend.h"
#include "poweroff.h"
#include "util.h"

#define BOARD_SIZE 600
#define BOARD_MARGIN 12

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state, gpointer user_data) {
    (void)keycode;
    (void)state;
    (void)user_data;
    if (keyval == GDK_KEY_Escape) {
        GtkWidget *window = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
        GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
        if (app) g_application_quit(G_APPLICATION(app));
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

static void load_css(const char *css_path) {
    GtkCssProvider *provider = gtk_css_provider_new();
    if (css_path) {
        gtk_css_provider_load_from_path(provider, css_path);
    } else {
        const char *css =
            "@keyframes fadeIn { from { opacity: 0; transform: scale(0.95); } }"
            "window { background-color: rgba(30, 30, 46, 0.94); border-radius: 14px; "
            "  min-width: 260px; animation: fadeIn 0.15s ease-out; }"
            "window.background { background-color: rgba(30, 30, 46, 0.94); }"
            "box.panel-section { padding: 14px 18px; }"
            "label.panel-clock { color: #cdd6f4; font-weight: 600; font-size: 15px; }"
            "label.section-label { color: #585b70; font-size: 10px; font-weight: 700; "
            "  letter-spacing: 1.2px; margin-top: 6px; margin-bottom: 2px; }"
            "entry { background-color: #313244; color: #cdd6f4; "
            "  border-radius: 8px; border: 1px solid #45475a; "
            "  padding: 9px 12px; font-size: 14px; caret-color: #cdd6f4; "
            "  min-height: 18px; }"
            "entry:focus { border-color: #b4befe; background-color: #45475a; }"
            "button { background-color: #89b4fa; color: #1e1e2e; "
            "  border-radius: 8px; border: none; "
            "  padding: 8px 16px; font-size: 13px; min-width: 140px; font-weight: 700; }"
            "button:hover { background-color: #9fc3fc; }"
            "button:active { background-color: #74a5f8; }"
            "button.danger-button { background-color: #f38ba8; color: #1e1e2e; }"
            "button.danger-button:hover { background-color: #f5a2ba; }"
            "button.danger-button:active { background-color: #f17496; }"
            "togglebutton { background-color: #585b70; color: #cdd6f4; "
            "  border-radius: 8px; border: 1px solid #6c7086; "
            "  padding: 8px 16px; font-size: 13px; min-width: 140px; font-weight: 600; }"
            "togglebutton:hover { background-color: #6c7086; }"
            "togglebutton:checked { background-color: #a6e3a1; color: #1e1e2e; "
            "  border: none; font-weight: 700; }"
            "togglebutton:checked:hover { background-color: #b8eab4; }"
            "scale { margin: 3px 0; }"
            "scale trough { background-color: #313244; "
            "  border-radius: 6px; min-height: 6px; }"
            "scale highlight { background-color: #89b4fa; "
            "  border-radius: 6px; }"
            "scale slider { background-color: #cdd6f4; border-radius: 50%; "
            "  min-width: 14px; min-height: 14px; margin: -5px; }"
            "scale value { color: #585b70; font-size: 11px; }"
            "separator { background-color: #313244; "
            "  margin: 3px 0; min-height: 1px; }";
        gtk_css_provider_load_from_string(provider, css);
    }
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void on_launch_entry_activate(GtkEntry *entry, gpointer user_data) {
    GtkWidget *window = (GtkWidget *)user_data;
    const char *text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry));
    if (text[0] == '\0') return;

    GError *error = NULL;
    if (!g_spawn_command_line_async(text, &error)) {
        g_warning("launch: %s", error->message);
        g_error_free(error);
        return;
    }
    GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
    if (app) g_application_quit(G_APPLICATION(app));
}

static void on_custom_button_clicked(GtkButton *button, gpointer user_data) {
    (void)user_data;
    const char *command = g_object_get_data(G_OBJECT(button), "cmd");
    GError *error = NULL;
    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("custom button: %s", error->message);
        g_error_free(error);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    const char *custom_path = (const char *)user_data;
    HitoriConfig *cfg = config_load(custom_path);
    load_css(cfg->css_path);

    GtkWidget *window = gtk_application_window_new(app);

    gtk_layer_init_for_window(GTK_WINDOW(window));

    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);

    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, BOARD_MARGIN);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, BOARD_MARGIN);

    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), 0);

    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
    gtk_widget_add_controller(window, key_controller);

    gtk_widget_set_size_request(window, BOARD_SIZE, BOARD_SIZE);

    GtkWidget *board = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(board, "panel-section");
    gtk_widget_set_valign(board, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(board, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(board, TRUE);
    gtk_widget_set_hexpand(board, TRUE);
    gtk_window_set_child(GTK_WINDOW(window), board);

    GtkWidget *title_label = gtk_label_new("control panel");
    gtk_widget_add_css_class(title_label, "panel-clock");
    gtk_box_append(GTK_BOX(board), title_label);

    GtkWidget *launch_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(launch_entry), "launch program...");
    gtk_widget_set_hexpand(launch_entry, TRUE);
    g_signal_connect(launch_entry, "activate", G_CALLBACK(on_launch_entry_activate), window);
    gtk_box_append(GTK_BOX(board), launch_entry);

    GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(board), sep1);

    if (cfg->clock) {
        GtkWidget *clock_label = gtk_label_new("");
        gtk_widget_add_css_class(clock_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), clock_label);
        update_clock(clock_label);
    }

    gboolean have_battery = find_battery();

    if (have_battery && cfg->battery) {
        GtkWidget *battery_label = gtk_label_new("");
        gtk_widget_add_css_class(battery_label, "panel-clock");
        gtk_box_append(GTK_BOX(board), battery_label);
        update_battery(battery_label);
    }

    if (cfg->power_save) {
        gboolean power_save_active = FALSE;
        if (g_find_program_in_path("powerprofilesctl"))
            power_save_active = get_power_save_active();
        GtkWidget *power_save_toggle = gtk_toggle_button_new_with_label(
            power_save_active ? "Power Save: On" : "Power Save: Off");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(power_save_toggle), power_save_active);
        g_signal_connect(power_save_toggle, "toggled", G_CALLBACK(on_power_save_toggled), NULL);
        gtk_widget_set_halign(power_save_toggle, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), power_save_toggle);
    }

    if (have_battery && has_charge_threshold() && cfg->charge_limit) {
        gboolean charge_limit_active = (read_charge_threshold() == 80);
        GtkWidget *charge_limit_toggle = gtk_toggle_button_new_with_label(
            charge_limit_active ? "Charge Limit: 80%" : "Charge Limit: Off");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(charge_limit_toggle), charge_limit_active);
        g_signal_connect(charge_limit_toggle, "toggled", G_CALLBACK(on_charge_limit_toggled), NULL);
        gtk_widget_set_halign(charge_limit_toggle, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), charge_limit_toggle);
    }

    if (cfg->suspend) {
        GtkWidget *suspend_button = gtk_button_new_with_label("Suspend");
        gtk_widget_add_css_class(suspend_button, "danger-button");
        g_signal_connect(suspend_button, "clicked", G_CALLBACK(on_suspend_clicked), NULL);
        gtk_widget_set_halign(suspend_button, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), suspend_button);
    }

    if (cfg->poweroff) {
        GtkWidget *poweroff_button = gtk_button_new_with_label("Poweroff");
        gtk_widget_add_css_class(poweroff_button, "danger-button");
        g_signal_connect(poweroff_button, "clicked", G_CALLBACK(on_poweroff_clicked), NULL);
        gtk_widget_set_halign(poweroff_button, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), poweroff_button);
    }

    if (cfg->brightness) {
        char backlight_path[256] = {0};
        if (find_backlight(backlight_path, sizeof(backlight_path))) {
            char cur_file[300], max_file[300];
            snprintf(cur_file, sizeof(cur_file), "%s/brightness", backlight_path);
            snprintf(max_file, sizeof(max_file), "%s/max_brightness", backlight_path);

            int cur = 0, max = 1;
            read_int_file(cur_file, &cur);
            read_int_file(max_file, &max);
            int initial_percent = (max > 0) ? (cur * 100) / max : 50;

            GtkWidget *brightness_sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_box_append(GTK_BOX(board), brightness_sep);
            GtkWidget *brightness_label = gtk_label_new("BRIGHTNESS");
            gtk_widget_add_css_class(brightness_label, "section-label");
            gtk_box_append(GTK_BOX(board), brightness_label);

            GtkWidget *brightness_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
            gtk_range_set_value(GTK_RANGE(brightness_scale), initial_percent);
            gtk_scale_set_draw_value(GTK_SCALE(brightness_scale), TRUE);
            gtk_widget_set_size_request(brightness_scale, 180, -1);
            gtk_box_append(GTK_BOX(board), brightness_scale);

            GtkWidget *brightness_apply = gtk_button_new_with_label("Apply Brightness");
            g_signal_connect(brightness_apply, "clicked", G_CALLBACK(on_brightness_apply), brightness_scale);
            gtk_widget_set_halign(brightness_apply, GTK_ALIGN_CENTER);
            gtk_box_append(GTK_BOX(board), brightness_apply);
        }
    }

    if (cfg->volume) {
        int initial_volume = 50;
        gboolean have_wpctl = (g_find_program_in_path("wpctl") != NULL);

        GtkWidget *volume_sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_append(GTK_BOX(board), volume_sep);
        GtkWidget *volume_label = gtk_label_new("VOLUME");
        gtk_widget_add_css_class(volume_label, "section-label");
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

    for (guint i = 0; i < cfg->custom_buttons->len; i++) {
        CustomButtonEntry *entry = g_ptr_array_index(cfg->custom_buttons, i);
        GtkWidget *btn = gtk_button_new_with_label(entry->label);
        g_object_set_data_full(G_OBJECT(btn), "cmd", g_strdup(entry->command), g_free);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_custom_button_clicked), NULL);
        gtk_widget_set_halign(btn, GTK_ALIGN_CENTER);
        gtk_box_append(GTK_BOX(board), btn);
    }

    config_free(cfg);
    gtk_window_present(GTK_WINDOW(window));
    gtk_widget_grab_focus(launch_entry);
}

int main(int argc, char *argv[]) {
    gchar *custom_config = NULL;
    GOptionEntry entries[] = {
        { "config", 'c', 0, G_OPTION_ARG_STRING, &custom_config,
          "Path to config file (default: ~/.config/hitori/config.ini)", "PATH" },
        { NULL, 0, 0, 0, NULL, NULL, NULL }
    };

    GOptionContext *ctx = g_option_context_new("- control panel");
    g_option_context_add_main_entries(ctx, entries, NULL);
    GError *error = NULL;
    if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
        g_print("hitori: %s\n", error->message);
        g_error_free(error);
        g_option_context_free(ctx);
        return 1;
    }
    g_option_context_free(ctx);

    GtkApplication *app = gtk_application_new("com.example.gtk4layershelldemo",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), custom_config);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_free(custom_config);
    g_object_unref(app);
    return status;
}
