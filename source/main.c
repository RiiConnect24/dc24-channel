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
        char version_patch = 3;

	printf("\n:---------------------------------------------------------:\n");
	printf("  RiiConnect24 Mail Patcher - (C) RiiConnect24 ");
	#ifdef COMMITHASH
		printf("v%u.%u.%u-%s\n", version_major, version_minor, version_patch, COMMITHASH);
	#else
		printf("v%u.%u.%u\n", version_major, version_minor, version_patch);
        #endif
        printf("  Compiled on %s at %s\n", __DATE__ , __TIME__);
	printf(":---------------------------------------------------------:\n\n");

	printf("Initializing... ");
    WPAD_Init();
    initNetwork();
	NAND_Init();
		printf("OK!\n");
	

    if (isDolphin()) {
        printf("\n:---------------------------------------------------------------:\n"
               ": Dolphin is not supported!                                     :\n"
               ": This tool can only run on a real Wii Console.                 :\n"
               ":                                                               :\n"
               ": Exiting in 10 seconds...                                       :\n"
               ":---------------------------------------------------------------:\n");
        sleep(10);
        exit(0);
    } else if (CheckvWii()){
        printf("\n:---------------------------------------------------------------:\n"
               ": vWii Detected                                                 :\n"
               ": This tool will still patch your nwc24msg.cfg, but you will be :\n"
               ": unable to fully utilize Wii Mail.                             :\n" 
               ":---------------------------------------------------------------:\n");
    }
    printf("\nPatching...\n\n");

    // s32 systemVersion = getSystemMenuVersion();

    s64 friendCode = getFriendCode();
    s32 error = patchMail();
    if (error == RESPONSE_AREGISTERED) {
        printf("You have already patched your Wii to use Wii Mail.\n");
	printf("In most cases, there is no need to run this patcher again.\n");
	printf("If you're having any sorts of problems, reinstalling RiiConnect24 is unnecessary\n");
	printf("and unlikely to fix issues.\n");
	printf("If you still need to have your Wii Number removed, please contact us.\n");
	printf("\nContact info:\n- Discord: https://discord.gg/rc24\n		Wait time: Short, send a Direct Message to a developer.\n- E-Mail: support@riiconnect24.net\n		Wait time: up to 24 hours, sometimes longer\n");
        printf("\nWhen contacting, please provide a brief explanation of the issue, and");
	printf("\ninclude your Wii Number: w");
        printf("%016llu\n", friendCode);
	printf("\nPress the HOME Button to exit.\n");
    } else if (error == -128) {
	printf("We're probably performing maintenance or having some issues. Hang tight!\n\nMake sure to check https://status.rc24.xyz/ for more info.\n\n");
	printf("\nContact info:\n- Discord: https://discord.gg/rc24\n		Wait time: Short, send a Direct Message to a developer.\n- E-Mail: support@riiconnect24.net\n		Wait time: up to 24 hours, sometimes longer\n");
	printf("\nPress the HOME Button to exit.\n");
    } else if (error != 0) {
        printf("There was an error while patching.\nPlease make a screenshot of this error message and contact us.\n");
        printf("\nPlease contact us using:\n- Discord: https://discord.gg/rc24\n		Wait time: Short, send a Direct Message to a developer.\n- E-Mail: support@riiconnect24.net\n		Wait time: up to 24 hours, sometimes longer\n");
        printf("\nWhen contacting, please provide a brief explanation of the issue, and");
	printf("\ninclude your Wii Number: w");
        printf("%016llu\n", friendCode);
	printf("\nPress the HOME Button to exit.\n");
    } else {
        printf("All done, all done! Thank you for installing RiiConnect24.\n");
        printf("\nPress the HOME Button to exit.\n");
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
