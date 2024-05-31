#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <sys/ioctl.h>
#include "ioctl_cmd.h"
#define DEVICE_FILE "/dev/meradevice"
#define O_RDWR1 0x00000001
#define ACC_MODE(x) ("\004\002\006\006"[(x)])
int main() {
	printf("Test == %d\n",ACC_MODE(O_RDWR1));
	int retVal;
	char buffer[10];
	int fd = open(DEVICE_FILE, O_RDWR);
	perror("ioctl");
	unsigned long l;
	ioctl(fd, MSG_IOCTL_GET_LENGTH, &l);
	perror("ioctl");
	printf("l == %lu\n",l);
	close(fd);
	return 0;
}
