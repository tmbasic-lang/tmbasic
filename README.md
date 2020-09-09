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
- `./container-x64.sh` or `./container-arm64.sh`
- `make`
- `exit`

## Cross-compile ARM64
To build ARM64 binaries on an x64 host, install the following packages first and then perform the build steps above. Skip this step if you only want to build binaries native to your host platform. Note: cross-compilation is slow; consider using an ARM64 cloud instance instead.

```
sudo apt-get install qemu binfmt-support qemu-user-static
```

## License
TMBASIC is bound by the following licenses:

Component | License
-- | --
TMBASIC | [MIT license](LICENSE)
Turbo Vision - original code | [Borland license](ext/tvision/COPYRIGHT)
Turbo Vision - magiblot changes | [MIT license](ext/tvision/COPYRIGHT)
immer | [Boost Software License](ext/immer/LICENSE)
