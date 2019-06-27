#ifndef NAND_HPP
#define NAND_HPP

#include <gccore.h>

s32 NAND_Init();
s32 NAND_Exit();

bool NAND_IsFilePresent(const char *filePath);
s32 NAND_ReadFile(const char *filePath, void *buffer, u32 bufferLength);
s32 NAND_WriteFile(const char *filePath, const void *buffer, u32 bufferLength, bool createFile);
s32 NAND_GetFileSize(const char *filePath, u32 *fileSize);

#endif
