{
  inputs = {
    utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  };

  outputs = { nixpkgs, utils, ... }:
    utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [ clang-tools clang_20 raylib ];
        };

        defaultPackage = pkgs.stdenv.mkDerivation {
          pname = "scratchrat";
          version = "1.0";

          src = ./.;

          buildInputs = with pkgs; [ clang_20 raylib ];

          nativeBuildInputs = [ pkgs.makeWrapper ];

          buildPhase = ''
            ${pkgs.clang}/bin/clang -std=c23 -pedantic -W -Wall -Wextra -c scratchrat.c -o scratchrat.o
            ${pkgs.clang}/bin/clang -lm -lpthread -ldl -lrt -lX11 -lraylib scratchrat.o -o scratchrat
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp scratchrat $out/bin/
          '';
        };
      });
}

