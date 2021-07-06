# Building from Source

<!-- update the table of contents with: doctoc --github building-from-source.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Build for Linux and Windows](#build-for-linux-and-windows)
- [Build for macOS](#build-for-macos)
- [Use Visual Studio Code for development and debugging](#use-visual-studio-code-for-development-and-debugging)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

___

## Build for Linux and Windows
Linux and Windows builds must be produced on a Linux build machine. Ubuntu Linux is recommended.

1. Install git and Docker. Give your user account permission to run `docker`.

    ```
    sudo apt-get update -y && \
    sudo apt-get upgrade -y && \
    sudo apt-get install -y git && \
    sudo curl -fsSL https://get.docker.com -o get-docker.sh && \
    sudo sh get-docker.sh && \
    sudo usermod -aG docker $(whoami) && \
    sudo reboot
    ```

1. Clone the git repository.

    ```
    git clone https://github.com/electroly/tmbasic.git
    cd tmbasic
    ```

1. Start the build environment and compile TMBASIC.

    ```
    cd build
    ./dev.sh
    make
    ```

    This will create a development build for Linux suitable for debugging.
    This build of TMBASIC will be unable to produce executables for other platforms because it does not contain the necessary builds of the interpreter.

    Use one of the `linux-*.sh` or `win-*.sh` scripts instead of `dev.sh` to produce a build for a particular target platform. The `linux-*.sh` scripts require [AWSCLI](https://aws.amazon.com/cli/) to be installed and configured in order to download Linux system root images from the `tmbasic` bucket.

1. Type `exit` to leave the build environment.

## Build for macOS
1. Install Xcode (version 12 or higher).

1. Start the build environment and compile:

    ```
    cd build
    ./mac-x64.sh
    make
    ```

    Use `./mac-arm64.sh` instead to build for Apple Silicon.

1. Type `exit` to leave the build environment.

## Use Visual Studio Code for development and debugging
1. Install the `ms-vscode.cpptools` and `ms-vscode-remote.remote-containers` extensions in Visual Studio Code.
1. Start the container using the instructions above.
1. In Visual Studio Code, click "View" > "Command Palette..." and run the "Remote-Containers: Attach to Running Container..." command.
1. Choose the `/tmbasic-dev` container.
1. Choose the `/code` directory.
