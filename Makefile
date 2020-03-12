NAME_KMODULE=ipcore_axi

PLATFORM:=arm
KERNEL_SRC_DIR=/home/andrei/projects/cpu/imx6q/linux/linux-4.14.71
XENOMAI_INSTALL_PATH = "/home/andrei/opt/xenomai/3.0.7_imx/"
CROSS_TOP_DIR:=/home/andrei/opt/toolchains/gcc-linaro-7.3.1
CROSS_COMPILE:=$(CROSS_TOP_DIR)/bin/arm-linux-gnueabihf-



NAME_KMODULE_FILE=$(NAME_KMODULE).ko

################################################################################

ifneq ($(KERNELRELEASE),)
################################################################################
# Build kernel module. Kbuild part of Makefile.

EXTRA_CFLAGS += -Wno-error=date-time
# Main kernel module that will be composed from object files:
obj-m += $(NAME_KMODULE).o

# Composite parts of a module (here list object files produced from c-files):
$(NAME_KMODULE)-objs := $(NAME_KMODULE)_controller.o
$(NAME_KMODULE)-objs += $(NAME_KMODULE)_stack.o

EXTRA_CFLAGS +=      \
	$(DEFLIST)       \
	$(IDIRS)         \
	$(CFLAGS_GLOBAL) \
	$(CFLAGS_CONFIG) \
	$(CFLAGS_DEBUG)  \
	-I$(ECATSLAVE_STACK_SRC_DIR)/include \
	-I$(ECATSLAVE_STACK_SRC_DIR)/include/platforms/linux_sdk_$(PLATFORM)

else

################################################################################
# Build kernel module. The rest part of the Makefile.

PWD=$(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC_DIR) M=$(PWD) ARCH=$(PLATFORM) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNEL_SRC_DIR) M=$(PWD) clean
	$(RM) Module.symvers
	$(RM) $($(NAME_KMODULE)-OBJ_FILES) *.d
	$(RM) Module.markers
	$(RM) modules.order

endif
