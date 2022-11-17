K=kernel
U=user

OBJS = \
  $K/entry.o \
  $K/main.o \
  $K/uart.o \
  $K/printf.o \
  $K/proc.o \
  $K/spinlock.o \
  $K/string.o \
  $K/swtch.o \
  $K/console.o \
  $K/sleeplock.o \
  $K/file.o \
  $K/kalloc.o\
  $K/vm.o\
  $K/trap.o\
  $K/kernelvec.o\
  $K/tlbrefill.o\
  $K/merror.o\
  $K/apic.o\
  $K/extioi.o\
  $K/ramdisk.o\
  $K/bio.o\
  $K/log.o\
  $K/fs.o\
  $K/pipe.o\
  $K/exec.o\
  $K/syscall.o\
  $K/sysproc.o\
  $K/sysfile.o\
  $K/uservec.o

UNAME_M=$(shell uname -m)
ifeq ($(findstring loongarch64,$(UNAME_M)),loongarch64)
TOOLPREFIX ?= 
else
TOOLPREFIX = loongarch64-unknown-linux-gnu-
endif

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

ASFLAGS = -march=loongarch64 -mabi=lp64s
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -march=loongarch64 -mabi=lp64s
CFLAGS += -ffreestanding -fno-common -nostdlib
CFLAGS += -I. -fno-stack-protector
CFLAGS += -fno-pie -no-pie
LDFLAGS = -z max-page-size=4096

$K/kernel: $(OBJS) $K/kernel.ld $U/initcode
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS)
	$(OBJDUMP) -S $K/kernel > $K/kernel.asm
	$(OBJDUMP) -t $K/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kernel.sym

$U/initcode: $U/initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -Ikernel -c $U/initcode.S -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o
	$(OBJCOPY) -S -O binary $U/initcode.out $U/initcode
	$(OBJDUMP) -S $U/initcode.o > $U/initcode.asm

tags: $(OBJS) _init
	etags *.S *.c

ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

$U/usys.S : $U/usys.pl
	perl $U/usys.pl > $U/usys.S

$U/usys.o : $U/usys.S
	$(CC) $(CFLAGS) -c -o $U/usys.o $U/usys.S

$U/_forktest: $U/forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_forktest $U/forktest.o $U/ulib.o $U/usys.o
	$(OBJDUMP) -S $U/_forktest > $U/forktest.asm

SH_FLAGS = -O -fno-omit-frame-pointer -ggdb -MD -march=loongarch64 -mabi=lp64s -ffreestanding -fno-common -nostdlib -I. -fno-stack-protector -fno-pie -no-pie -c -o

$U/_sh: $U/sh.c $(ULIB)
	$(CC) $(SH_FLAGS) $U/sh.o $U/sh.c
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_sh $U/sh.o $(ULIB)
	$(OBJDUMP) -S $U/_sh > $U/sh.asm

mkfs/mkfs: mkfs/mkfs.c $K/fs.h $K/param.h
	gcc -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	$U/_cat\
	$U/_echo\
	$U/_forktest\
	$U/_grep\
	$U/_init\
	$U/_kill\
	$U/_ln\
	$U/_ls\
	$U/_mkdir\
	$U/_rm\
	$U/_sh\
	$U/_stressfs\
	$U/_usertests\
#	$U/_grind\
	$U/_wc\
	$U/_zombie\

fs.img: mkfs/mkfs README $(UPROGS)
	mkfs/mkfs fs.img README $(UPROGS)
	xxd -i fs.img > kernel/ramdisk.h

-include kernel/*.d user/*.d

all: fs.img $K/kernel 

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym \
	$U/initcode $U/initcode.out $K/kernel fs.img \
	mkfs/mkfs .gdbinit \
        $U/usys.S \
	$(UPROGS)




