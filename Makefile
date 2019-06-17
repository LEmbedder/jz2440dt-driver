KERN_DIR = /work/platform-2440/js2440_dtb/kernel/linux-4.19-rc3

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= led_drv.o buttons.o

