# Developers

## Build for Linux and Windows
1. Install Docker and give your user account permission to run `docker`. The following command will prepare a fresh AWS Ubuntu instance for building TMBASIC.

    ```
    sudo apt-get update -y && \
    sudo apt-get upgrade -y && \
    sudo apt-get install -y docker.io && \
    sudo usermod -aG docker ubuntu && \
    sudo reboot
    ```

1. Start the build environment and build using the following commands:

    ```
    cd build
    ./dev.sh
    make
    ```

1. Type `exit` to leave the build environment.

This development build of TMBASIC will be unable to produce executables from BASIC programs because it does not contain the necessary builds of the interpreter for all platforms. See the "Release builds" instructions below to produce a TMBASIC build that can itself produce executables.

### Use Visual Studio Code for debugging in Linux
1. Install the `ms-vscode.cpptools` and `ms-vscode-remote.remote-containers` extensions in Visual Studio Code.
1. Start the container using the instructions above.
1. In Visual Studio Code, click "View" > "Command Palette..." and run the "Remote-Containers: Attach to Running Container..." command. Choose the `/tmbasic-dev` container.

## Build for macOS
1. Install Xcode (version 10 or higher).

1. Start the build environment and compile:

    ```
    cd build
    ./mac-x64.sh
    make
    ```

1. Type `exit` to leave the build environment.

## Release builds
To produce a fully-baked set of distribution files, follow the instructions in [`build/publish/README.md`](build/publish/README.md).
