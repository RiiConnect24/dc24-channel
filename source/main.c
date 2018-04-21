#include <gccore.h>
#include <malloc.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>

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

    // The console understands VT terminal escape codes
    // This positions the cursor on row 2, column 0
    // we can use variables for this with format codes too
    // e.g. printf ("\x1b[%d;%dH", row, column );
    // textPos(2, 2);
    //
    // printf("===Network status===");
    //
    // if (initNetwork()) {
    // 	textPos(2, 4);
    // 	printf("Connected, yay!");
    //
    // 	char ipAddress[16] = "";
    //   char netmask[16] = "";
    //   char gateway[16] = "";
    //
    // 	s32 error = if_config(ipAddress, netmask, gateway, true, 50);
    //
    // 	textPos(2, 5);
    //
    // 	if (error >= 0) {
    // 		printf("IP: %s, NM: %s, GW: %s", ipAddress, netmask, gateway);
    //
    //   	textPos(0, 6);
    //
    //     char response[2048];
    //     getRequest("rc24.xyz", "/robots.txt", 443, &response, sizeof(response));
    //     printf("%s\n", response);
    // 	} else {
    // 		printf("An error has occured while trying to get the network data, wooooops :/");
    // 	}
    //
    // } else {
    // 	printf("Not connected :/");
    // }

    // textPos(24, 0);
    // printf("===NAND Access & Network Test===\n\n");

    // if (initNetwork()) {
    //   printf("Connected, making the request...\n");
    //
    //   char response[2048];
    //   if (getRequest("rc24.xyz", "/robots.txt", 443, &response, sizeof(response)) >= 0) {
    //     printf("Request successful !\n");
    //
    //     s32 file = ISFS_Open("/shared2/wc24/teste.txt", ISFS_OPEN_RW);
    //     printf("ISFS_Open: %li\n", file);
    //
    //     error = ISFS_Write(file, response, strlen(response));
    //     printf("ISFS_Write: %li\n", error);
    //
    //     error = ISFS_Close(file);
    //     printf("ISFS_Close: %li\n", error);
    //   } else {
    //     printf("Request error !\n");
    //   }
    // } else {
    //   printf("Couldn't connect to internet !\n");
    // }

    // static char buffer[1024] = "";
    // BYTE sha256[SHA256_BLOCK_SIZE];
    // NAND_ReadFile("/shared2/wc24/nwc24msg.cfg", buffer, 1024);
    // calcSHA256(sha256, buffer, 1024);
    //
    // for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
    //   printf("%.1x", sha256[i]);xdelta3

    // s64 fc = getFriendCode();
    // char fcChar[16];
    // sprintf(fcChar, "%lli", fc);
    // textPos(21, 14);
    // printf("Your Wii Number is ");
    // for (int i = 0; i < 4; i++) {
    //   char separate[5] = "";
    //   memcpy(&separate, fcChar + (4 * i), 0x4);
    //   printf("%s", separate);
    //
    //   if (i < 3)
    //     printf("-");
    // }
    // printf("\n");
    // textPos(23, 16);
    // printf("Press HOME/START to return to HSC.\n");

    // static u8 buffer[6] = "";
    // s32 error = NAND_ReadFile("/test.txt", buffer, 6);
    // printf("ReadFile1: %li\n", error);
    //
    //   for (int i = 0; i < sizeof(buffer); i++)
    //     printf("%x", buffer[i]);
    //
    //   printf("\n");
    //
    // static u8 patchf[41] = "";
    // error = NAND_ReadFile("/wfs/slt", patchf, 41);
    // printf("ReadFile2: %li\n", error);
    //
    // for (int i = 0; i < sizeof(patchf); i++)
    //   printf("%x", patchf[i]);
    //
    // printf("\n");
    //
    // uint8_t patchedOutput[6];
    // memset(patchedOutput, 0, 6);
    // usize_t gotOutput = 0;
    // // uint8_t diff[] = {0xD6, 0xC3, 0xC4, 0x00, 0x05, 0x02, 0x10, 0x74, 0x2E, 0x74, 0x78, 0x74,
    // 0x2F, 0x2F, 0x74, 0x65, 0x73, 0x74, 0x2E, 0x74, 0x78, 0x74, 0x2F, 0x04, 0x10, 0x06, 0x00,
    // 0x06, 0x01, 0x00, 0x08, 0x7A, 0x02, 0x28, 0x73, 0x61, 0x6C, 0x69, 0x74, 0x0A, 0x07};
    // // uint8_t origin[] = {0x73, 0x61, 0x6C, 0x75, 0x74, 0x0A};
    // // uint8_t changed[] = {0x73, 0x61, 0x6C, 0x69, 0x74, 0x0A};
    // uint8_t tst[64] = "";
    // int errInt = xd3_decode_memory(patchf, 39, buffer, 6, tst, &gotOutput, 64, 0);
    // // int errInt = xd3_encode_memory(buffer, 6, patchf, 6, tst, &gotOutput, 1024, 0);
    // printf("decode: %i\n", errInt);
    //
    // for (int i = 0; i < sizeof(tst); i++)
    //   printf("%.2x", tst[i]);

    printf("\n\nRunning...\n\n\n\n\n");

    s32 systemVersion = getSystemMenuVersion();

    if (systemVersion < 256) {
        printf(
        "Your System Menu is outdated.\nPlease update to the latest version of the Wii system.");
    } else {
        if (systemVersion >= 256 && systemVersion < 512) {
            printf("RiiConnect24 works best on 4.3 (if you update, please repatch!)\nThe "
                   "installer will continue.\n");
        }

        s32 error = patchMail();
        if (error == RESPONSE_AREGISTERED) {
          printf("If your previous registration failed, please\ncontact a developer at support@riiconnect24.net.\n");
        } else if (error != 0) {
          printf("An error occurred! Please send a screenshot of this error message\nto a developer or at support@riiconnect24.net.\n");
        } else {
          printf("Finished!\nPress HOME to exit.\n");
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
