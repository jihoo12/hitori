{ lib, stdenv, pkg-config, gtk4, gtk4-layer-shell }:

stdenv.mkDerivation {
  pname = "hitori";
  version = "0.3.2";
  src = ./.;

  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ gtk4 gtk4-layer-shell ];

  buildPhase = ''
    make CC=${stdenv.cc.targetPrefix}cc
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp hitori $out/bin/
  '';

  meta = with lib; {
    description = "A minimal Wayland control panel";
    longDescription = ''
      A small panel that renders as a Wayland overlay via GTK4 Layer Shell,
      featuring clock, battery status, power-save toggle, charge limit,
      brightness slider, volume slider, and suspend button.
    '';
    homepage = "https://github.com/jihoo12/hitori";
    license = licenses.asl20;
    platforms = platforms.linux;
    maintainers = [ ];
  };
}
