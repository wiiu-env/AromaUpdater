#pragma once

#include <cstdint>

int32_t LoadFileToMem(const char *filepath, uint8_t **inbuffer, uint32_t *size);
int32_t CreateSubfolder(const char *fullpath);
int32_t saveBufferToFile(const char *path, void *buffer, uint32_t size);