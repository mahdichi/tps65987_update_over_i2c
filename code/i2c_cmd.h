#ifndef _I2C_CMD_H_
#define _I2C_CMD_H_
/*--------------------------------------------------------------------------*/
#include <stdint.h>
/*--------------------------------------------------------------------------*/
int i2c_open(void);
void i2c_close(void);

int i2c_read(uint8_t reg, uint8_t len, uint8_t* data);
int i2c_write(uint8_t reg, uint8_t len, uint8_t *data);

int i2c_write_64(uint8_t reg, uint8_t *data);
int i2c_read_64(uint8_t reg, uint8_t *data);
/*--------------------------------------------------------------------------*/
#endif /* _I2C_CMD_H_ */
