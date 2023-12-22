#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "i2c_cmd.h"
/*--------------------------------------------------------------------------*/
/*
list i2cs
sudo i2cdetect -l

tps65987D is in i2c4 bus and with address 0x22
sudo i2cdetect -y -a -r 4

read register 0x00
sudo i2cget -f -y 4 0x22 0

*/
/*--------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
    int ret;
    uint8_t value;
    uint8_t data[10];

    printf("hello\n");

    ret = i2c_open();
    if(ret){
        printf("i2c open error\n");
        exit(1);
    }

    // ret = i2c_read(0, &value);
    // printf("i2c add 0:%02x\n",value);

    ret = i2c_read(1, 4, data);
    ret = i2c_read(3, 4, data);
    ret = i2c_read(4, 4, data);
}
/*--------------------------------------------------------------------------*/
