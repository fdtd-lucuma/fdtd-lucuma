[![C/C++ CI](https://github.com/fdtd-lucuma/fdtd-lucuma/workflows/C/C++%20CI/badge.svg)](https://github.com/fdtd-lucuma/fdtd-lucuma/actions?query=workflow%3A%22C%2FC%2B%2B+CI%22)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/fdtd-lucuma/fdtd-lucuma?logo=github)
![GitHub](https://img.shields.io/github/license/fdtd-lucuma/fdtd-lucuma?logo=gnu)

# FDTD GUI
<img width="2560" height="1440" alt="20260411_19h49m57s_grim" src="https://github.com/user-attachments/assets/8c083398-3215-4dd1-b242-ed5bb8f632f5" />


## Now with a pretty fast compilation (thanks to C++20 modules)
<img width="2560" height="1440" alt="20260411_19h52m45s_grim" src="https://github.com/user-attachments/assets/5b5fdc60-b079-4154-abc2-cf8dfae835ab" />


## Run

```bash
git clone https://github.com/fdtd-lucuma/fdtd-lucuma
cd fdtd-lucuma

docker compose up
```

## Run as root (on wayland)
```bash
sudo su

# https://github.com/systemd/systemd/issues/6249
cat > /etc/dbus-1/session-local.conf <<EOF
<busconfig>
  <policy context="mandatory">
    <allow user="root"/>
  </policy>
</busconfig>
EOF

systemctl restart dbus

exit

sudo --preserve-env=XDG_SESSION_TYPE --preserve-env=WAYLAND_DISPLAY --preserve-env=DISPLAY --preserve-env=XDG_RUNTIME_DIR --preserve-env=DBUS_SESSION_BUS_ADDRESS fdtd-lucuma
```

## Dependencies

Read [./pkg/ubuntu/dependencies.txt](./pkg/ubuntu/dependencies.txt)

## Build (Debian experimental)
``` bash
git clone https://github.com/fdtd-lucuma/fdtd-lucuma
cd fdtd-lucuma

vcpkg install shader-slang glm
xargs -oa pkg/ubuntu/dependencies.txt -- sudo apt -t experimental install

cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="$VCPKG_PATH/scripts/buildsystems/vcpkg.cmake"
cmake --build build

cd build
./fdtd-lucuma
```

## Build (Arch Linux)
``` bash
git clone https://github.com/fdtd-lucuma/fdtd-lucuma
cd fdtd-lucuma

paru -Bi pkg/arch/

fdtd-lucuma
```

## Build (any other linux distro)
```bash
git clone https://github.com/fsquillace/junest.git ~/.local/share/junest
export PATH=~/.local/share/junest/bin:$PATH
export PATH="$PATH:~/.junest/usr/bin_wrappers"

junest setup
junest

sudo sed -i 's/#MAKEFLAGS.*/MAKEFLAGS="-j$(nproc)"/g' /etc/makepkg.conf
sudo pacman -Syu base-devel

git clone https://github.com/fdtd-lucuma/fdtd-lucuma
cd fdtd-lucuma

yay -Bi pkg/arch/

fdtd-lucuma
```
