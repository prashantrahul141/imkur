{
  description = "imkur package and development environment";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";
  };

  outputs =
    inputs:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forEachSupportedSystem =
        f:
        inputs.nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import inputs.nixpkgs { inherit system; };
          }
        );
    in
    {

      devShells = forEachSupportedSystem (
        { pkgs }:
        {
          default =
            pkgs.mkShell.override
              {
                stdenv = pkgs.clangStdenv;
              }
              {
                nativeBuildInputs =
                  with pkgs;
                  [
                    cmake
                    pkg-config
                    meson
                    ccache
                    ninja

                  ]
                  ++ (
                    if stdenv.isLinux then
                      [
                        gdb
                      ]
                    else
                      [ ]
                  );

                buildInputs = with pkgs; [
                  libGL
                  xorg.libX11
                  xorg.libX11.dev
                  xorg.libXcursor
                  xorg.libXi
                  xorg.libXinerama
                  xorg.libXrandr
                  glfw
                  dbus
                ];

                LD_LIBRARY_PATH =
                  with pkgs;
                  lib.makeLibraryPath [
                    libGL
                    xorg.libXrandr
                    xorg.libXinerama
                    xorg.libXcursor
                    xorg.libXi
                    dbus.dev
                  ];
                LIBCLANG_PATH = "${pkgs.libclang.lib}/lib";

                shellHook = '''';
              };
        }
      );
    };
}
