#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"

/* Entry point */
int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    InitOSFunctionPointers();
    InitSocketFunctionPointers();

    log_init("192.168.178.3");
    log_print("Starting launcher\n");

    InitFSFunctionPointers();
    InitVPadFunctionPointers();

    log_print("Function exports loaded\n");

    //!*******************************************************************
    //!                    Initialize heap memory                        *
    //!*******************************************************************
    log_print("Initialize memory management\n");
    //! We don't need bucket and MEM1 memory so no need to initialize
    memoryInitialize();

    //!*******************************************************************
    //!                        Initialize FS                             *
    //!*******************************************************************
    log_printf("Mount SD partition\n");
    mount_sd_fat("sd");

    VPADInit();

    // Prepare screen
    int screen_buf0_size = 0;
    int screen_buf1_size = 0;

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    screen_buf1_size = OSScreenGetBufferSizeEx(1);

    unsigned char *screenBuffer = MEM1_alloc(screen_buf0_size + screen_buf1_size, 0x40);

    OSScreenSetBufferEx(0, screenBuffer);
    OSScreenSetBufferEx(1, (screenBuffer + screen_buf0_size));

    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);

    // Clear screens
    OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);

    // print to TV
    OSScreenPutFontEx(0, 0, 0, "Hello world on TV!!!");
    OSScreenPutFontEx(0, 0, 1, "Press HOME-Button to exit.");

    // print to DRC
    OSScreenPutFontEx(1, 0, 0, "Hello world on DRC!!!");
    OSScreenPutFontEx(1, 0, 1, "Press HOME-Button to exit.");

    // Flush the cache
    DCFlushRange(screenBuffer, screen_buf0_size);
    DCFlushRange((screenBuffer + screen_buf0_size), screen_buf1_size);

    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    s32 vpadError = -1;
    VPADData vpad;

    while(1)
    {
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0 && ((vpad.btns_d | vpad.btns_h) & VPAD_BUTTON_HOME))
            break;

		usleep(50000);
    }

	MEM1_free(screenBuffer);
	screenBuffer = NULL;

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************

    log_printf("Unmount SD\n");
    unmount_sd_fat("sd");
    log_printf("Release memory\n");
    memoryRelease();
    log_deinit();

    return EXIT_SUCCESS;
}

