# Maintainer Instructions

<!-- update the table of contents with: doctoc --github maintainer-instructions.md -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Take screenshots for the website](#take-screenshots-for-the-website)
- [Update third party dependencies](#update-third-party-dependencies)

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
    - Font quality: Antialiased
- Window > Colours > ANSI Cyan: 58, 150, 221

Crop to the console area including the one pixel black outline. Post-process with:

```
pngcrush -brute -reduce -ow screenshot.png
```

## Update third party dependencies

1. In `build/`, run `scripts/depsCheck.sh`. Update `build/scripts/depsDownload.sh` and `build/files/mingw.sh`.
1. In `build/`, run `scripts/depsDownload.sh` to pull the latest version of each dep.
1. In `build/downloads/`, `rm sysroot-* ; aws s3 sync . s3://tmbasic/deps/ --acl public-read --size-only`
1. Commit as "Update deps".
1. Check for new Alpine releases. Search for `alpine:` to find the Dockerfiles to update. Commit as "Update Alpine".
