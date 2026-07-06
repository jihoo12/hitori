{ config, lib, pkgs, ... }:

with lib;

let
  cfg = config.programs.hitori;

  ini = generators.toINI { } {
    widgets = {
      clock = cfg.widgets.clock;
      battery = cfg.widgets.battery;
      power_save = cfg.widgets.powerSave;
      charge_limit = cfg.widgets.chargeLimit;
      brightness = cfg.widgets.brightness;
      volume = cfg.widgets.volume;
      suspend = cfg.widgets.suspend;
    };
    custom_buttons = cfg.customButtons;
  };
in {
  options.programs.hitori = {
    enable = mkEnableOption "hitori - a minimal Wayland control panel";

    package = mkPackageOption pkgs "hitori" { };

    widgets = {
      clock = mkOption {
        type = types.bool;
        default = true;
        description = "Show the clock widget.";
      };
      battery = mkOption {
        type = types.bool;
        default = true;
        description = "Show the battery status widget.";
      };
      powerSave = mkOption {
        type = types.bool;
        default = true;
        description = "Show the power-save toggle button.";
      };
      chargeLimit = mkOption {
        type = types.bool;
        default = true;
        description = "Show the charge limit toggle (requires battery with charge_control_end_threshold).";
      };
      brightness = mkOption {
        type = types.bool;
        default = true;
        description = "Show the brightness slider.";
      };
      volume = mkOption {
        type = types.bool;
        default = true;
        description = "Show the volume slider.";
      };
      suspend = mkOption {
        type = types.bool;
        default = true;
        description = "Show the suspend button.";
      };
    };
  };

  config = mkIf cfg.enable {
    home.packages = [ cfg.package ];

    xdg.configFile."hitori/config.ini".text = ini;
  };
}
