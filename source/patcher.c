#include "patcher.h"
#include "config.h"
#include "nand.h"
#include "network.h"
#include "network/picohttpparser.h"
#include <malloc.h>

unsigned int calcChecksum(char* buffer, int length) {
    int totalChecksum = 0;
    for (int i = 0; i < length; i += 4) {
        int currentBytes;
        memcpy(&currentBytes, buffer + i, 4);

        totalChecksum += currentBytes;
    }

    return totalChecksum;
}

s64 getFriendCode() {
    // Open the file containing the friend code
    static char buffer[32];
    s32 error = NAND_ReadFile("/shared2/wc24/nwc24msg.cfg", buffer, 32);
    if (error < 0) return error;

    // Copy the friend code (0x8 -> 0xF)
    s64 fc = 0;
    memcpy(&fc, buffer + 0x8, 0x8);

    return fc;
}

s32 getSystemMenuVersion() {
    // Get the system menu tmd
    s32 systemMenuVersion;
    u32 tmdSize;
    s32 error = __ES_Init();
    if (error < 0) return error;

    error = ES_GetStoredTMDSize(0x0000000100000002, &tmdSize);
    if (error < 0) return error;

    signed_blob* tmdContent = memalign(32, tmdSize);
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

s32 getSystemMenuIOS(const s32 systemMenuVersion) {
    const s32 smv = systemMenuVersion;

    if (smv == 33)
        return 9;
    else if (smv == 128 || smv == 97 || smv == 130 || smv == 162)
        return 11;
    else if (smv >= 192 && smv <= 194)
        return 20;
    else if ((smv >= 224 && smv <= 290) || (smv >= 352 && smv <= 354))
        return 30;
    else if (smv == 326)
        return 40;
    else if (smv >= 384 && smv <= 386)
        return 50;
    else if (smv == 390)
        return 52;
    else if (smv >= 416 && smv <= 454)
        return 60;
    else if (smv >= 480 && smv <= 486)
        return 70;
    else if (smv >= 512 && smv <= 518)
        return 80;

    return -1;
}

void patchNWC24MSG(unionNWC24MSG* unionFile, char passwd[0x20], char mlchkid[0x24]) {
    // Patch mail domain
    strcpy(unionFile->structNWC24MSG.mailDomain, BASE_MAIL_URL);

    // Patch mlchkid and passwd
    strcpy(unionFile->structNWC24MSG.passwd, passwd);
    strcpy(unionFile->structNWC24MSG.mlchkid, mlchkid);

    // Patch the URLs
    const char engines[0x5][0x80] = { "account", "check", "receive", "delete", "send" };
    for (int i = 0; i < 5; i++) {
        char formattedLink[0x80] = "";
        sprintf(formattedLink, "http://%s/cgi-bin/%s.cgi", BASE_HTTP_URL, engines[i]);

        strcpy(unionFile->structNWC24MSG.urls[i], formattedLink);
    }

    // Patch the title booting
    unionFile->structNWC24MSG.titleBooting = 1;

    // Update the checksum
    int checksum = calcChecksum(unionFile->charNWC24MSG, 0x3FC);
    unionFile->structNWC24MSG.checksum = checksum;
}

s32 patchMail() {
    // Read the nwc24msg.cfg file
    static char fileBufferNWC24MSG[0x400] = "";
    unionNWC24MSG fileUnionNWC24MSG;

    s32 error = NAND_ReadFile("/shared2/wc24/nwc24msg.cfg", fileBufferNWC24MSG, 0x400);
    if (error < 0) {
        printf("The nwc24msg.cfg file couldn't be read\n");
        return error;
    }

    s32 copyingError = NAND_WriteFile("/shared2/wc24/nwc24msg.cbk", fileBufferNWC24MSG, 0x400, false);
    if (copyingError < 0) {
        printf("The nwc24msg.cfg file couldn't be backed up.\n");
        return copyingError;
    }
    memcpy(&fileUnionNWC24MSG, fileBufferNWC24MSG, 0x400);

    // Separate the file magic and checksum
    unsigned int oldChecksum = fileUnionNWC24MSG.structNWC24MSG.checksum;
    unsigned int calculatedChecksum = calcChecksum(fileUnionNWC24MSG.charNWC24MSG, 0x3FC);

    // Check the file magic and checksum
    if (strcmp(fileUnionNWC24MSG.structNWC24MSG.magic, "WcCf") != 0) {
        printf("The file couldn't be verified\n");
        return -1;
    }
    if (oldChecksum != calculatedChecksum) {
        printf("The checksum isn't corresponding\n");
        return -1;
    }

    // Get the friend code
    s64 fc = fileUnionNWC24MSG.structNWC24MSG.friendCode;
    if (fc < 0) {
        printf("Invalid Friend Code: %lli\n", fc);
        return fc;
    }

    // Request for a passwd/mlchkid
    char response[2048] = "";
    sprintf(response, "mlid=w%016lli", fc);
    error = postRequest(BASE_HTTP_URL, "/cgi-bin/patcher.cgi", 80, &response, sizeof(response));
    if (error < 0) {
        printf("Couldn't request the data: %li\n", error);
        return error;
    }

    // Parse the response
    struct phr_header headers[10];
    size_t num_headers;
    num_headers = sizeof(headers) / sizeof(headers[0]);
    error = phr_parse_headers(response, strlen(response) + 1, headers, &num_headers, 0);

    serverResponseCode responseCode = RESPONSE_NOTINIT;
    char responseMlchkid[0x24] = "";
    char responsePasswd[0x20] = "";

    for (int i = 0; i != num_headers; ++i) {
        char* currentHeaderName;
        currentHeaderName = malloc((int)headers[i].name_len);
        sprintf(currentHeaderName, "%.*s", (int)headers[i].name_len, headers[i].name);

        char* currentHeaderValue;
        currentHeaderValue = malloc((int)headers[i].value_len);
        sprintf(currentHeaderValue, "%.*s", (int)headers[i].value_len, headers[i].value);

        if (strcmp(currentHeaderName, "cd") == 0)
            responseCode = atoi(currentHeaderValue);
        else if (strcmp(currentHeaderName, "mlchkid") == 0)
            memcpy(&responseMlchkid, currentHeaderValue, 0x24);
        else if (strcmp(currentHeaderName, "passwd") == 0)
            memcpy(&responsePasswd, currentHeaderValue, 0x20);
    }

    // Check the response code
    switch (responseCode) {
    case RESPONSE_INVALID:
        printf("Invalid friend code\n");
        return 1;
        break;
    case RESPONSE_AREGISTERED:
        printf("Already registered\n");
        return RESPONSE_AREGISTERED;
        break;
    case RESPONSE_DB_ERROR:
        printf("Server database error.");
        return 1;
        break;
    case RESPONSE_OK:
        if (strcmp(responseMlchkid, "") == 0 || strcmp(responsePasswd, "") == 0) {
            // If it's empty, nothing we can do.
            return 1;
        } else {
            // Patch the nwc24msg.cfg file
            printf("before:%s\n", fileUnionNWC24MSG.structNWC24MSG.mailDomain);
            patchNWC24MSG(&fileUnionNWC24MSG, responsePasswd, responseMlchkid);
            printf("after:%s\n", fileUnionNWC24MSG.structNWC24MSG.mailDomain);

            error = NAND_WriteFile("/shared2/wc24/nwc24msg.cfg", fileUnionNWC24MSG.charNWC24MSG, 0x400, false);
            if (error < 0) {
                printf("The nwc24msg.cfg file couldn't be updated.\n");
                return error;
            }
            return 0;
            break;
        }
    default:
        printf("Incomplete data. Check if the server is up.\nFeel free to send a developer the "
               "following content: \n%s\n",
               response);
        return 1;
        break;
    }

    return 0;
}
