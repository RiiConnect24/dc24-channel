#include <cstring>
#include <iostream>

#include <network.h>

#include "config.hpp"
#include "nand.hpp"
#include "patcher.hpp"

unsigned int Patcher::Checksum(char* buffer, int length) {
    int totalChecksum = 0;
    for (int i = 0; i < length; i += 4) {
        int currentBytes;
        memcpy(&currentBytes, buffer + i, 4);

        totalChecksum += currentBytes;
    }

    return totalChecksum;
}

void Patcher::NWC24MSG(UnionNWC24MSG* unionFile, char mlchkid[0x24], char passwd[0x20]) {
    // Patch the used mail domain,
    strcpy(unionFile->NWC24MSG.MailDomain, MAIL);

    // and then patch the Mail check ID and password.
    strcpy(unionFile->NWC24MSG.Passwd, passwd);
    strcpy(unionFile->NWC24MSG.Mlchkid, mlchkid);

    // Also, we need to patch the old URLs with working ones. These are the engines.
    const char engines[0x5][0x80] = { "account", "check", "receive", "delete", "send" };
    for (int i = 0; i < 5; i++) {
        // Copy the new engine in...
        char formattedLink[0x80];
        sprintf(formattedLink, "http://%s/cgi-bin/%s.cgi", HTTP, engines[i]);

        // And write it into the buffer.
        strcpy(unionFile->NWC24MSG.URLs[i], formattedLink);
    }

    // This'll patch the title booting.
    unionFile->NWC24MSG.titleBooting = 1;

    // And we need a proper checksum.
    unionFile->NWC24MSG.Checksum = Patcher::Checksum(unionFile->Raw, 0x3FC);
}

s32 Patcher::Mail() {
    // Read the nwc24msg.cfg file...
    static char fileBufferNWC24MSG[0x400];
    UnionNWC24MSG fileUnionNWC24MSG;

    // ...into our buffer.
    s32 error = NAND::ReadFile("/shared2/wc24/nwc24msg.cfg", fileBufferNWC24MSG, 0x400);
    if (error < 0) {
        std::cout << "The nwc24msg.cfg file couldn't be read." << std::endl;
        return error;
    }

    memcpy(&fileUnionNWC24MSG, fileBufferNWC24MSG, 0x400);

    // Separate the file magic and checksum,
    unsigned int oldChecksum = fileUnionNWC24MSG.NWC24MSG.Checksum;
    unsigned int calculatedChecksum = Patcher::Checksum(fileUnionNWC24MSG.Raw, 0x3FC);

    // and then check the file magic and checksum.
    if (strcmp(fileUnionNWC24MSG.NWC24MSG.Magic, "WcCf") != 0) {
        std::cout << "The file couldn't be verified." << std::endl;
        return 1;
    } else if (oldChecksum != calculatedChecksum) {
        std::cout << "The checksum isn't corresponding." << std::endl;
        return 2;
    }

    // Fetch the friend code.
    s64 fc = fileUnionNWC24MSG.NWC24MSG.FriendCode;
    if (fc < 0) {
        std::cout << "Invalid Friend Code: " << fc << std::endl;
        return 3;
    }

    // For data transmission, we need a socket. A working one, of course.
    s32 sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == INVALID_SOCKET) {
        std::cout << "Socket failure: " << sock << std::endl;
        return 4;
    }

    // This is our request for a new Mail Check ID and Password.
    char request_text[1024];
    sprintf(request_text, "POST /cgi-bin/patcher.cgi HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n\r\nmlid=w%016lli", HTTP, USERAGENT, fc);

    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons(80);
    sain.sin_addr.s_addr = *((unsigned long*)(net_gethostbyname(HTTP)->h_addr_list[0]));

    std::cout << "Connecting to the server..." << std::endl;
    int result = net_connect(sock, (struct sockaddr*)&sain, sizeof(sain));
    if (result < 0) {
        std::cout << "Couldn't connect to server." << std::endl;
        return 5;
    }

    std::cout << "Connected to the server. Sending data..." << std::endl;
    net_send(sock, request_text, strlen(request_text), 0);
    std::cout << "Data sent. Awaiting response..." << std::endl;

    int bufferlen;
    char buffer[256];
    while((bufferlen = net_recv(sock, buffer, 255, 0)) != 0) if(bufferlen > 0)  buffer[bufferlen] = 0;
    std::cout << "Data received. Parsing..." << std::endl;

    net_shutdown(sock, 0);
    net_close(sock);    

    int responseCode = RESPONSE_NOTINIT; // cd
    char responseMlchkid[0x24]; // mlchkid
    char responsePasswd[0x20]; // passwd

    /*
        Well, it's pretty empty here. 
        You may wonder why - and I don't blame you.
        Currently, we're missing a library for parsing the HTTP response from the server.
        But, even after searching for a while, I didn't find any code that would work on a Wii.
        Yes, I even verified it, and everything I got was either an error, or an exception.
    */

    // Check the response code
    switch (responseCode) {
    case RESPONSE_INVALID:
        std::cout << "Invalid friend code." << std::endl;
        return 1;
        break;
    case RESPONSE_AREGISTERED:
        std::cout << "Already registered." << std::endl;
        return RESPONSE_AREGISTERED;
        break;
    case RESPONSE_DB_ERROR:
        std::cout << "Server database error." << std::endl;
        return 1;
        break;
    case RESPONSE_OK:
        if (strcmp(responseMlchkid, "") == 0 || strcmp(responsePasswd, "") == 0) {
            // If it's empty, there's nothing we can do.
            return 1;
        } else {
            // Patch the nwc24msg.cfg file...
            std::cout << "Before: " << fileUnionNWC24MSG.NWC24MSG.MailDomain << std::endl;
            Patcher::NWC24MSG(&fileUnionNWC24MSG, responsePasswd, responseMlchkid);
            std::cout << "After: " << fileUnionNWC24MSG.NWC24MSG.MailDomain << std::endl;

            // ...and write the new file to the NAND.
            error = NAND::WriteFile("/shared2/wc24/nwc24msg.cfg", fileUnionNWC24MSG.Raw, 0x400, false);
            if (error < 0) {
                std::cout << "The nwc24msg.cfg file couldn't be updated." << std::endl;
                return error;
            }

            return 0;
            break;
        }
    case RESPONSE_NOTINIT:
        std::cout << "Incomplete data. Check if the server is up.\nFeel free to send a developer the following content: (this may be a lot of data)\n" << buffer << std::endl;
        return 1;
        break;
    }

    return 0;
}
