# How to publish a TMBASIC release

Start the following three machines. Prepare them for building per [DEVELOPERS.md](../../DEVELOPERS.md).

- Ubuntu 18.04 on ARM64 (AWS)
- Ubuntu 18.04 on x64 (AWS)
- macOS 10.13 on x64 (MacinCloud)

On the Linux machines, run the following command to clear all Docker images. This ensures that we build using the latest versions of our dependencies.

```
docker rmi $(docker images -a -q) --force
```

Make sure that the two Ubuntu instances are accessible via ssh with private key authentication. Copy the private keys (`.pem` files) onto the Mac.

On the Mac, run the following commands from the same directory as this readme. Fill in all the environment variables.

```
export ARM_KEY=/path/to/arm64-ssh-key.pem
export ARM_USER=ubuntu
export ARM_HOST=arm64-hostname-or-ip
export X64_KEY=/path/to/x64-ssh-key.pem
export X64_USER=ubuntu
export X64_HOST=x64-hostname-or-ip
./publish.sh
```
