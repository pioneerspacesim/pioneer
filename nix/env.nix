{ pkgs ? import <nixpkgs> { } }:
let
  assimp = pkgs.assimp or pkgs.callPackage ./assimp.nix { };
in
pkgs.myEnvFun {
  name = "pioneer";
  buildInputs = with pkgs; [
    autoconf automake pkgconfig
    libsigcxx SDL2 SDL2_image freetype libvorbis libpng
    assimp mesa
  ];
}