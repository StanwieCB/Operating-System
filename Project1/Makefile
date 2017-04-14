obj-m := ptree.o
KID := ~/kernel/goldfish
CORSS_COMPILE=arm-linux-androideabi-
CC=$(CORSS_COMPILE)gcc
LD=$(CORSS_COMPILE)ld

all:
	make -C $(KID) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(shell pwd) modules
clean:
	rm -rf *.ko *.o *.mod.c *.order *symvers