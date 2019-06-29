#include <cstdlib>
#include <iostream>

#include <gccore.h>
#include <network.h>
#include <wiiuse/wpad.h>

#include "config.hpp"
#include "data.hpp"
#include "nand.hpp"
#include "patcher.hpp"

int main(int argc, char** argv) {
    VIDEO_Init();

    GXRModeObj* rmode = VIDEO_GetPreferredMode(NULL);
    void* xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    char localip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};

    std::cout << "\nRiiConnect24 - Mail Patcher " << VERSION << "\nMade by " << CREDITS << ".\nInitializing..." << std::endl;
    WPAD_Init();
    NAND::Init();
    
    s32 systemVersion = Data::SystemMenuVersion();
    if (systemVersion < 512) std::cout << "Your System Menu is too outdated.\nPlease download the latest update (4.3)." << std::endl;
    //else if (systemVersion == 610) cout << "Wii Mail does not work on a WiiU." << std::endl;
    else {
        std::cout << "Connecting to the Internet..." << std::endl;
        if (if_config(localip, netmask, gateway, true, 20) >= 0) std::cout << "Connected to the Internet." << std::endl;
        else {
            std::cout << "An error occurred! We could not connect to the Internet." << std::endl;
            goto exitPoint;
        }

        std::cout << "Patching in progress..." << std::endl;

        s32 error = Patcher::Mail();
        if (error == RESPONSE_AREGISTERED) std::cout << "You seem to be registered already.\nIf your previous registration failed, please\ncontact a developer at " << SUPPORT << "." << std::endl;
        else if (error != 0) std::cout << "An error occurred! Please send a screenshot of this error message\nto a developer or at " << SUPPORT << ".\nError code: " << error << std::endl;
        else std::cout << "Finished!" << std::endl;
    }

exitPoint:
    std::cout << "Please press HOME to quit." << std::endl;
    NAND::Exit();

    while (true) {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
    }

    return 0;
}