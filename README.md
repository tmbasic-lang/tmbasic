# TMBASIC

## Platforms

TMBASIC is tested on:
- Ubuntu 18.04 (x64, ARM64)
- Android 10 (ARM64) using [Termux](https://termux.com/)

## Build

The build scripts and instructions assume that the host machine is Ubuntu 18+.

To cross-compile ARM64 binaries on an x64 host, install the following packages first. Skip this step if you only want to build binaries native to your host platform.

```
sudo apt-get install qemu binfmt-support qemu-user-static
```

The build environment runs in a Docker container. Make sure that Docker is installed and that your user has permission to run `docker` (i.e. add your user to the `docker` group).
- `cd build`
- `./container-x64.sh` or `./container-arm64.sh`
- `make`
- `exit`

## License

TMBASIC is bound by the following licenses:

Component | License
-- | --
TMBASIC | [MIT license](LICENSE)
Turbo Vision - original code | [Borland license](ext/tvision/COPYRIGHT)
Turbo Vision - magiblot changes | [MIT license](ext/tvision/COPYRIGHT)
