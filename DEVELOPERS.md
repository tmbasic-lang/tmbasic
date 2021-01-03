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
1. In Visual Studio Code, click "View" > "Command Palette..." and run the "Remote-Containers: Attach to Running Container..." command.
1. Choose the `/tmbasic-dev` container.
1. Choose the `/code` directory.

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
    - Click "999 commits"
    - Click top commit hash
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
