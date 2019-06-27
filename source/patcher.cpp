#include <cstdlib>
#include <cstring>
#include <iostream>

#include <malloc.h>
#include <network.h>

#include "config.hpp"
#include "nand.hpp"
#include "patcher.hpp"

using namespace std;

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

s32 getSystemMenuIOS(const s32 systemMenuVersion) {
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

    // laziness came all over me here uwu

    if ((systemMenuVersion >= 224 && systemMenuVersion <= 290) || (systemMenuVersion >= 352 && systemMenuVersion <= 354)) return 30;
    if (systemMenuVersion >= 384 && systemMenuVersion <= 386) return 50;
    if (systemMenuVersion >= 416 && systemMenuVersion <= 454) return 60;
    if (systemMenuVersion >= 480 && systemMenuVersion <= 486) return 70;
    if (systemMenuVersion >= 512 && systemMenuVersion <= 518) return 80;

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
        cout << "The nwc24msg.cfg file couldn't be read" << endl;
        return error;
    }

    memcpy(&fileUnionNWC24MSG, fileBufferNWC24MSG, 0x400);

    // Separate the file magic and checksum
    unsigned int oldChecksum = fileUnionNWC24MSG.structNWC24MSG.checksum;
    unsigned int calculatedChecksum = calcChecksum(fileUnionNWC24MSG.charNWC24MSG, 0x3FC);

    // Check the file magic and checksum
    if (strcmp(fileUnionNWC24MSG.structNWC24MSG.magic, "WcCf") != 0) {
        cout << "The file couldn't be verified" << endl;
        return 1;
    }
    if (oldChecksum != calculatedChecksum) {
        cout << "The checksum isn't corresponding" << endl;
        return 2;
    }

    // Get the friend code
    s64 fc = fileUnionNWC24MSG.structNWC24MSG.friendCode;
    if (fc < 0) {
        cout << "Invalid Friend Code: " << fc << endl;
        return 3;
    }

    s32 sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == INVALID_SOCKET) {
        cout << "Socket failure: " << sock << endl;
        return 4;
    }

    // Request for a passwd/mlchkid
    char request_text[1024];
    sprintf(request_text, "GET /cgi-bin/patcher.cgi\r\nHost: %s\r\nUser-Agent: %s\r\n\r\nmlid=w%016lli", BASE_HTTP_URL, USERAGENT, fc);

    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = *((unsigned long*)(net_gethostbyname(BASE_HTTP_URL)->h_addr_list[0]));

    if (net_connect(sock, (struct sockaddr*)&sain, sizeof(sain)) < 0) {
        cout << "Couldn't connect to server." << endl;
        return 5;
    }

    net_send(sock, request_text, strlen(request_text), 0);
    cout << "Data sent! Awaiting response...";

    char response[2048];
    net_recv(sock, response, strlen(response), 0);

    net_shutdown(sock, 0);
    net_close(sock);    

    int responseCode = RESPONSE_NOTINIT;
    char* responseMlchkid = "";
    char* responsePasswd = "";

    sscanf(response, "cd:%d", &responseCode);
    sscanf(response, "mlchkid:%s", responseMlchkid);
    sscanf(response, "passwd:%s", responsePasswd);

    // Check the response code
    switch (responseCode) {
    case RESPONSE_INVALID:
        cout << "Invalid friend code." << endl;
        return 1;
        break;
    case RESPONSE_AREGISTERED:
        cout << "Already registered." << endl;
        return RESPONSE_AREGISTERED;
        break;
    case RESPONSE_DB_ERROR:
        cout << "Server database error." << endl;
        return 1;
        break;
    case RESPONSE_OK:
        if (strcmp(responseMlchkid, "") == 0 || strcmp(responsePasswd, "") == 0) {
            // If it's empty, nothing we can do.
            return 1;
        } else {
            // Patch the nwc24msg.cfg file
            cout << "before: " << fileUnionNWC24MSG.structNWC24MSG.mailDomain << endl;
            patchNWC24MSG(&fileUnionNWC24MSG, responsePasswd, responseMlchkid);
            cout << "after: " << fileUnionNWC24MSG.structNWC24MSG.mailDomain << endl;

            error = NAND_WriteFile("/shared2/wc24/nwc24msg.cfg", fileUnionNWC24MSG.charNWC24MSG, 0x400, false);
            if (error < 0) {
                cout << "The nwc24msg.cfg file couldn't be updated." << endl;
                return error;
            }
            return 0;
            break;
        }
    default:
        cout << "Incomplete data. Check if the server is up.\nFeel free to send a developer the following content: \n" << response << endl;
        return 1;
        break;
    }

    return 0;
}
