{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs.buildPackages;
  let
    # gcc 14.3.0
    # cmake 4.1.2
    # gtest 1.17.0
    # simdjson 4.0.7
    # nlohmann_json 3.12.0
    Main = import (builtins.fetchTarball https://github.com/nixos/nixpkgs/tarball/667993862518f5a890747dfe7aba2c6d0c7787ce) {};
  in
  [
    Main.gcc
    Main.cmake
    Main.gtest
    Main.simdjson
    Main.nlohmann_json
  ];
}
