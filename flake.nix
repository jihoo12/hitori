{
  description = "hitori - a minimal Wayland control panel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        packages = {
          default = pkgs.callPackage ./default.nix { };
          hitori = pkgs.callPackage ./default.nix { };
        };
      }
    ) // {
      homeManagerModules = {
        default = import ./hm-module.nix;
        hitori = import ./hm-module.nix;
      };
    };
}
