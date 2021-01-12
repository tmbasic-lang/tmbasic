# How to publish a TMBASIC release

Start the following three machines. Prepare them for building per [DEVELOPERS.md](../../DEVELOPERS.md).

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

On the Mac, run the following commands from the same directory as this readme (`build/publish`). Fill in all the environment variables with the `ssh` connection details for the Ubuntu ARM64 and x64 build machines.

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
