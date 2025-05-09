.PHONY: all

all: build_ko

obj-m += simple_gpu.o

KDIR:=$(PWD)/3-party/linux

build_ko:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
