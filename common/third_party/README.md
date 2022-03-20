## Building the Linux Kernel
1. Check linux.config for what version of Linux to use.
2. Download it from https://www.kernel.org/pub/linux/kernel/ and extract it under this folder.  
   Alternatively, clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git and checkout the version.
3. Enter the folder and run:
```
cp ../linux.config .config
make -j <number-of-cores>
```

## Building Busybox
1. Check busybox.config for what version of Busybox to use.
2. Download it from https://busybox.net/downloads/ and extract it under this folder.  
   Alternatively, clone https://git.busybox.net/busybox and checkout the version.
3. Enter the folder and run:
```
cp ../busybox.config .config
make -j <number-of-cores>
```
