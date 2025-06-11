.PHONY: all

all: build_ko

obj-m += simple_gpu.o
simple_gpu-objs += simple_gpu_module.o msix_handlers.o 

ccflags-y=-I$(PWD)/devices/pcie/simple-gpu3

KDIR:=$(PWD)/3-party/linux

build_ko:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
