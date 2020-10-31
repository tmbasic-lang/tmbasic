# Building TMBASIC from source

## Linux and Windows
The build environment runs in a Docker container. Make sure that Docker is installed and that your user has permission to run `docker`. The following command will prepare a fresh AWS Ubuntu instance for building TMBASIC.

```
sudo apt-get update -y && \
    sudo apt-get upgrade -y && \
    sudo apt-get install -y docker.io && \
    sudo usermod -aG docker ubuntu && \
    sudo reboot
```

If you want to build ARM64 binaries on an x64 system, install the following prerequisite packages. Skip this step if you only want to build binaries native to your platform.

```
sudo apt-get install qemu binfmt-support qemu-user-static
```

Clone the TMBASIC git repository, then start the build environment and build using the following commands:
<table><tr><th>Linux (x64)</th><th>Linux (ARM64)</th><th>Windows (x64)</th></tr>
<tr><td><pre>cd build
./linux-x64.sh
make</pre></td><td><pre>cd build
./linux-arm64.sh
make</pre></td><td><pre>cd build
./win-x64.sh
make win</pre></td></tr></table>

## macOS
1. Install Xcode.
1. Install prerequisites: `pushd build && ./mac-x64.sh && popd`
1. Build TMBASIC: `make mac -j8`
