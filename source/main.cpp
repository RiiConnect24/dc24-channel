#include <cstdlib>
#include <iostream>

#include <gccore.h>
#include <network.h>
#include <wiiuse/wpad.h>

#include "config.hpp"
#include "nand.hpp"
#include "patcher.hpp"

static void* xfb = NULL;
static GXRModeObj* rmode = NULL;

using namespace std;

int main(int argc, char** argv) {
	VIDEO_Init();
	WPAD_Init();
	NAND_Init();

	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

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


	cout << "\nRiiConnect24 - Mail Patcher " << VERSION << "\nMade by " << CREDITS << ".\nInitializing..." << endl;

	if (if_config(localip, netmask, gateway, true, 20) >= 0) cout << "Connected to the Internet." << endl;
	else {
		cout << "An error occurred! We could not connect to the Internet.\nPress HOME to quit." << endl;

		while (true) {
			VIDEO_WaitVSync();
			WPAD_ScanPads();
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(1);
		}
	}

	s32 systemVersion = getSystemMenuVersion();
	if (systemVersion < 512) cout << "Your System Menu is too outdated.\nPlease download the latest update (4.3)." << endl;
	else if (systemVersion == 610) cout << "Wii Mail does not work on a WiiU." << endl;
	else {
		cout << "Patching in progress..." << endl;

		s32 error = patchMail();
		if (error == RESPONSE_AREGISTERED) cout << "You seem to be registered already.\nIf your previous registration failed, please\ncontact a developer at " << SUPPORT << "." << endl;
		else if (error != 0) cout << "An error occurred! Please send a screenshot of this error message\nto a developer or at " << SUPPORT << ".\nError code: " << error << endl;
		else cout << "Finished!" << endl;
	}

	cout << "Please press HOME to quit." << endl;

	while (true) {
		VIDEO_WaitVSync();
		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
	}

	NAND_Exit();
	return 0;
}
