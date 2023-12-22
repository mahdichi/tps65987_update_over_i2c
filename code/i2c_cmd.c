#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include "i2c/smbus.h"
/*--------------------------------------------------------------------------*/
#define I2C_BUS "/dev/i2c-4"
#define I2C_ADDRESS 0x22
/*--------------------------------------------------------------------------*/
int i2c_file;
/*--------------------------------------------------------------------------*/
int i2c_open(void)
{
    /* got these values from i2cdetect */
    const char *i2c_device = I2C_BUS;
    const int device_address = I2C_ADDRESS;

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
void i2c_close(void)
{
    close(i2c_file);
}
/*--------------------------------------------------------------------------*/
// int i2c_read(uint8_t addr, uint8_t* value)
// {
// 	int res = i2c_smbus_read_byte_data(i2c_file, addr);

// 	if (res < 0)
// 	{
// 		printf("i2c_read error \n");
// 		return -1;
// 	}

// 	*value = res;
// 	return 0;
// }
/*--------------------------------------------------------------------------*/
int i2c_read(uint8_t reg, uint8_t len, uint8_t *data)
{
    uint8_t buff[65];
    struct i2c_msg messages[2];

    memset(data, 0, len + 1);

    messages[0].addr = I2C_ADDRESS;
    messages[0].flags = 0; // Write operation
    messages[0].len = 1;
    messages[0].buf = &reg;

    messages[1].addr = I2C_ADDRESS;
    messages[1].flags = I2C_M_RD;              // Read operation
    messages[1].len = len+1;
    messages[1].buf = buff;

    // Execute the read transaction
    struct i2c_rdwr_ioctl_data transaction;
    transaction.msgs = messages;
    transaction.nmsgs = 2;

    if (ioctl(i2c_file, I2C_RDWR, &transaction) < 0) {
        perror("Failed to perform read operation");
        return -1;
    }

    memcpy(data, buff1, 32);
    memcpy(data + 32, buff2, 32);

    return 0;
}
/*--------------------------------------------------------------------------*/