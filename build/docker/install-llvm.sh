#!/bin/bash
set -euxo pipefail
cd /tmp 
wget -q https://apt.llvm.org/llvm.sh 
chmod +x llvm.sh 
./llvm.sh 10 
rm -f llvm.sh 
apt-get install -y \
    clang-format-10 \
    clang-tidy-10 \
    clang-tools-10 \
    clang-10 \
    libc++-10-dev \
    libc++1-10 \
    libc++abi-10-dev \
    libc++abi1-10 \
    libclang-10-dev \
    libclang1-10 \
    libomp-10-dev \
    libomp5-10 \
    lld-10 \
    lldb-10 \
    llvm-10-dev \
    llvm-10-runtime \
    llvm-10

update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-10 9999 
update-alternatives --install /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-10 9999 
update-alternatives --install /usr/bin/llvm-strip llvm-strip /usr/bin/llvm-strip-10 9999 
update-alternatives --install /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-10 9999 
update-alternatives --install /usr/bin/wasm-ld wasm-ld /usr/bin/wasm-ld-10 9999 
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-10 9999 
update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-10 9999 
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang-10 9999 
update-alternatives --install /usr/bin/cc cc /usr/bin/clang-10 9999 
update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-10 9999
