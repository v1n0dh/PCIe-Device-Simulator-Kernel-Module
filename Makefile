obj-m := pcie_sim.o       # Kernel object

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Default target: build the kernel module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean target: remove generated files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Install target: optional, insert module
install: all
	sudo insmod pcie_sim.ko

# Remove module
uninstall:
	sudo rmmod pcie_sim.ko

# Check module info
info:
	modinfo pcie_sim.ko
