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
// https://www.kernel.org/doc/html/v5.5/i2c/smbus-protocol.html
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
    messages[1].flags = I2C_M_RD; // Read operation
    messages[1].len = len + 1;
    messages[1].buf = buff;

    // Execute the read transaction
    struct i2c_rdwr_ioctl_data transaction;
    transaction.msgs = messages;
    transaction.nmsgs = 2;

    if (ioctl(i2c_file, I2C_RDWR, &transaction) < 0)
    {
        perror("Failed to perform read operation");
        return -1;
    }

    memcpy(data, buff + 1, len);

    return 0;
}
/*--------------------------------------------------------------------------*/
int i2c_write(uint8_t reg, uint8_t len, uint8_t *data)
{
    int res = i2c_smbus_write_block_data(i2c_file, reg, len, data);

    if (res < 0)
    {
        printf("i2c_write error \n");
        return -1;
    }

    return 0;
}
/*--------------------------------------------------------------------------*/
int i2c_write_64(uint8_t reg, uint8_t *data)
{
    uint8_t buff0[2];
    uint8_t buff1[32];
    uint8_t buff2[32];
    struct i2c_msg messages[3];

    memset(buff0, 0, sizeof(buff0));
    memset(buff1, 0, sizeof(buff1));
    memset(buff2, 0, sizeof(buff2));

    buff0[0] = reg;
    buff0[1] = 64;
    memcpy(buff1, data, 32);
    memcpy(buff2, data + 32, 32);

    messages[0].addr = I2C_ADDRESS;
    messages[0].flags = 0; // Write operation
    messages[0].len = 2;
    messages[0].buf = buff0;

    messages[1].addr = I2C_ADDRESS;
    messages[1].flags = 0; // Write operation without repeated start
    messages[1].len = 32;
    messages[1].buf = buff2;

    messages[2].addr = I2C_ADDRESS;
    messages[2].flags = 0; // Write operation without repeated start
    messages[2].len = 32;
    messages[2].buf = buff1;

    // Execute the read transaction
    struct i2c_rdwr_ioctl_data transaction;
    transaction.msgs = messages;
    transaction.nmsgs = 3;

    if (ioctl(i2c_file, I2C_RDWR, &transaction) < 0)
    {
        perror("Failed to perform read operation");
        return -1;
    }

    return 0;
}
/*--------------------------------------------------------------------------*/
int i2c_read_64(uint8_t reg, uint8_t *data)
{
    uint8_t buff0;
    uint8_t buff1[32];
    uint8_t buff2[32];
    struct i2c_msg messages[4];

    memset(data, 0, 64);
    memset(buff1, 0, 32);
    memset(buff2, 0, 32);


    messages[0].addr = I2C_ADDRESS;
    messages[0].flags = 0; // Write operation
    messages[0].len = 1;
    messages[0].buf = &reg;

    messages[1].addr = I2C_ADDRESS;
    messages[1].flags = I2C_M_RD; // Write operation
    messages[1].len = 1;
    messages[1].buf = &buff0;

    messages[2].addr = I2C_ADDRESS;
    messages[2].flags = I2C_M_RD; // Read operation
    messages[2].len = 32;
    messages[2].buf = buff1;

    messages[3].addr = I2C_ADDRESS;
    messages[3].flags = I2C_M_RD; // Read operation
    messages[3].len = 32;
    messages[3].buf = buff2;

    // Execute the read transaction
    struct i2c_rdwr_ioctl_data transaction;
    transaction.msgs = messages;
    transaction.nmsgs = 4;

    int ret = ioctl(i2c_file, I2C_RDWR, &transaction);
    if (ret < 0)
    {
        perror("Failed to perform read operation");
        return -1;
    }

    memcpy(data, buff1, 32);
    memcpy(data + 32, buff2, 32);

    return 0;
}
/*--------------------------------------------------------------------------*/