
.PHONY: hello-simulator busybox linux initramfs boot boot_gpu clean \
all_drivers all config_qemu qemu test_gpu0 pack_sys_file gen_sys_file

linux_src=$(PWD)/3-party/linux
busybox_src=$(PWD)/3-party/busybox

TEST_INDEX:=2

current_dir:=$(PWD)

all_drivers:
	make -C $(current_dir)/drivers

all: $(subdirs)

hello-simulator:
	@echo "current_dir:$(current_dir)"
	@echo "linux_src:$(linux_src)"
	@echo "busybox_src:$(busybox_src)"
	@echo "hello simulator"


# http://mgalgs.github.io/2015/05/16/how-to-build-a-custom-linux-kernel-for-qemu-2015-edition.html

INITRAMFS=initramfs-busybox-x86.cpio.gz

initramfs: $(INITRAMFS)

BUSYBOX_BIN:=$(busybox_src)/busybox

$(BUSYBOX_BIN):
	ln -f config/busybox.conf $(busybox_src)/.config
	cd $(busybox_src) ; patch -f -p1 < $(current_dir)/patchs/busybox.patch ; $(MAKE)

busybox: $(BUSYBOX_BIN)

gen_sys_file: busybox $(current_dir)/script/rcS
	mkdir -pv build/bin
	mkdir -pv build/sbin
	mkdir -pv build/etc
	mkdir -pv build/proc
	mkdir -pv build/sys
	mkdir -pv build/usr
	mkdir -pv build/usr/bin
	mkdir -pv build/usr/sbin
	mkdir -pv build/etc/init.d
	cp $(current_dir)/script/rcS build/etc/init.d/
	chmod a+x build/etc/init.d/rcS
	cd build/bin ; $(BUSYBOX_BIN) --install .

$(INITRAMFS): gen_sys_file
	cd build ; find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../$(INITRAMFS)

pack_sys_file: all_drivers initramfs
	rm -f build/*.ko
	mv $(current_dir)/drivers/pcie/simple-gpu$(TEST_INDEX)/simple_gpu.ko build
	cd build ; find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../$(INITRAMFS)

bzImage:=$(current_dir)/bzImage
$(bzImage): 
	ln -f config/linux.conf $(linux_src)/.config
	cd $(linux_src) ; yes '' | $(MAKE)
	cp $(linux_src)/arch/x86/boot/bzImage $(current_dir)

linux: $(bzImage)

qemu_dir:=$(current_dir)/3-party/qemu

qemu_exe=$(current_dir)/3-party/qemu/build/qemu-system-x86_64
qemu_build_dir=$(current_dir)/3-party/qemu/build
qemu_misc_builer=$(current_dir)/3-party/qemu/hw/misc/meson.build

add_device_srcs:=$(wildcard $(current_dir)/devices/*/*/*.c)
config_qemu=$(qemu_build_dir)/__qemu.config

$(config_qemu):
	cd $(qemu_build_dir) ; ../configure ; touch __qemu.config

# apply patch and rebuild need not reconfig
apply_patch_gpu:=$(qemu_dir)/hw/misc/simple-gpu$(TEST_INDEX)
$(apply_patch_gpu):
	ln -sf $(current_dir)/devices/pcie/simple-gpu$(TEST_INDEX)/ $(qemu_dir)/hw/misc/
	python3 script/sed.py 155 "subdir('simple-gpu$(TEST_INDEX)')" $(qemu_dir)/hw/misc/meson.build
	python3 script/sed.py 219 "source simple-gpu$(TEST_INDEX)/Kconfig" $(qemu_dir)/hw/misc/Kconfig

$(qemu_build_dir):
	cd $(current_dir)/3-party/qemu; mkdir -p build

qemu: |$(qemu_build_dir) $(apply_patch_gpu) $(config_qemu) $(add_device_srcs)
	cd $(qemu_build_dir) ; $(MAKE) -j8

boot: qemu pack_sys_file
	$(current_dir)/3-party/qemu/build/qemu-system-x86_64 -machine q35 \
	-machine kernel-irqchip=on -smp 2 \
	-kernel $(current_dir)/bzImage \
	-initrd $(current_dir)/initramfs-busybox-x86.cpio.gz -m 4096M -monitor telnet::4444,server,nowait \
	-nographic -append "rdinit=/init console=ttyS0" \
	-device sgpu,id=sgpu0,bus=pcie.0,addr=0x4 \
	-d guest_errors -D /tmp/qemu.log

# $(subdirs):
# 	make -C $@

clean: clean_busybox
	rm -rf build
	rm -f bzImage linux/arch/x86/boot/bzImage $(busybox_src)/busybox $(INITRAMFS)

clean_busybox:
	cd $(busybox_src); git checkout .; $(MAKE) clean

linux_makefile=$(current_dir)/3-party/linux
busybox_makefile=$(current_dir)/3-party/busybox
qemu_makefile=$(current_dir)/3-party/qemu

update_3party: update_3party_linux update_3party_busybox update_3party_qemu

update_3party_linux: linux_makefile
linux_makefile: 3-party/linux.tar.gz
	rm -rf 3-party/linux
	cd 3-party; tar -xzvf linux.tar.gz; mv linux-4.20 linux
3-party/linux.tar.gz:
	wget https://github.com/torvalds/linux/archive/refs/tags/v4.20.tar.gz -O 3-party/linux.tar.gz

update_3party_busybox: busybox_makefile
busybox_makefile: 3-party/busybox.tar.gz
	rm -rf 3-party/busybox
	cd 3-party; tar -xzvf busybox.tar.gz; mv busybox-1_29_3 busybox
3-party/busybox.tar.gz:
	wget https://github.com/mirror/busybox/archive/refs/tags/1_29_3.tar.gz -O 3-party/busybox.tar.gz

update_3party_qemu: qemu_makefile
qemu_makefile: 3-party/qemu.tar.gz
	rm -rf 3-party/qemu
	cd 3-party; tar -xzvf qemu.tar.gz; mv qemu-9.2.3 qemu
3-party/qemu.tar.gz:
	wget https://github.com/qemu/qemu/archive/refs/tags/v9.2.3.tar.gz -O 3-party/qemu.tar.gz

###############################################################################
# test
###############################################################################

test_char_dev:
	cd $(current_dir)/drivers/char; make test

test_gpu0: qemu pack
	make boot
