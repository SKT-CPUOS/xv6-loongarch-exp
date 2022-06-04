# xv6-loongarch

Porting the xv6 OS to the LoongArch. Inspired by MIT's xv6-riscv Edition, check https://github.com/mit-pdos/xv6-riscv for more details.

## Dependencies

* [LoongArch Toolchain](https://github.com/loongson/build-tools/releases/download/2021.12.21/loongarch64-clfs-2021-12-18-cross-tools-gcc-full.tar.xz)

## Run on qemu-system-loongarch64

```bash

make all
cd qemu-loongarch64-runenv
./run_loongach.sh -k ../kernel/kernel
```