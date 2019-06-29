#ifndef NAND_HPP
#define NAND_HPP

#include <gccore.h>

// Allows modification of the console's NAND.
class NAND {
public:
    // Initializes the NAND.
    static s32 Init();

    // Deinitializes the NAND for the current application.
    static s32 Exit();

    // Checks if a file is present on the given path.
    static bool IsFilePresent(const char *filePath);

    // Reads the file into the buffer.
    static s32 ReadFile(const char *filePath, void *buffer, u32 bufferLength);

    // Writes the buffer into the file.
    static s32 WriteFile(const char *filePath, const void *buffer, u32 bufferLength, bool createFile);

    // Writes the file size into the given variable.
    static s32 GetFileSize(const char *filePath, u32 *fileSize);
};

#endif
