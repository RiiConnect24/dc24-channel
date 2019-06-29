#ifndef PATCHER_HPP
#define PATCHER_HPP

#include <gccore.h>

// Different responses from the server.
typedef enum {
    // The response hasn't been received due to an error with the connection.
    RESPONSE_NOTINIT = 0,

    // Everything went okay.
    RESPONSE_OK = 100,

    // Invalid data has been sent.
    RESPONSE_INVALID = 610,

    // This ID has been registered already.
    RESPONSE_AREGISTERED = 211,

    // The Database sent an error.
    RESPONSE_DB_ERROR = 410,
} ServerResponseCode;

typedef union {
    // Defines a single nwc24msg.cfg container.
    struct NWC24MSG {
        // File magic, holds the "WcCf" code at start.
        char Magic[0x4];

        // Unknown data.
        char Unknown[0x4];

        // The user's friend code.
        s64 FriendCode;

        // ID used for generation of specific data.
        char IDGen[0x4];

        // ID used for registration of the console.
        char IDRegistration[0x4];

        // Used Mail domain for sending mails.
        char MailDomain[0x40];

        // Password necessary for authentication.
        char Passwd[0x20];

        // Mail Check ID.
        char Mlchkid[0x24];

        // All URLs in one array.
        char URLs[0x5][0x80];

        // Reserved data.
        char Reserved[0xDC];

        // If set 1, Title Booting will be possible.
        unsigned int titleBooting;

        // Checksum used for verification.
        unsigned int Checksum;
    } NWC24MSG;

    // Holds the configuration as a char.
    char Raw[0x400];
} UnionNWC24MSG;

// Defines the Content Map, defining different APP files for an application.
typedef struct {
    // The file's given name.
    char Filename[0x8];

    // The hash of the providen file.
    u8 filehash[0x14];
} ContentMapObject;

class Patcher {
public:
    // Calculates the new Checksum.
    static unsigned int Checksum(char *buffer, int length);

    // Patches the ncw24msg.cfg with new URLs, a new Mail Check ID and a new password.
    static void NWC24MSG(UnionNWC24MSG *unionFile, char mlchkid[0x24], char passwd[0x20]);

    // Patches mail. This should be called, instead of the other calls.
    static s32 Mail();

    // Patches the content map.
    static s32 ContentMap();

    // Patches the Hash of the used IOS.
    static s32 IOSHash();
};

#endif