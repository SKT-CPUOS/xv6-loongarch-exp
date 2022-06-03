#!/bin/bash
make clean
make all
cd qemu-loongarch64-runenv
./run_loongach.sh -k ../kernel/kernel
