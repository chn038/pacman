{
    inputs.nixpkgs.url = "github:Nixos/nixpkgs/nixpkgs-unstable";

    outputs = {self, nixpkgs}:
        let
            pkgs = import nixpkgs { system = "x86_64-linux"; };
        in {
            devShells.x86_64-linux.default = pkgs.mkShell {
                packages = with pkgs; [
                    dbus
                    allegro5
                    gcc
                    pkg-config
                    zsh
                ];
                dbus = pkgs.dbus;
                shellHook = ''
                    zsh
                    exit
                '';
            };
        };
}
