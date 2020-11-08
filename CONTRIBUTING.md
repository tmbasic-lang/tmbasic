# Building TMBASIC from source

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

Clone the TMBASIC git repository, then start the build environment and build using the following commands:
<table><tr><th>Linux (x64)</th><th>Linux (ARM64)</th><th>Windows (x64)</th></tr>
<tr><td><pre>cd build
./linux-x64.sh
make</pre></td><td><pre>cd build
./linux-arm64.sh
make</pre></td><td><pre>cd build
./win-x64.sh
make win</pre></td></tr></table>

## Use Visual Studio Code for debugging
The C++ extension in Visual Studio Code can be used by installing TMBASIC's dependencies in the host machine. These instructions are for Ubuntu Linux 18.04. Run these commands from the `tmbasic` directory to install the libraries. Then use the Docker container configured in the previous section for building.

1. Install prerequisite packages:
    ```
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test && \
    sudo apt-get install -y build-essential libboost-dev libgtest-dev \
        libncurses5 libncurses5-dev libncursesw5 libncursesw5-dev cmake gcc-9 \
        g++-9 gpm libgpm-dev && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 \
        --slave /usr/bin/g++ g++ /usr/bin/g++-9
    ```
1. Build gtest:
    ```
    cd /usr/src/gtest && \
    sudo mkdir build && \
    cd build && \
    sudo cmake .. && \
    sudo make -j2 && \
    sudo make install
    ```
1. Install node.js:
    ```
    curl -sL https://deb.nodesource.com/setup_14.x | sudo bash - && \
    sudo apt-get install -y nodejs
    ```
1. Build and install mpdecimal:
    ```
    cp ext/mpdecimal/mpdecimal*.tar.gz /tmp && \
    pushd /tmp && \
    tar zxf mpdecimal-*.tar.gz && \
    cd mpdecimal-2.5.0 && \
    ./configure && \
    make -j2 && \
    sudo make install && \
    popd && \
    rm -rf /tmp/mpdecimal-2.5.0.tar.gz /tmp/mpdecimal-2.5.0
    ```

## Build for macOS
1. Install Xcode.
1. Install prerequisites:
    ```
    pushd build && ./mac-x64.sh && popd
    ```
1. Build TMBASIC:
    ```
    make mac -j8
    ```
