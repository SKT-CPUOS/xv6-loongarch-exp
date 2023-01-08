# xv6-loongarch的安装使用

xv6是 麻省理工的一个教学操作系统，是 Dennis Ritchie 和 Ken Thompson 的 Unix 的重新实现版本 6 (v6)。 被广泛应用于操作系统教学课程，现有x86和[RISC-V](https://github.com/mit-pdos/xv6-riscv)版本。其中x86版本已停止维护和更新。

LoongArch是由我国龙芯中科研发的自主指令系统（龙芯架构）。

本项目将xv6移植到LoongArch平台上，感谢张福新老师提供的初始版本，下面将介绍如何在Ubuntu 20.04中通过QEMU模拟器（在PC上模拟LoongArch硬件）编译xv6并运行调试。


## 配置交叉编译环境

为了能编译在LoongArch下运行的xv6内核，需要下载交叉编译工具链。
```sh
wget https://github.com/loongson/build-tools/releases/download/2022.05.29/loongarch64-clfs-5.0-cross-tools-gcc-full.tar.xz

sudo tar -vxf loongarch64-clfs-5.0-cross-tools-gcc-full.tar.xz -C /opt
```

配置交叉编译工具的环境变量
```sh
. setenv.sh
```

`setenv.sh`是一个用于设置环境变量的脚本文件
```sh
#!/bin/sh
set -x
CC_PREFIX=/opt/cross-tools

export PATH=$CC_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$CC_PREFIX/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/loongarch64-unknown-linux-gnu/lib/:$LD_LIBRARY_PATH

set +x
```

上述的命令只是临时设置环境变量，如需永久设置，可通过修改/etc/profile实现； 
通过命令检验是否设置成功
```sh
loongarch64-unknown-linux-gnu-gcc --version
```

看到如下提示则说明已正确设置
![](./imgs/Pasted%20image%2020220528100304.png)

## 编译xv6内核

下载xv6-loongarch
```
git clone --recurse-submodules https://github.com/SKT-CPUOS/xv6-loongarch-exp.git
cd xv6-loongarch-exp
```

开始编译
```
make all
```

终端会输出包括如下部分的编译信息
![](./imgs/Pasted%20image%2020220528101719.png)

当前路径下会生成`fs.img`文件，`\kernel`下也会生成所有链接时需要的\*.o, \*.d等文件，以及最终的`kernel`二进制文件
![](./imgs/Pasted%20image%2020220528102004.png)

## QEMU运行xv6-loongarch

```bash
cd qemu-loongarch-runenv
```

通过脚本文件`./run_loongarch.sh`的-k参数，指定我们编译好的xv6-loongarch内核，即可启动仿真运行
```bash
./run_loongarch.sh -k ../kernel/kernel
```

（如果提示有缺少的库，则按提示安装即可    
例如: 如果提示缺少spice-server   可以尝试： apt install -y libspice-server-dev    
      如果提示缺少libSDL2-2.0.so.0   可以尝试：sudo apt-get install -y libsdl2-2.0-0    
      如果提示缺少libfdt.so.1  可以尝试：sudo apt-get install libfdt-dev    
      如果提示缺少libusbredirparser.so.1  可以尝试：sudo apt-get install libusbredirparser-dev    
      如果提示缺少libfuse3.so.3 可以尝试：sudo apt-get install libfuse3-dev fuse3    
请根据自己系统的具体情况进行相应的完善)


启动后如下图所示，按下“Ctrl” + “A”组合键，松开后再按"X"来退出QEMU。

![](./imgs/Pasted%20image%2020220602102051.png)


# 调试观察

## GDB工具安装使用

安装一些必要依赖，根据编译时的报错情况自行安装所需依赖
```sh
sudo apt install texinfo bison flex libgmp-dev
```

编译gdb工具，大概需要等待几分钟的时间
```sh
git clone https://github.com/foxsen/binutils-gdb 
cd binutils-gdb 
git checkout loongarch-v2022-03-10 
mkdir build 
cd build 
../configure --target=loongarch64-unknown-linux-gnu --prefix=/opt/gdb 
make 
make install
```

查看自定义的安装路径下的文件，如下图所示
![](./imgs/Pasted%20image%2020220602095832.png)


## GDB调试观察

使用-D参数启动调试模式，实际上是执行调试服务器gdbserver的角色，默认监听TCP::1234端口，等待gdb客户端的接入。
```sh
./run_loongarch.sh -k ../kernel/kernel -D
```

![](./imgs/Pasted%20image%2020220602105307.png)

此时在另一个终端上启动gdb，并指定被调试对象(编译好的kernel文件路径)
```bash
/opt/gdb/bin/loongarch64-unknown-linux-gnu-gdb ../kernel/kernel
```

![](./imgs/Pasted%20image%2020220602105953.png)


在gdb命令提示符下执行`target remote :1234`命令连接到xv6目标系统上
```gdb
target remote :1234
```

![](./imgs/Pasted%20image%2020220602110048.png)

使用`b`命令在kernel/main.c的函数入口设置断点，并使用`c`命令继续执行xv6内核代码，直到碰到断点
![](./imgs/Pasted%20image%2020220602111325.png)

此时xv6的输出窗口中显示了相应的启动过程，但由于设置了断点，仍未到达shell的初始化。
![](./imgs/Pasted%20image%2020220602111602.png)

## GDB调试基本指令

### 运行命令

- r(run)
- c(continue)
- n(next)
- s(step)
- q(quit)

### 断点

- b(break n 

### 查看运行信息

- i(info) stack
- i register
- i thread
