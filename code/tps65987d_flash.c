#include <stdint.h>

// Provides details on PD Controller boot flags and silicon revision.
#define REG_ADDR_BOOTFLAG   0x2D
#define REG_LEN_BOOTFLAG    12

#define REGION_0    0
#define REGION_1    1

typedef struct {
    uint8_t a;
} s_AppContext;


static int32_t PreOpsForFlashUpdate()
{
    s_AppContext *const pCtx = &gAppCtx;
    s_TPS_bootflag *p_bootflags = NULL;
    s_TPS_portconfig *p_portconfig = NULL;
    uint8_t outdata[MAX_BUF_BSIZE] = {0};
    uint8_t indata[MAX_BUF_BSIZE] = {0};
    int32_t retVal = 0;
    /*
     * Read BootFlags (0x2D) register:
     * - Note #1: Applications shouldn't proceed w/ flash-update if the device's
     * boot didn't succeed
     * - Note #2: Flash-update shall be attempted on the inactive region first
     */
    retVal = ReadReg(REG_ADDR_BOOTFLAG, REG_LEN_BOOTFLAG, &outdata[0]);
    RETURN_ON_ERROR(retVal);
    /*
     * Note #1
     * Error during patch load - Don't attempt flash-update as the device wouldn't
     * be able to process the 4CC commands
     */
    p_bootflags = (s_TPS_bootflag *)&outdata[1];
    if (0 != p_bootflags->patchheadererr)
    {
        ERR_PRINT(p_bootflags->patchheadererr);
        SignalEvent(APP_EVENT_ERROR);
        retVal = 0; /* For the state-machine */
        goto error;
    }

    /*
     * Note #2
     * Region1 = 0 indicates that device didn't attempt 'Region1',
     * which implicitly means that the content at Region0 is valid/active
     */
    if (0 == p_bootflags->region1)
    {
        pCtx->active_region = REGION_0;
        pCtx->inactive_region = REGION_1;
    }
    else if ((1 == p_bootflags->region1) &&
             (1 == p_bootflags->region0) &&
             ((0 == p_bootflags->region1crcfail) &&
              (0 == p_bootflags->region1flasherr) &&
              (0 == p_bootflags->region1invalid)))
    {
        pCtx->active_region = REGION_1;
        pCtx->inactive_region = REGION_0;
    }
    /*
     * Keep the port disabled during the flash-update
     */
    retVal = ReadReg(REG_ADDR_PORTCONFIG, REG_LEN_PORTCONFIG, &outdata[0]);
    RETURN_ON_ERROR(retVal);
    memcpy(&indata[0], &outdata[1], REG_LEN_PORTCONFIG); /* outdata[0] holds the register length
                                                          */
    p_portconfig = (s_TPS_portconfig *)&indata[0];
    p_portconfig->typecstatemachine = DISABLE_PORT;
    retVal = WriteReg(REG_ADDR_PORTCONFIG, REG_LEN_PORTCONFIG, &indata[0]);
    RETURN_ON_ERROR(retVal);
error:
    return retVal;
}

/**/
static int32_t StartFlashUpdate()
{
    s_AppContext *const pCtx = &gAppCtx;
    int32_t retVal = 0;
    UART_PRINT("\n\rActive Region is [%d] - Region being updated is [%d]\n\r",
               pCtx->active_region, pCtx->inactive_region);
    /*
     * Region-0 is currently active, hence update Region-1
     */
    retVal = UpdateAndVerifyRegion(pCtx->inactive_region);
    if (0 != retVal)
    {
        UART_PRINT("Region[%d] update failed.! Next boot will happen from Region[%d]\n\r",
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

        UART_PRINT("Region[%d] update failed.! Next boot will happen from Region[%d]\n\r",
                   pCtx->active_region, pCtx->inactive_region);
        retVal = 0;
        goto error;
    }
error:
    SignalEvent(APP_EVENT_END_UPDATE);
    return retVal;
}

/**/
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
    flrrInData.regionnum = region_number;
    retVal = ExecCmd(FLrr, sizeof(flrrInData), (int8_t *)&flrrInData,
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
    UART_PRINT("Updating [%d] 4k chunks starting @ 0x%x \n\r", flemInData.numof4ksector, regAddr);
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
            UART_PRINT("\n\r");
            UART_PRINT("Flash Write FAILED.!\n\r");
            retVal = -1;
            goto error;
        }
    }
    UART_PRINT("\n\r");
    /*
     * Write is through. Now verify if the content/copy is valid
     */
    flvyInData.flashaddr = regAddr;
    retVal = ExecCmd(FLvy, sizeof(flvyInData), (int8_t *)&flvyInData,
                     TASK_RET_CODE_LEN, &outdata[0]);
    if (0 != outdata[1])
    {
        UART_PRINT("Flash Verify FAILED.!\n\r");
        retVal = -1;
        goto error;
    }
error:
    return retVal;
}

/**/
static int32_t ResetPDController()
{
    int32_t retVal = -1;
    /*
     * Execute GAID, and wait for reset to complete
     */
    UART_PRINT("Waiting for device to reset\n\r");
    ExecCmd(GAID, 0, NULL, 0, NULL);
    Board_IF_Delay(1000);
    retVal = ReadMode();
    RETURN_ON_ERROR(retVal);
    retVal = ReadVersion();
    RETURN_ON_ERROR(retVal);
    retVal = ReadBootStatus();
    RETURN_ON_ERROR(retVal);
    return 0;
}