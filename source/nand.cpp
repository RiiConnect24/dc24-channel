#include <cstdlib>
#include <cstring>

#include <malloc.h>

#include "nand.hpp"

static bool isinit;

s32 NAND::Init() {
    s32 error = ISFS_Initialize();

    if (error >= 0) isinit = true;
    else return error;

    return 0;
}

s32 NAND::Exit() {
    s32 error = ISFS_Deinitialize();
    if (error >= 0) isinit = false;
    else return error;
    return 0;
}

s32 NAND::ReadFile(const char* filePath, void* buffer, u32 bufferLength) {
    if (!isinit) return -1;

    if (!NAND::IsFilePresent(filePath)) return -1;

    s32 file = ISFS_Open(filePath, ISFS_OPEN_READ);
    if (file < 0) return file;

    u8* readBuffer = (u8*)memalign(32, bufferLength);

    s32 error = ISFS_Read(file, readBuffer, bufferLength);
    if (error < 0) {
        ISFS_Close(file);
        return error;
    }

    error = ISFS_Close(file);
    if (error < 0) return error;

    memcpy(buffer, readBuffer, bufferLength);

    return 0;
}

s32 NAND::WriteFile(const char* filePath, const void* buffer, u32 bufferLength, bool createFile) {
    if (!isinit) return -1;

    if (!NAND::IsFilePresent(filePath)) {
        if (!createFile) return -1;
        else ISFS_CreateFile(filePath, 0, 3, 3, 3);
    }

    s32 file = ISFS_Open(filePath, ISFS_OPEN_WRITE);
    if (file < 0) return file;

    s32 error = ISFS_Write(file, buffer, bufferLength);
    if (error < 0) {
        ISFS_Close(file);
        return error;
    }

    error = ISFS_Close(file);
    if (error < 0) return error;

    return 0;
}

bool NAND::IsFilePresent(const char* filePath) {
    if (!isinit) return false;

    s32 file = ISFS_Open(filePath, ISFS_OPEN_READ);

    if (file < 0) return false;
    else {
        ISFS_Close(file);
        return true;
    }
}

s32 NAND::GetFileSize(const char* filePath, u32* fileSize) {
    if (!isinit) return -1;

    s32 file = ISFS_Open(filePath, ISFS_OPEN_READ);
    if (file < 0) return file;

    fstats* fileStats = (fstats*)memalign(32, sizeof(fstats));
    s32 error = ISFS_GetFileStats(file, fileStats);
    if (error < 0) return error;

    error = ISFS_Close(file);
    if (error < 0) return error;

    *fileSize = fileStats->file_length;

    return 0;
}
