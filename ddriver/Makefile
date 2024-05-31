obj-m:=driver.o miscdriver.o
all:
	make -C "/lib/modules/6.5.0-35-generic/build" M=$(PWD) modules
clean:
	rm -rf *.mod *.ko *.order *.symvers *.mod.*
