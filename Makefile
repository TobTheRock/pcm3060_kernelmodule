
#obj-m := test/helloworldmodule.o
#KDIR := $(PWD)/5.7.0-rc6-1-ARCH/build

#BUILD OPTIONS
BUILD_DIR := $(PWD)/build#
INCLUDE := include#
EXPORT := pkg#
SOURCE := src#
TEST := test#
TARGET := pcm3060_module#
TEST_TARGET := test_suite#
CPP := cpp#

ifdef HK_LINUX
DEVICETREE := $(PWD)/devicetree/hardkernel
DTS_FILE := $(DEVICETREE)/meson64_odroidn2.dts
else
DEVICETREE := $(PWD)/devicetree/official
DTS_FILE := $(DEVICETREE)/meson-g12b-odroid-n2.dts
endif

# LOCAL SETTINGS
#COMPILE_LOCAL := true# true or false
ifndef COMPILE_LOCAL
COMPILE_LOCAL := false# true or false
endif

PWD := $(shell pwd)#
KVERSION := 5.7-rc6#
KDIR_LOCAL_ARCHARM := $(PWD)/linux-$(KVERSION)#
KDIR_LOCAL_HK := $(PWD)/linux-hardkernel#

ifdef HK_LINUX
KDIR_LOCAL := $(KDIR_LOCAL_HK)#
else
KDIR_LOCAL := $(KDIR_LOCAL_ARCHARM)#
endif

KCONFIG := $(PWD)/config/kernelconfig#
KMAKEFILE := $(PWD)/config/kernelmakefile#
#Build with all cores#
NB_CORES_LOCAL := $(shell grep -c '^processor' /proc/cpuinfo)#
#DTC_LOCAL := $(KDIR_LOCAL)/scripts/dtc/dtc
DTC_LOCAL := dtc
#DTSI_ROOT := arch/arm64/boot/dts/amlogic/#
# REMOTE SETTINGS
IP_REMOTE := 192.168.178.30
USER_REMOTE := admin
BUILD_DIR_REMOTE := /home/admin/build_$(TARGET)
KDIR_REMOTE := /lib/modules/$$(uname -r)/build
SSH_CONFIG = $(PWD)/config/ssh
SSH := ssh  -t -F $(SSH_CONFIG) $(USER_REMOTE)@$(IP_REMOTE)
RSYNC := rsync -aL -e 'ssh -F $(SSH_CONFIG)'
DTC_REMOTE := dtc
BOOT_REMOTE := /boot

##
.PHONY = install_test install_dtb

## Build dirs

BUILD_DIR_DTB = $(BUILD_DIR)/dtb
BUILD_DIR_MODULE = $(BUILD_DIR)/module
BUILD_DIR_TEST = $(BUILD_DIR)/test


SOURCES=$(shell find $(SOURCE) -type f -name '*.c')
INCLUDES:=$(shell find $(SOURCE) -type f -name '*.h')
BUILD_SOURCES:=$(subst $(SOURCE),$(BUILD_DIR_MODULE),$(SOURCES))
BUILD_INCLUDES:=$(subst $(SOURCE),$(BUILD_DIR_MODULE),$(INCLUDES))
TEST_SOURCES=$(shell find $(TEST) -type f -name '*.c')
TEST_INCLUDES=$(shell find $(TEST) -type f -name '*.h')
BUILD_TEST_SOURCES=$(subst $(TEST),$(BUILD_DIR_TEST),$(TEST_SOURCES))
# BUILD_TEST_SOURCES+=$(BUILD_SOURCES)
BUILD_TEST_INCLUDES=$(subst $(TEST),$(BUILD_DIR_TEST),$(TEST_INCLUDES))
# BUILD_TEST_INCLUDES+=$(BUILD_INCLUDES)
DEVICETREE_SOURCES:=$(wildcard $(DEVICETREE)/*)
DT_NAME := $(basename $(notdir $(DTS_FILE)))


#create build dir if not existing

$(BUILD_DIR_MODULE)/$(INCLUDE):
	mkdir -p "$@"
	
$(BUILD_DIR_TEST)/$(INCLUDE):
	mkdir -p "$@"
	
$(BUILD_DIR_DTB):
	mkdir -p "$@"

$(BUILD_DIR_MODULE):
	mkdir -p "$@"
$(BUILD_DIR_TEST):
	mkdir -p "$@"

# $(BUILD_DIR)/$(INCLUDE)/%:
# 	mkdir -p "$@"

# $(BUILD_DIR_MODULE)/$(INCLUDE)/%: $(SOURCE)/%
# 	mkdir -p "$(@D)"
# 	ln -s $(PWD)/$< $@

# $(BUILD_DIR_TEST)/$(INCLUDE)/%: $(TEST)/%
# 	mkdir -p "$(@D)"
# 	ln -s $(PWD)/$< $@

$(BUILD_DIR_TEST)/%: $(TEST)/%
	mkdir -p "$(@D)"
	ln -s $(PWD)/$< $@

$(BUILD_DIR_MODULE)/%: $(SOURCE)/%
	mkdir -p "$(@D)"
	ln -s $(PWD)/$< $@

	# Generate a Makefile with the needed obj-m and mymodule-objs set
# $(BUILD_DIR)/Makefile:
	#$(BUILD_TEST_SOURCES)
	#$(subst $(BUILD_DIR)/,,$(BUILD_TEST_SOURCES))
	# printf "obj-m += $(TARGET).o\n$(TARGET)-objs := $(subst $(TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR)/,,$(BUILD_TEST_SOURCES))))" > $@
	#echo "EXTRA_CFLAGS=-I$(BUILD_DIR_REMOTE)/include" >> $@

$(BUILD_DIR_DTB)/%: $(DEVICETREE)/%
	ln -s $< $@

$(KDIR_LOCAL_ARCHARM):
	wget https://git.kernel.org/torvalds/t/linux-$(KVERSION).tar.gz
	tar -xzf linux-$(KVERSION).tar.gz
	rm linux-$(KVERSION).tar.gz

	cp -f $(KCONFIG) $(KDIR_LOCAL)/.config
	patch -p1 -N -d $(KDIR_LOCAL) -i ../kernel_archarm_57rc6_patch/0001-net-smsc95xx-Allow-mac-address-to-be-set-as-a-parame.patch
	patch -p1 -N -d $(KDIR_LOCAL) -i ../kernel_archarm_57rc6_patch/0002-arm64-dts-rockchip-disable-pwm0-on-rk3399-firefly.patch
	patch -p1 -N -d $(KDIR_LOCAL) -i ../kernel_archarm_57rc6_patch/0003-arm64-dts-rockchip-add-usb3-controller-node-for-RK33.patch
	patch -p1 -N -d $(KDIR_LOCAL) -i ../kernel_archarm_57rc6_patch/0004-arm64-dts-rockchip-enable-usb3-nodes-on-rk3328-rock6.patch

$(KDIR_LOCAL_HK):
	git clone https://github.com/hardkernel/linux.git $(KDIR_LOCAL_HK)
	cd $(KDIR_LOCAL_HK)
	git checkout odroidn2-4.9.y

ifeq ($(COMPILE_LOCAL),true)
#Crosscompile flags
export ARCH := arm64#
export CROSS_COMPILE := aarch64-linux-gnu-#
export PATH :=$(PWD)/toolchain/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/:$(PATH)#
#export CONFIG_LOCALVERSION_AUTO=n
export LOCALVERSION := #
# targets
.PHONY += module localkernel clean dtb install_module install_dtb install_test

module: localkernel $(BUILD_DIR_MODULE) $(BUILD_DIR_MODULE)/$(INCLUDE) $(BUILD_SOURCES) $(BUILD_INCLUDES)
	printf "obj-m += $(TARGET).o\n$(TARGET)-objs := $(subst $(TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_MODULE)/,,$(BUILD_SOURCES))))" > $(BUILD_DIR_MODULE)/Makefile
	$(MAKE) -C $(KDIR_LOCAL)  M=$(BUILD_DIR_MODULE) EXTRA_CFLAGS=-I$(BUILD_DIR_MODULE) modules

localkernel: $(KDIR_LOCAL)
	#$(MAKE) -C $(KDIR_LOCAL) defconfig
	#sed -i "s|CONFIG_PREEMPT.*|CONFIG_PREEMPT=n|" $(KDIR_LOCAL)/.config
	#TODO COPY correct config

	$(MAKE) -C $(KDIR_LOCAL) prepare
	#cp $(KMAKEFILE) $(KDIR_LOCAL)/Makefile
	#$(MAKE) -C $(KDIR_LOCAL) -j$(NB_CORES_LOCAL) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) oldconfig
	#sed -i "s|CONFIG_LOCALVERSION_AUTO=.*|CONFIG_LOCALVERSION_AUTO=n|" $(KDIR_LOCAL)/.config
	#$(MAKE) -C $(KDIR_LOCAL) -j$(NB_CORES_LOCAL)

	# unset LDFLAGS
	# $(MAKE) -C $(KDIR) ${MAKEFLAGS} Image Image.gz modules
	# $(MAKE) -C $(KDIR) ${MAKEFLAGS} DTC_FLAGS="-@" dtbs


test: $(BUILD_DIR_TEST) $(BUILD_TEST_SOURCES) $(BUILD_TEST_INCLUDES)  #$(BUILD_INCLUDES) #localkernel
	@printf '%s\n' \
			'CC=$(CROSS_COMPILE)gcc' \
			'CFLAGS=-I.' \
			'DEPS = $(subst $(TEST_TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_TEST)/,,$(BUILD_TEST_INCLUDES))))' \
			'OBJ = $(subst $(TEST_TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_TEST)/,,$(BUILD_TEST_SOURCES))))' \
			'%.o: %.c $$(DEPS)' > $(BUILD_DIR_TEST)/Makefile
	@printf 	'\t$$(CC) -c -o $$@ $$< $$(CFLAGS)\n' >> $(BUILD_DIR_TEST)/Makefile
	@printf	'$(TEST_TARGET): $$(OBJ)\n' >> $(BUILD_DIR_TEST)/Makefile
	@printf		'\t $$(CC) -o $$@ $$^ $$(CFLAGS)' >> $(BUILD_DIR_TEST)/Makefile
	# (TEST_TARGET)-objs := " > $(BUILD_DIR_TEST)/Makefile
	# $(MAKE) -C $(KDIR_LOCAL)  M=$(BUILD_DIR) EXTRA_CFLAGS=-I$(BUILD_DIR) modules
	$(MAKE) -C $(BUILD_DIR_TEST)

clean:
	$(MAKE) -C $(KDIR_LOCAL) M=$(PWD) clean
	rm -r $(BUILD_DIR)

dtb: $(BUILD_DIR_DTB) $(subst $(DEVICETREE),$(BUILD_DIR_DTB),$(DEVICETREE_SOURCES)) $(KDIR_LOCAL)
	$(CPP) -nostdinc -I $(KDIR_LOCAL)/include -I $(KDIR_LOCAL)/arch  -undef -x assembler-with-cpp  $(BUILD_DIR_DTB)/$(DT_NAME).dts $(BUILD_DIR_DTB)/$(DT_NAME)-preprocessed.dts
	$(DTC_LOCAL) -I dts -O dtb  $(BUILD_DIR_DTB)/$(DT_NAME)-preprocessed.dts -o $(BUILD_DIR_DTB)/$(DT_NAME).dtb
	# -iquote $(KDIR_LOCAL)/$(DTSI_ROOT)

install_module:
	$(RSYNC) -r $(BUILD_DIR_MODULE)/$(TARGET).ko $(USER_REMOTE)@$(IP_REMOTE):$(BUILD_DIR_REMOTE)
	$(SSH) -t 'sudo insmod $(BUILD_DIR_REMOTE)/$(TARGET).ko'

install_dtb:
	$(RSYNC) -r $(BUILD_DIR_DTB)/$(DT_NAME).dtb $(USER_REMOTE)@$(IP_REMOTE):$(BUILD_DIR_REMOTE)
	$(SSH) -t ' sudo cp $(BUILD_DIR_REMOTE)/$(DT_NAME).dtb $(BOOT_REMOTE)/$(DT_NAME).dtb &&\
		sudo sed -i "s|load mmc \$${devno}:1 \$${dtb_loadaddr} [^ ]*.dtb|load mmc \$${devno}:1 \$${dtb_loadaddr} /$(DT_NAME).dtb|" $(BOOT_REMOTE)/boot.ini &&\
		sudo reboot'
		
else
.PHONY += module clean test dtb  install_module install_dtb
module: $(BUILD_DIR_MODULE)  $(BUILD_DIR_MODULE)/$(INCLUDE) $(BUILD_SOURCES) $(BUILD_INCLUDES) #$(BUILD_INCLUDES)

	printf "obj-m += $(TARGET).o\n$(TARGET)-objs := $(subst $(TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_MODULE)/,,$(BUILD_SOURCES))))" > $(BUILD_DIR_MODULE)/Makefile
	$(RSYNC) -r $(BUILD_DIR)/ $(USER_REMOTE)@$(IP_REMOTE):$(BUILD_DIR_REMOTE) &&\
	$(SSH) 'cd $(BUILD_DIR_REMOTE)/module && $(MAKE) -C $(KDIR_REMOTE)  M=$(BUILD_DIR_REMOTE)/module EXTRA_CFLAGS=-I$(BUILD_DIR_REMOTE)/module modules'


clean:
	rm -r $(BUILD_DIR)
	$(SSH) 'rm -r $(BUILD_DIR_REMOTE)'

dtb:  $(BUILD_DIR_DTB) $(subst $(DEVICETREE),$(BUILD_DIR_DTB),$(DEVICETREE_SOURCES))
	$(RSYNC) -r $(BUILD_DIR)/ $(USER_REMOTE)@$(IP_REMOTE):$(BUILD_DIR_REMOTE)
	$(SSH) '$(CPP) -nostdinc -I $(KDIR_REMOTE)/include -I $(KDIR_REMOTE)/arch  -undef -x assembler-with-cpp $(BUILD_DIR_REMOTE)/dtb/$(DT_NAME).dts $(BUILD_DIR_REMOTE)/dtb/$(DT_NAME)-preprocessed.dts &&\
	$(DTC_REMOTE) -I dts -O dtb  $(BUILD_DIR_REMOTE)/dtb/$(DT_NAME)-preprocessed.dts -o $(BUILD_DIR_REMOTE)/dtb/$(DT_NAME).dtb'

test: $(BUILD_DIR_TEST) $(BUILD_TEST_SOURCES) $(BUILD_TEST_INCLUDES)  #$(BUILD_INCLUDES) #localkernel
	@printf '%s\n' \
			'CC=gcc' \
			'CFLAGS=-I.' \
			'DEPS = $(subst $(TEST_TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_TEST)/,,$(BUILD_TEST_INCLUDES))))' \
			'OBJ = $(subst $(TEST_TARGET).o,, $(subst .c,.o,$(subst $(BUILD_DIR_TEST)/,,$(BUILD_TEST_SOURCES))))' \
			'%.o: %.c $$(DEPS)' > $(BUILD_DIR_TEST)/Makefile
	@printf 	'\t$$(CC) -c -o $$@ $$< $$(CFLAGS)\n' >> $(BUILD_DIR_TEST)/Makefile
	@printf	'$(TEST_TARGET): $$(OBJ)\n' >> $(BUILD_DIR_TEST)/Makefile
	@printf		'\t $$(CC) -o $$@ $$^ $$(CFLAGS)' >> $(BUILD_DIR_TEST)/Makefile
	$(RSYNC) -r $(BUILD_DIR)/ $(USER_REMOTE)@$(IP_REMOTE):$(BUILD_DIR_REMOTE) &&\
	$(SSH) '$(MAKE) -C $(BUILD_DIR_REMOTE)/test'

install_module:
	$(SSH) 'sudo insmod $(BUILD_DIR_REMOTE)/module/$(TARGET).ko'

install_dtb:
	$(SSH) ' sudo cp $(BUILD_DIR_REMOTE)/$(DT_NAME).dtb $(BOOT_REMOTE)/$(DT_NAME).dtb &&\
		sudo sed -i "s|load mmc \$${devno}:1 \$${dtb_loadaddr} [^ ]*.dtb|load mmc \$${devno}:1 \$${dtb_loadaddr} /$(DT_NAME).dtb|" $(BOOT_REMOTE)/boot.ini &&\
		sudo reboot'

install_test:
	$(SSH) '$(BUILD_DIR_REMOTE)/test/$(TEST_TARGET)'
endif




