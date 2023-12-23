#include <stdint.h>
#include <stddef.h>

#include "i2c_cmd.h"
/*--------------------------------------------------------------------------*/
// Provides details on PD Controller boot flags and silicon revision.
#define REG_ADDR_BOOTFLAG 0x2D
#define REG_LEN_BOOTFLAG 12

#define REG_ADDR_PORTCONFIG 0x28
#define REG_LEN_PORTCONFIG 8

#define REG_ADDR_CMD1 0x08
#define REG_ADDR_Data1 0x09

#define REGION_0 0
#define REGION_1 1

#define MAX_BUF_BSIZE 64

#define DISABLE_PORT 0x3

#define RETURN_ON_ERROR(x)  \
    if (x < 0)              \
    {                       \
        printf("error \n"); \
        return -1;          \
    }

#define ERR_PRINT(x) printf("error: %d\n", x)

#define CONV_4CC_TO_WORD(_A_, _B_, _C_, _D_) ((_D_ << 24) | (_C_ << 16) | (_B_ << 8) | _A_)

#define nCMD CONV_4CC_TO_WORD('!', 'C', 'M', 'D')

#define FLrr 0
#define FLem 1
#define FLad 2
#define FLwd 3
#define FLvy 4

#define OUTPUT_LEN_FLRR 4
#define TASK_RET_CODE_LEN 1
#define PATCH_BUNDLE_SIZE 64
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t PatchHeaderErr : 1;     // Bit 0
    uint32_t Reserved1 : 1;          // Bit 1
    uint32_t DeadBatteryFlag : 1;    // Bit 2
    uint32_t SpiFlashPresent : 1;    // Bit 3
    uint32_t Region0 : 1;            // Bit 4
    uint32_t Region1 : 1;            // Bit 5
    uint32_t Region0Invalid : 1;     // Bit 6
    uint32_t Region1Invalid : 1;     // Bit 7
    uint32_t Region0FlashErr : 1;    // Bit 8
    uint32_t Region1FlashErr : 1;    // Bit 9
    uint32_t PatchDownloadErr : 1;   // Bit 10
    uint32_t Reserved2 : 1;          // Bit 11
    uint32_t Region0CrcFail : 1;     // Bit 12
    uint32_t Region1CrcFail : 1;     // Bit 13
    uint32_t CustomerOtpInvalid : 1; // Bit 14
    uint32_t reserved3 : 1;          // Bit 15
    uint32_t Reserved4 : 1;          // Bit 16
    uint32_t PP1Switch : 1;          // Bit 17
    uint32_t PP2Switch : 1;          // Bit 18
    uint32_t PP3Switch : 1;          // Bit 19
    uint32_t PP4Switch : 1;          // Bit 20
    uint32_t reserved5 : 11;         // Bit 31:21

    uint32_t REV_ID_Metal : 4; // Bit 3:0
    uint32_t REV_ID_Base : 4;  // Bit 7:4
    uint32_t reserved5 : 24;   // Bit 31:8

    uint32_t REV_ID_REG0 : 32; // Bit 31:0
    uint32_t REV_ID_REG1 : 32; // Bit 31:0
    uint32_t REV_ID_REG2 : 32; // Bit 31:0
    uint32_t REV_ID_REG3 : 32; // Bit 31:0
} s_TPS_bootflag;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t TypeCStateMachine : 2;     // Bit 1:0
    uint32_t Reserved1 : 1;             // Bit 2
    uint32_t ReceptacleType : 3;        // Bit 5:3
    uint32_t AudioAccessorySupport : 1; // Bit 6
    uint32_t DebugAccessorySupport : 1; // Bit 7
    uint32_t SupportTypeCOptions : 2;   // Bit 9:8
    uint32_t Reserved2 : 1;             // Bit 10
    uint32_t VCONNsupported : 2;        // Bit 12:11
    uint32_t USB3rate : 2;              // Bit 14:13
    uint32_t Reserved3 : 1;             // Bit 15
    ////////////////////////////////
    uint32_t VBUS_SetUvpTo4P5V : 1;            // Bit 0
    uint32_t VBUS_UvpTripPoint5V : 3;          // Bit 3:1
    uint32_t VBUS_UvpTripHV : 3;               // Bit 6:4
    uint32_t VBUS_OvpTripPoint : 6;            // Bit 12:7
    uint32_t VBUS_OvpUsage : 2;                // Bit 14:13
    uint32_t VBUS_HighVoltageWarningLevel : 1; // Bit 15
    uint32_t VBUS_LowVoltageWarningLevel : 1;  // Bit 16
    uint32_t SoftStart : 2;                    // Bit 18:17
    uint32_t Reserved4 : 1;                    // Bit 19
    uint32_t EnableUVPDebounce : 1;            // Bit 20
    uint32_t Reserved5 : 3;                    // Bit 23:21
    ////////////////////////////////
    uint32_t VoltageThresAsSinkContract : 8; // Bit 7:0
    uint32_t PowerThresAsSourceContract : 8; // Bit 7:0
    uint32_t Reserved6 : 8;                  // Bit 7:0
} s_TPS_portconfig;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t RegionNum : 1; // Bit 0
} s_TPS_flrr;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t flashaddr : 32;    // Bit 31:0
    uint32_t numof4ksector : 8; // Bit 7:0
} s_TPS_flem;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t flashaddr : 32; // Bit 31:0
} s_TPS_flad;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint32_t flashaddr : 32; // Bit 31:0
} s_TPS_flvy;
/*--------------------------------------------------------------------------*/
typedef struct
{
    uint8_t active_region;
    uint8_t inactive_region;
} s_AppContext;
s_AppContext gAppCtx;
/*--------------------------------------------------------------------------*/
static UpdateAndVerifyRegion(uint8_t region_number);
/*--------------------------------------------------------------------------*/
static int32_t PreOpsForFlashUpdate()
{
    s_AppContext *const pCtx = &gAppCtx;
    s_TPS_bootflag *p_bootflags = NULL;
    s_TPS_portconfig *p_portconfig = NULL;
    uint8_t outdata[MAX_BUF_BSIZE] = {0};
    uint8_t indata[MAX_BUF_BSIZE] = {0};
    // int32_t retVal = 0;
    int retVal = 0;
    /*
     * Read BootFlags (0x2D) register:
     * - Note #1: Applications shouldn't proceed w/ flash-update if the device's
     * boot didn't succeed
     * - Note #2: Flash-update shall be attempted on the inactive region first
     */
    retVal = i2c_read(REG_ADDR_BOOTFLAG, REG_LEN_BOOTFLAG, &outdata[0]);
    RETURN_ON_ERROR(retVal);

    /*
     * Note #1
     * Error during patch load - Don't attempt flash-update as the device wouldn't
     * be able to process the 4CC commands
     */
    p_bootflags = (s_TPS_bootflag *)&outdata[1];

    if (0 != p_bootflags->PatchHeaderErr)
    {
        ERR_PRINT(p_bootflags->PatchHeaderErr);
        // SignalEvent(APP_EVENT_ERROR);
        retVal = 0; /* For the state-machine */
        goto error;
    }

    /*
     * Note #2
     * Region1 = 0 indicates that device didn't attempt 'Region1',
     * which implicitly means that the content at Region0 is valid/active
     */
    if (0 == p_bootflags->Region1)
    {
        pCtx->active_region = REGION_0;
        pCtx->inactive_region = REGION_1;
    }
    else if ((1 == p_bootflags->Region1) &&
             (1 == p_bootflags->Region0) &&
             ((0 == p_bootflags->Region1CrcFail) &&
              (0 == p_bootflags->Region1FlashErr) &&
              (0 == p_bootflags->Region1Invalid)))
    {
        pCtx->active_region = REGION_1;
        pCtx->inactive_region = REGION_0;
    }
    /*
     * Keep the port disabled during the flash-update
     */
    retVal = i2c_read(REG_ADDR_PORTCONFIG, REG_LEN_PORTCONFIG, &outdata[0]);
    RETURN_ON_ERROR(retVal);
    memcpy(&indata[0], &outdata[1], REG_LEN_PORTCONFIG); /* outdata[0] holds the register length
                                                          */
    p_portconfig = (s_TPS_portconfig *)&indata[0];
    p_portconfig->TypeCStateMachine = DISABLE_PORT;
    retVal = i2c_write(REG_ADDR_PORTCONFIG, REG_LEN_PORTCONFIG, &indata[0]);
    RETURN_ON_ERROR(retVal);
error:
    return retVal;
}
/*--------------------------------------------------------------------------*/
static int32_t StartFlashUpdate()
{
    s_AppContext *const pCtx = &gAppCtx;
    int32_t retVal = 0;
    printf("\n\rActive Region is [%d] - Region being updated is [%d]\n\r",
           pCtx->active_region, pCtx->inactive_region);
    /*
     * Region-0 is currently active, hence update Region-1
     */
    retVal = UpdateAndVerifyRegion(pCtx->inactive_region);
    if (0 != retVal)
    {
        printf("Region[%d] update failed.! Next boot will happen from Region[%d]\n\r",
               pCtx->inactive_region, pCtx->active_region);
        retVal = 0;
        goto error;
    }
    /*
     * Region-1 is successfully updated.
     * To maintain a redundant copy for a fail-safe flash-update, copy the same
     * content at Region-0
     */
    retVal = UpdateAndVerifyRegion(pCtx->active_region);
    if (0 != retVal)
    {

        printf("Region[%d] update failed.! Next boot will happen from Region[%d]\n\r",
               pCtx->active_region, pCtx->inactive_region);
        retVal = 0;
        goto error;
    }
error:
    // SignalEvent(APP_EVENT_END_UPDATE);
    return retVal;
}
/*--------------------------------------------------------------------------*/
int ExecCmd(uint8_t cmd, uint8_t indata_size, uint8_t *indata, uint8_t outdata_size, uint8_t *outdata)
{
    int retVal;
    uint8_t fourCCcmd[4];
    uint32_t event = 0xFFFFFFFF;
    uint8_t rtnCMD[4];
    int i;

    retVal = i2c_write(REG_ADDR_Data1, indata_size, indata);
    RETURN_ON_ERROR(retVal);

    if (cmd == FLrr)
    {
        fourCCcmd[0] = 'F';
        fourCCcmd[1] = 'L';
        fourCCcmd[2] = 'r';
        fourCCcmd[3] = 'r';
    }
    else if (cmd == FLem)
    {
        fourCCcmd[0] = 'F';
        fourCCcmd[1] = 'L';
        fourCCcmd[2] = 'e';
        fourCCcmd[3] = 'm';
    }
    else if (cmd == FLad)
    {
        fourCCcmd[0] = 'F';
        fourCCcmd[1] = 'L';
        fourCCcmd[2] = 'a';
        fourCCcmd[3] = 'd';
    }
    else if (cmd == FLwd)
    {
        fourCCcmd[0] = 'F';
        fourCCcmd[1] = 'L';
        fourCCcmd[2] = 'w';
        fourCCcmd[3] = 'd';
    }
    else if (cmd == FLvy)
    {
        fourCCcmd[0] = 'F';
        fourCCcmd[1] = 'L';
        fourCCcmd[2] = 'v';
        fourCCcmd[3] = 'y';
    }
    retVal = i2c_write(REG_ADDR_CMD1, 4, fourCCcmd);
    RETURN_ON_ERROR(retVal);

    // Read Command Register
    do
    {
        event = 0;
        retVal = i2c_read(REG_ADDR_CMD1, 4, rtnCMD);
        if (retVal < 0)
        {
            return -1;
        }
        else
        {
            for (i = 0; i < 4; i++)
                event |= rtnCMD[i] << (i * 8);
        }
    } while (!((event == 0) || (event == nCMD)));

    // if (cmd == FLrr){
    retVal = i2c_read(REG_ADDR_Data1, outdata_size, outdata);
    RETURN_ON_ERROR(retVal);
    //}

    return 0;
}
/*--------------------------------------------------------------------------*/
static UpdateAndVerifyRegion(uint8_t region_number)
{
    s_TPS_flrr flrrInData = {0};
    s_TPS_flem flemInData = {0};
    s_TPS_flad fladInData = {0};
    s_TPS_flvy flvyInData = {0};
    uint8_t outdata[MAX_BUF_BSIZE] = {0};
    uint32_t patchBundleSize = 0;
    uint32_t regAddr = 0;
    int32_t idx = -1;
    int32_t retVal = -1;
    patchBundleSize = sizeof(tps6598x_lowregion_array);
    /*
     * Get the location of the region 'region_number'
     */
    flrrInData.RegionNum = region_number;
    retVal = ExecCmd(FLrr, sizeof(flrrInData), (uint8_t *)&flrrInData,
                     OUTPUT_LEN_FLRR, &outdata[0]);
    RETURN_ON_ERROR(retVal);
    regAddr = (outdata[4] << 24) | (outdata[3] << 16) |
              (outdata[2] << 8) | (outdata[1] << 0);
    /*
     * Erase #'numof4ksector' sectors at address 'regAddr' of the sFLASH
     * - Note: The below snippet assumes that the total number of 4kB segments
     * required to hold the maximum size of the patch-bundle is 4.
     * Ensure its validity for the TPS6598x being used for your
     * application.
     */
    flemInData.flashaddr = regAddr;
    flemInData.numof4ksector = TOTAL_4kBSECTORS_FOR_PATCH;
    retVal = ExecCmd(FLem, sizeof(flemInData), (int8_t *)&flemInData,
                     TASK_RET_CODE_LEN, &outdata[0]);
    RETURN_ON_ERROR(retVal);

    /*
     * Set the start address for the next write
     */
    fladInData.flashaddr = regAddr;
    retVal = ExecCmd(FLad, sizeof(fladInData), (int8_t *)&fladInData,
                     TASK_RET_CODE_LEN, &outdata[0]);
    RETURN_ON_ERROR(retVal);
    /**/
    printf("Updating [%d] 4k chunks starting @ 0x%x \n\r", flemInData.numof4ksector, regAddr);
    for (idx = 0; idx < patchBundleSize / PATCH_BUNDLE_SIZE; idx++)
    {

        UART_PRINT(".");
        /*
         * Execute FLwd with PATCH_BUNDLE_SIZE bytes of patch-data
         * in each iteration
         */
        retVal = ExecCmd(FLwd, PATCH_BUNDLE_SIZE,
                         (int8_t *)&tps6598x_lowregion_array[idx * PATCH_BUNDLE_SIZE],
                         TASK_RET_CODE_LEN, &outdata[0]);
        RETURN_ON_ERROR(retVal);
        /*
         * 'outdata[1]' will contain the command's return code
         */
        if (0 != outdata[1])
        {
            printf("\n\r");
            printf("Flash Write FAILED.!\n\r");
            retVal = -1;
            goto error;
        }
    }
    printf("\n\r");
    /*
     * Write is through. Now verify if the content/copy is valid
     */
    flvyInData.flashaddr = regAddr;
    retVal = ExecCmd(FLvy, sizeof(flvyInData), (int8_t *)&flvyInData,
                     TASK_RET_CODE_LEN, &outdata[0]);
    if (0 != outdata[1])
    {
        printf("Flash Verify FAILED.!\n\r");
        retVal = -1;
        goto error;
    }
error:
    return retVal;
}
/*--------------------------------------------------------------------------*/
static int32_t ResetPDController()
{
    int32_t retVal = -1;
    uint8_t fourCCcmd[4];

    fourCCcmd[0] = 'G';
    fourCCcmd[1] = 'A';
    fourCCcmd[2] = 'I';
    fourCCcmd[3] = 'D';

    /*
     * Execute GAID, and wait for reset to complete
     */
    printf("Waiting for device to reset\n\r");
    // ExecCmd(GAID, 0, NULL, 0, NULL);
    retVal = i2c_write(REG_ADDR_CMD1, 4, fourCCcmd);
    RETURN_ON_ERROR(retVal);

    sleep(1);

    // Board_IF_Delay(1000);
    // retVal = ReadMode();
    // RETURN_ON_ERROR(retVal);
    // retVal = ReadVersion();
    // RETURN_ON_ERROR(retVal);
    // retVal = ReadBootStatus();
    // RETURN_ON_ERROR(retVal);
    return 0;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
