#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include "i2c/smbus.h"
/*--------------------------------------------------------------------------*/
#define I2C_BUS "/dev/i2c-4"
#define TPS_I2C_ADDR 0x22
/*--------------------------------------------------------------------------*/
int i2c_file;
/*--------------------------------------------------------------------------*/
int i2c_open(void)
{
	/* got these values from i2cdetect */
	const char *i2c_device = I2C_BUS;
	const int device_address = TPS_I2C_ADDR;

	/* open the i2c device file */
	i2c_file = open(i2c_device, O_RDWR);
	if (i2c_file < 0)
	{
		printf("Failed opening %s\n", i2c_device);
		return 1;
	}

	if (ioctl(i2c_file, I2C_SLAVE, device_address) < 0)
	{
		printf("Failed addressing device at %02X\n", device_address);
		close(i2c_file);
		return 1;
	}
}
/*--------------------------------------------------------------------------*/
int i2c_read(uint8_t addr)
{
	int res = i2c_smbus_read_byte_data(i2c_file, addr);

	if (res < 0)
	{
		printf("i2c_read error \n");
		return -1;	
	}
	
	return res;
}
/*--------------------------------------------------------------------------*/
