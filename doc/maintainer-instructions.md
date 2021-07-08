# Maintainer Instructions

<!-- update the table of contents with: doctoc --github maintainer-instructions.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Take screenshots for the website](#take-screenshots-for-the-website)
- [Update third party dependencies](#update-third-party-dependencies)
- [Update Linux sysroots](#update-linux-sysroots)
- [Make a release build](#make-a-release-build)
- [Run tests on all platforms](#run-tests-on-all-platforms)

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
1. Update the version in `build/scripts/depsDownload.sh`.
1. Commit as "Update foobar to version ____" or "Update foobar to commit ____".

## Update Linux sysroots

We keep prebuilt sysroots in the `tmbasic` bucket. These instructions will build new images, capturing updates in the base Alpine files.

1. Start the following machines. Prepare them for building using the instructions in the [Building from Source](https://github.com/electroly/tmbasic/blob/master/doc/building-from-source.md) document. Make sure they have AWSCLI installed and configured with access to write to the `tmbasic` S3 bucket.

    - **Small Linux x64 machine** &mdash; Ubuntu Linux 20.04 &mdash; x64
        - Specs: 1 vCPU + 1GB RAM + 10GB disk
        - Recommended: AWS `c5a.large`
    - **Small Linux ARM64 machine** &mdash; Ubuntu Linux 20.04 &mdash; ARM64, *excluding* Apple M1
        - Specs: 1 vCPU + 1GB RAM + 10GB disk
        - Recommended: AWS `c6g.medium`
        - Apple M1 is not supported because it cannot run ARM32 code.

1. On the ARM64 machine: `rm -rf tmbasic && git clone https://github.com/electroly/tmbasic.git && cd tmbasic/build/sysroots && ./buildArmSysroots.sh`

1. On the x64 machine: `rm -rf tmbasic && git clone https://github.com/electroly/tmbasic.git && cd tmbasic/build/sysroots && ./buildIntelSysroots.sh`

1. Edit the `build/scripts/sysrootDownload.sh` file to include the new filenames, and commit/push to git.

## Make a release build

1. If this is for a production release, then update the Linux sysroots using the instructions above.

1. Start the following machines. Prepare them for building using the instructions in the [Building from Source](https://github.com/electroly/tmbasic/blob/master/doc/building-from-source.md) document.

    - **Mac** &mdash; macOS 11.0 &mdash; x64 or ARM64
    - **Big Linux machine** &mdash; Ubuntu Linux 20.04 &mdash; x64 or ARM64, including Apple M1
        - Specs: 8 vCPU + 4GB RAM + 100GB disk
        - Recommended: Parallels VM on Apple M1, or AWS `c6g.2xlarge`

1. On Linux: `sudo apt-get install -y zip && docker system prune -a`.

1. On the Mac: `rm -rf mac-*`.

1. Make sure that the Linux machines are accessible from the Mac via `ssh` with public key authentication.

1. On the Mac, configure your bash session with the `ssh` connection details for the Linux build machine using the following commands.

    ```
    export BUILD_KEY=/path/to/ssh-key.pem
    export BUILD_USER=ubuntu
    export BUILD_HOST=hostname-or-ip
    ```

1. On the Mac, produce distribution-ready production builds: `cd build/publish && ./publish.sh`

    Output files will appear in the `dist` directory.

## Run tests on all platforms

1. Start the following machines. Prepare them for building using the instructions in the [Building from Source](https://github.com/electroly/tmbasic/blob/master/doc/building-from-source.md) document.

    - **Mac** &mdash; macOS 11.0 &mdash; ARM64
    - **Big Linux machine** &mdash; Ubuntu Linux 20.04 &mdash; x64 or ARM64, including Apple M1
        - Specs: 8 vCPU + 4GB RAM + 100GB disk
        - Recommended: Parallels VM on Apple M1, or AWS `c6g.2xlarge`
    - **Small Linux x64 machine** &mdash; Ubuntu Linux 20.04 &mdash; x64
        - Specs: 1 vCPU + 1GB RAM + 10GB disk
        - Recommended: AWS `c5a.large`
    - **Small Linux ARM64 machine** &mdash; Ubuntu Linux 20.04 &mdash; ARM64, *excluding* Apple M1
        - Specs: 1 vCPU + 1GB RAM + 10GB disk
        - Recommended: AWS `c6g.medium`
        - Apple M1 is not supported because it cannot run ARM32 code.

1. Make sure that the Linux machines are accessible from the Mac via `ssh` with public key authentication.

1. On the Mac, configure your bash session with the `ssh` connection details for the Linux machines using the following commands.

    ```
    export X64_KEY=/path/to/ssh-key.pem
    export X64_USER=ubuntu
    export X64_HOST=hostname-or-ip
    export ARM_KEY=/path/to/ssh-key.pem
    export ARM_USER=ubuntu
    export ARM_HOST=hostname-or-ip
    export BUILD_KEY=/path/to/ssh-key.pem
    export BUILD_USER=ubuntu
    export BUILD_HOST=hostname-or-ip
    ```

1. On the Mac: `cd build/test && ./test.sh`
