# Maintainer Instructions

<!-- update the table of contents with: doctoc --github maintainer-instructions.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Take screenshots for the website](#take-screenshots-for-the-website)
- [Update third party dependencies](#update-third-party-dependencies)
- [Make a release build](#make-a-release-build)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

___

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

1. Get the project link from [`doc/third-party-libraries.md`](https://github.com/electroly/tmbasic/blob/master/doc/third-party-libraries.md).
1. Download the latest version. If this is a GitHub/Gitlab "Download ZIP" link, make sure it points to a specific commit hash.
1. Upload to S3: `aws s3 cp ___ s3://tmbasic/___/ --acl public-read` (use downloaded filename)
1. Update the version in `build/files/deps.mk`.
1. Commit as "Update foobar to version ____" or "Update foobar to commit ____".

## Make a release build
1. Start the following three build machines. Prepare them for building using the instructions in the [Building from Source](https://github.com/electroly/tmbasic/blob/master/doc/building-from-source.md) document.

    - Ubuntu Linux &bull; ARM64 (AWS `c6g.2xlarge`)
    - Ubuntu Linux &bull; x64 (AWS `c5a.2xlarge`)
    - macOS 11.0 &bull; ARM64

1. Clear any existing dependencies on the three build machines so that we perform a fresh build using the latest versions.

    Linux: `docker system prune -a`

    Mac: `rm -rf mac-*`

1. Perform the rest of these instructions on the Mac. Make sure that the two Linux instances are accessible via `ssh` with public key authentication.

1. Copy the private keys (`.pem` files) onto the Mac. Set the permissions on the key files: `chmod 600 filename.pem`

1. Configure your bash session with the `ssh` connection details for the Linux ARM64 and x64 build machines using the following commands.

    ```
    export ARM_KEY=/path/to/arm64-ssh-key.pem
    export ARM_USER=ubuntu
    export ARM_HOST=arm64-hostname-or-ip
    export X64_KEY=/path/to/x64-ssh-key.pem
    export X64_USER=ubuntu
    export X64_HOST=x64-hostname-or-ip
    ```

1. Run all tests on all platforms: `pushd build/test && ./test.sh && popd`

1. Produce distribution-ready production builds: `pushd build/publish && ./publish.sh && popd`

    Output files will appear in the `dist` directory.