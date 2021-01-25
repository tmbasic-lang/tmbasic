# Developers

<!-- update the table of contents with: doctoc --github DEVELOPERS.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Build for Linux and Windows](#build-for-linux-and-windows)
  - [Use Visual Studio Code for debugging in Linux](#use-visual-studio-code-for-debugging-in-linux)
- [Build for macOS](#build-for-macos)
- [Take screenshots for the website](#take-screenshots-for-the-website)
- [Update third party dependencies](#update-third-party-dependencies)
  - [tvision](#tvision)
  - [nameof](#nameof)
- [Make a release build](#make-a-release-build)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

___

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

## Build for macOS
1. Install Xcode (version 10 or higher).

1. Start the build environment and compile:

    ```
    cd build
    ./mac-x64.sh
    make
    ```

1. Type `exit` to leave the build environment.

## Use Visual Studio Code for development and debugging
1. Install the `ms-vscode.cpptools` and `ms-vscode-remote.remote-containers` extensions in Visual Studio Code.
1. Start the container using the instructions above.
1. In Visual Studio Code, click "View" > "Command Palette..." and run the "Remote-Containers: Attach to Running Container..." command.
1. Choose the `/tmbasic-dev` container.
1. Choose the `/code` directory.

## Take screenshots for the website
SVG screenshots would have been nice, but they get garbled in some browsers (Chrome on Android). Instead, we will just take regular PNG screenshots.

- Windows 10 at 175% scaling
- PuTTY
- Terminal size: 80x24
- Window > Appearance
    - Cursor appearance: Underline
    - Font: Consolas 14pt
    - Font quality: Non-Antialiased
- Window > Colours > ANSI Cyan: 58, 150, 221

Crop to the console area including the one pixel black outline. Post-process with:

```
pngcrush -brute -reduce -ow screenshot.png
```

## Update third party dependencies

### tvision
1. https://github.com/magiblot/tvision
    - Click commit hash
    - Click "Browse files"
    - Click "Code"
    - Right-click "Download ZIP"
    - Click "Copy link"
    - Click "Download ZIP" to download it
1. Edit `ext/README.md` and replace the tvision link.
1. Upload to S3: `aws s3 cp tvision-____.zip s3://tmbasic/tvision/ --acl public-read` (use the downloaded filename)
1. Edit `ext/README.md` and replace the mirror link. Test the mirror link to make sure it works.
1. Update the commit hash in the following files:
    - `build/mac-x64.sh`
    - `build/files/Dockerfile.build-dev`
    - `build/files/Dockerfile.build-linux`
    - `build/files/Dockerfile.build-win`
1. Run the publish instructions in `build/publish/README.md` to ensure nothing broke.
1. Commit as "Update tvision to commit ____"

### nameof
1. https://github.com/Neargye/nameof/releases/
1. Copy the `nameof.hpp` link. Use `wget` to download, overwriting `ext/nameof/nameof.hpp`.
1. Update link in `ext/README.md`.
1. Commit as "Update nameof to version ____"

## Make a release build

Start the following three machines. Prepare them for building using the instructions at the beginning of this document.

- Ubuntu 18.04 on ARM64 (AWS `c6g.large`)
- Ubuntu 18.04 on x64 (AWS `c5a.large`)
- macOS 10.13 on x64 (MacinCloud PAYG)

On the Linux machines, run the following command to clear all Docker images. This ensures that we build using the latest versions of our dependencies.

```
docker container rm $(docker container ls -aq)
docker rmi $(docker images -a -q) --force
```

Make sure that the two Ubuntu instances are accessible via `ssh` with public key authentication. Copy the private keys (`.pem` files) onto the Mac. Set the permissions on the key files:

```
chmod 600 filename.pem
```

On the Mac, run the following commands from the `build/publish/` directory. Fill in all the environment variables with the `ssh` connection details for the Ubuntu ARM64 and x64 build machines.

```
export ARM_KEY=/path/to/arm64-ssh-key.pem
export ARM_USER=ubuntu
export ARM_HOST=arm64-hostname-or-ip
export X64_KEY=/path/to/x64-ssh-key.pem
export X64_USER=ubuntu
export X64_HOST=x64-hostname-or-ip
./publish.sh
```

Output files will appear in the `dist` directory.
