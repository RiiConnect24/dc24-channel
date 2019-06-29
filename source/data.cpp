#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <malloc.h>

#include "nand.hpp"
#include "data.hpp"

s64 Data::FriendCode() {
    // Open the file containing the friend code
    static char buffer[32];
    s32 error = NAND::ReadFile("/shared2/wc24/nwc24msg.cfg", buffer, 32);
    if (error < 0) return error;

    // Copy the friend code (0x8 -> 0xF)
    s64 fc = 0;
    memcpy(&fc, buffer + 0x8, 0x8);

    return fc;
}

s32 Data::SystemMenuVersion() {
    // Get the system menu tmd
    s32 systemMenuVersion;
    u32 tmdSize;

    s32 error = __ES_Init();
    if (error < 0) return error;

    error = ES_GetStoredTMDSize(0x0000000100000002, &tmdSize);
    if (error < 0) return error;

    signed_blob* tmdContent = (signed_blob*)memalign(32, tmdSize);
    char tmdContentBuffer[tmdSize], titleVersionChar[5] = "";
    error = ES_GetStoredTMD(0x0000000100000002, tmdContent, tmdSize);
    if (error < 0) return error;

    memcpy(tmdContentBuffer, tmdContent, tmdSize);
    snprintf(titleVersionChar, 5, "%.2x%.2x", tmdContentBuffer[0x1DC], tmdContentBuffer[0x1DD]);
    systemMenuVersion = strtol(titleVersionChar, NULL, 16);

    error = __ES_Close();
    if (error < 0) return error;

    return systemMenuVersion;
}

s32 Data::SystemMenuIOS(const s32 systemMenuVersion) {
    switch (systemMenuVersion) {
        case 33:
            return 9;
        case 128:
        case 97:
        case 130:
        case 162:
            return 11;
        case 192:
        case 193:
        case 194:
            return 20;
        case 326:
            return 40;
        case 390:
            return 52;
        default:
            break;
    }

    // This featured by laziness, hope you enjoy.

    if ((systemMenuVersion >= 224 && systemMenuVersion <= 290) || (systemMenuVersion >= 352 && systemMenuVersion <= 354)) return 30;
    if (systemMenuVersion >= 384 && systemMenuVersion <= 386) return 50;
    if (systemMenuVersion >= 416 && systemMenuVersion <= 454) return 60;
    if (systemMenuVersion >= 480 && systemMenuVersion <= 486) return 70;
    if (systemMenuVersion >= 512 && systemMenuVersion <= 518) return 80;

    return -1;
}