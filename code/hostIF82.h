#ifndef __HOSTIF82_H__
#define __HOSTIF82_H__
#include <stdbool.h>
#include <stdint.h>
//********************* Enable/Disable UART Stream **********
#define UART_Stream_ON
//***********************************************************
//********************* Global 82 I2C Constants *************
//***********************************************************
//***************** TPS65982 Slave Address ******************
#define DefAddr1 0x38 //7-bit I2C Slave address 0 for primary interface
#define DefAddr2 0x3F //7-bit I2C Slave address 1 for primary interface
//Max Arugment Length [First Byte in I2C Read/Write is Length Byte]
#define MAX_ARG_LENGTH 65
//********************* Register #'s ***********************
#define REG_CMD1 0x08 //used for the primary command interface.
#define REG_DATA1 0x09 //used for the primary command interface.
#define REG_VERSION 0x0F //FW VERSION ##.##.##.##
#define REG_CMD2 0x10 //used for the secondary command interface.
#define REG_DATA2 0x11 //used for the secondary command interface.
…
#define REG_SYS_CONFIG 0x28 //System Config
…
#define REG_BOOT_FLAGS 0x2D //Boot Flags
//********************* Register Lengths ***********************
…
#define lenCMD1 4 //used for the primary command interface.
#define lenDATA1 64 //used for the primary command interface.
#define lenVERSION 4 // FW VERSION ##.##.##.##
#define lenCMD2 4 //used for the secondary command interface.
#define lenDATA2 64 //used for the secondary command interface.
…
#define lenSYS_CONFIG 10 //System Config
…
#define lenBOOT_FLAGS 2 //Boot Flags
…
//********************* 4CC Words ***********************
// Convert 4CC to 32 bit word - Little Endian
#define CONV_4CC_TO_WORD(_A_, _B_, _C_, _D_) ((_D_ << 24) | (_C_ << 16) | (_B_ << 8) | _A_)
#define nCMD CONV_4CC_TO_WORD('!','C','M','D')
…
#define FLrr CONV_4CC_TO_WORD('F', 'L', 'r', 'r')
…
#define FLem CONV_4CC_TO_WORD('F', 'L', 'e', 'm')
#define FLad CONV_4CC_TO_WORD('F', 'L', 'a', 'd')
#define FLwd CONV_4CC_TO_WORD('F', 'L', 'w', 'd')
…
#define FLvy CONV_4CC_TO_WORD('F', 'L', 'v', 'y')
…
#define GAID CONV_4CC_TO_WORD('G', 'A', 'I', 'D')

//************ 82 Selected Registers Structures ***********************
//Read-ONLY Registers
//Static - Set in FW image, so can only change after successful FW update
typedef struct {
uint32_t FW_VERSION_B0 :8; //Byte 0 = 0x00 as of 04-12-2016
uint32_t FW_VERSION_B1 :8; //Byte 1 = Major Revision = 0x01 on 04-12-2016
uint32_t FW_VERSION_B2 :8; //Byte 2 = Minor Revision = 0x07 on 04-12-2016
uint32_t FW_VERSION_B3 :8; //Byte 3 = Bug Fix = 0x06 on 04-12-2016
} tFWVersion82;
//Dynamic - Set by BOOT Code based on conditions during Boot
typedef struct {
uint32_t BootOk :1; // Bit 0
uint32_t ExtPhvSwitch :1; // Bit 1
uint32_t DeadBatteryFlag :1; // Bit 2
uint32_t SpiFlashPresent :1; // Bit 3
uint32_t Region0 :1; // Bit 4
uint32_t Region1 :1; // Bit 5
uint32_t Region0Invalid :1; // Bit 6
uint32_t Region1Invalid :1; // Bit 7
uint32_t Region0FlashErr :1; // Bit 8
uint32_t Region1FlashErr :1; // Bit 9
uint32_t reserved1 :1; // Bit 10
uint32_t UartCrcFail :1; // Bit 11
uint32_t Region0CrcFail :1; // Bit 12
uint32_t Region1CrcFail :1; // Bit 13
uint32_t CustomerOtpInvalid :1; // Bit 14
uint32_t reserved2 :1; // Bit 15
} tBootFlags82;

//Read-Write Registers (excluding CMD1 & DATA1, which are special)
// Note: Writing to SysConfig Register causes a Disconnect on the Type-C Port
typedef struct {
uint32_t PortInfo :3;
uint32_t ReceptacleType :3;
uint32_t TypeCCurrent :2;
uint32_t VCONNsupported :2;
uint32_t :4;
uint32_t HighVoltageWarningLevel :1;
uint32_t LowVoltageWarningLevel :1;
uint32_t OvpTripPoint :6;
uint32_t OvpUsage :2;
uint32_t PP_5V0config :2;
uint32_t PP_HVconfig :2;
uint32_t PP_HVEconfig :3;
uint32_t :1;
uint32_t BC12enable :1;
uint32_t USBRPenable :1;
uint32_t USBEPenable :1;

uint32_t USB3rate :2;
uint32_t USB2Supported :1;
uint32_t AudioAccessorySupport :1;
uint32_t DebugAccessorySupport :1;
uint32_t PoweredAccessorySupport :1;
uint32_t RSense :1;
uint32_t TrySRCSupport :1;
uint32_t BillboardAllowed :1;
uint32_t :2;
uint32_t PP_EXT_OC_Timeout :5;
uint32_t ResetZTimeoutCount :6;
uint32_t ResetZTimeoutClock :2;
uint32_t Vout3V3SupThresh :3;
uint32_t Vout3V3enable :1;
uint32_t :1;
uint32_t :2;
uint32_t setUvpTo4P5V :1;
uint32_t UvpTripPoint5V :3;
uint32_t UvpUsageHV :3;
uint32_t :7;
} tSysConfig82;

#endif //__HOSTIF82_H__