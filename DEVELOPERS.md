# Developers

## Build for Linux and Windows
The official build environment runs in a Docker container. Make sure that Docker is installed and that your user has permission to run `docker`. The following command will prepare a fresh AWS Ubuntu instance for building TMBASIC.

```
sudo apt-get update -y && \
sudo apt-get upgrade -y && \
sudo apt-get install -y docker.io && \
sudo usermod -aG docker ubuntu && \
sudo reboot
```

If you want to build ARM64 binaries on an x64 system, install the following prerequisite packages. Skip this step if you only want to build binaries native to your platform.

```
sudo apt-get install -y qemu binfmt-support qemu-user-static
```

Start the build environment and build using the following commands:
- `cd build`
- `./linux-x64.sh` or `./linux-arm64.sh` or `./win-x64.sh`
- `make`

## Build for macOS
1. Install Xcode.
1. Start the build environment and compile:
    ```
    cd build
    ./mac-x64.sh
    make
    ```

## Use Visual Studio Code for debugging in Linux
1. Install the `ms-vscode.cpptools` and `ms-vscode-remote.remote-containers` extensions in Visual Studio Code.
1. Start the debugging container:
    ```
    cd build
    ./vscode.sh
    make clean
    make
    ```
1. In Visual Studio Code, click "View" > "Command Palette..." and run the "Remote-Containers: Attach to Running Container..." command. Choose the `/tmbasic-vscode` container.
