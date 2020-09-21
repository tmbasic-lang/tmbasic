# TMBASIC

## Platforms
TMBASIC is tested on:
- Ubuntu 18.04 (x64, ARM64)
- Android 10 (ARM64) using [Termux](https://termux.com/)

## Build
The build scripts and instructions assume that the host machine is Ubuntu 18+. AWS instance types `c5a.xlarge` (x64) or `c6g.xlarge` (ARM64) are recommended for building. 

The build environment runs in a Docker container. Make sure that Docker is installed and that your user has permission to run `docker`. The following command will prepare a fresh AWS Ubuntu 18.04 instance for building TMBASIC.

```
sudo apt-get update -y && \
    sudo apt-get upgrade -y && \
    sudo apt-get install -y docker.io && \
    sudo usermod -aG docker ubuntu && \
    sudo reboot
```

Clone the TMBASIC git repository, then start the build environment and build using the following commands:
- `cd build`
- `./build-env-linux-x64.sh` or `./build-env-linux-arm64.sh`
- `make`
- `exit`

## Cross-compile ARM64
To build ARM64 binaries on an x64 host, install the following packages first and then perform the build steps above. Skip this step if you only want to build binaries native to your host platform. Note: cross-compilation is slow; consider using an ARM64 cloud instance instead.

```
sudo apt-get install qemu binfmt-support qemu-user-static
```

## Software license
TMBASIC is open source software. It is comprised of the following components:

Component | License
-- | --
tmbasic | [MIT license](LICENSE)
boost | [Boost Software License v1.0](ext/boost/LICENSE_1_0.txt)
[immer](https://github.com/arximboldi/immer) | [Boost Software License v1.0](ext/immer/LICENSE)
libstdc++ | [GPL v3](ext/gcc/GPL-3) with [GCC Runtime Library Exception v3.1](ext/gcc/copyright)
ncurses | [Ncurses License](ext/ncurses/COPYING)
[tvision](https://github.com/magiblot/tvision) | Turbo Vision | [Borland license](ext/tvision/COPYRIGHT) (original code), [MIT license](ext/tvision/COPYRIGHT) (Linux port)

## Documentation license
The TMBASIC documentation includes the following components:

Component | License
-- | --
tmbasic | [MIT license](LICENSE)
notoserif | [SIL OFL 1.1 License](ext/notoserif/OFL.txt)
opensans | [Apache License 2.0](ext/opensans/LICENSE.txt)
oxygenmono | [SIL OFL 1.1 License](ext/oxygenmono/OFL.txt)
