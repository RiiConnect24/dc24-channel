#ifndef DATA_HPP
#define DATA_HPP

#include <gccore.h>

class Data {
public:
    // Fetches the friend code of the user.
    static s64 FriendCode();

    // Returns the version of the used System Menu.
    static s32 SystemMenuVersion();

    // Returns the IOS used for the given System Menu version.
    static s32 SystemMenuIOS(const s32 systemMenuVersion);
};

#endif