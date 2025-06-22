#!/bin/sh
# Install GTK3 development libraries across popular Linux distros
set -e

if [ -f /etc/os-release ]; then
    . /etc/os-release
    distro=$ID
else
    echo "Cannot detect Linux distribution" >&2
    exit 1
fi

case "$distro" in
    debian|ubuntu|linuxmint)
        sudo apt-get update && sudo apt-get install -y libgtk-3-dev libgtksourceview-4-dev
        ;;
    fedora|centos|rhel)
        sudo dnf install -y gtk3-devel gtksourceview4-devel
        ;;
    arch|manjaro)
        sudo pacman -Sy --noconfirm gtk3 gtksourceview4
        ;;
    opensuse*)
        sudo zypper install -y gtk3-devel gtksourceview4-devel
        ;;
    gentoo)
        sudo emerge gtk+:3 dev-libs/libgtksourceview
        ;;
    alpine)
        sudo apk add gtk+3.0-dev gtksourceview4-dev
        ;;
    void)
        sudo xbps-install -Sy gtk+3-devel gtksourceview4-devel
        ;;
    solus)
        sudo eopkg install -y libgtk-3-devel libgtksourceview-4-devel
        ;;
    slackware)
        sudo slackpkg install gtk+3 gtksourceview
        ;;
    *)
        echo "Unsupported distribution: $distro" >&2
        exit 1
        ;;
esac

echo "GTK3 development packages installed for $distro"
