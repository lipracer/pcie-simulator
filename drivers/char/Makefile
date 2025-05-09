.PHONY: all copy

all: copy

obj-m += hello_char.o
# hello_char-objs += hello_char_module.o
# ccflags-y=-I

KDIR:=$(PWD)/3-party/linux

build_ko:
	make -C $(KDIR) M=$(CURDIR) modules

copy: build_ko
	cp hello_char.ko $(PWD)/build/

clean:
	make -C $(KDIR) M=$(CURDIR) clean
