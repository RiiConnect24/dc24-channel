#include <gccore.h>
#include <malloc.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <unistd.h>

#include "nand.h"
#include "network.h"
#include "patcher.h"

#define textPos(x, y) printf("\x1b[%d;%dH", y, x)
#define htons(x) (x)

static void* xfb = NULL;
static GXRModeObj* rmode = NULL;

//---------------------------------------------------------------------------------
int main(int argc, char** argv) {
//---------------------------------------------------------------------------------

    VIDEO_Init();
    WPAD_Init();
    initNetwork();
    NAND_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

	char version_major = 1;
	char version_minor = 3;

	printf("\n:---------------------------------------------------------:\n");
	printf("  RiiConnect24 Mail Patcher - (C) Spotlight ");
	#ifdef COMMITHASH
		printf("v%u.%u-%s\n", version_major , version_minor , COMMITHASH);
	#else
		printf("v%u.%u\n", version_major, version_minor);
    #endif
		printf("  Compiled on %s at %s\n", __DATE__ , __TIME__);
	printf(":---------------------------------------------------------:\n\n");

	printf("Running...\n\n");

    if (isDolphin()) {
        printf(":---------------------------------------------------------------:\n"
               ": Dolphin is not supported!                                     :\n"
               ": This tool can only run on a real Nintendo Wii Console.        :\n"
               ":                                                               :\n"
               ": Exiting in 5 seconds...                                       :\n"
               ":---------------------------------------------------------------:\n");
        sleep(5);
        exit(0);
    } else if (CheckvWii()){
        printf(":---------------------------------------------------------------:\n"
               ": vWii Detected                                                 :\n"
               ": This tool will still patch your nwc24msg.cfg, but you will be :\n"
               ": Unable to fully utilize Wii mail                              :\n" 
               ":---------------------------------------------------------------:\n");
    }
    printf("\nPatching...\n\n");

    s32 systemVersion = getSystemMenuVersion();

    if (systemVersion < 256) {
        printf(
        "Your System Menu is outdated.\nPlease update to the latest version of the Wii system.");
    } else {
        if (systemVersion >= 256 && systemVersion < 512) {
            printf("RiiConnect24 works best on 4.3 (if you update, please repatch!)\nThe "
                   "installer will continue.\n");
        }
	s64 friendCode = getFriendCode();
        s32 error = patchMail();
        if (error == RESPONSE_AREGISTERED) {
          printf("\nIf your previous registration failed, please contact us using:\n- Discord: https://discord.gg/b4Y7jfD\n		Wait time: Short, send a Direct Message to a developer.\n- E-Mail: support@riiconnect24.net\n		Wait time: up to 24 hours, sometimes longer\n");
		  printf("\nIf you're repatching, contact us and provide this info:\n");
		  printf("\nYour friend code: w");
		  printf("%016llu\n", friendCode);
        } else if (error != 0) {
          printf("There was an error while patching.\nPlease make a screenshot of this error message and send it\nto a developer.\n");
		  printf("\nContact using:\n- Discord: https://discord.gg/b4Y7jfD\n		Wait time: Short, send a Direct Message to a developer.\n- E-Mail: support@riiconnect24.net\n		Wait time: up to 24 hours, sometimes longer\n");
		  printf("\nYour friend code: w");
		  printf("%016llu\n", friendCode);
        } else {
          printf("All done, all done!\nPress HOME to exit.\n");
        }
    }

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME) exit(0);

        VIDEO_WaitVSync();
    }

    NAND_Exit();

    return 0;
}
