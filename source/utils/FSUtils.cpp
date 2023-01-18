#include "FSUtils.h"
#include "logger.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <whb/log.h>

#define ROUNDDOWN(val, align) ((val) & ~(align - 1))
#define ROUNDUP(val, align)   ROUNDDOWN(((val) + (align - 1)), align)

int32_t LoadFileToMem(const char *filepath, uint8_t **inbuffer, uint32_t *size) {
    //! always initialze input
    *inbuffer = nullptr;
    if (size) {
        *size = 0;
    }

    int32_t iFd = open(filepath, O_RDONLY);
    if (iFd < 0) {
        return -1;
    }

    uint32_t filesize = lseek(iFd, 0, SEEK_END);
    lseek(iFd, 0, SEEK_SET);

    auto *buffer = (uint8_t *) memalign(0x40, ROUNDUP(filesize, 0x40));
    if (buffer == nullptr) {
        close(iFd);
        return -2;
    }

    uint32_t blocksize = 0x20000;
    uint32_t done      = 0;
    int32_t readBytes  = 0;

    while (done < filesize) {
        if (done + blocksize > filesize) {
            blocksize = filesize - done;
        }
        readBytes = read(iFd, buffer + done, blocksize);
        if (readBytes <= 0) {
            break;
        }
        done += readBytes;
    }

    close(iFd);

    if (done != filesize) {
        free(buffer);
        buffer = nullptr;
        return -3;
    }

    *inbuffer = buffer;

    //! size is optional input
    if (size) {
        *size = filesize;
    }

    return filesize;
}

int32_t CheckFile(const char *filepath) {
    if (!filepath)
        return 0;

    struct stat filestat {};

    char dirnoslash[strlen(filepath) + 2];
    snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

    while (dirnoslash[strlen(dirnoslash) - 1] == '/')
        dirnoslash[strlen(dirnoslash) - 1] = '\0';

    char *notRoot = strrchr(dirnoslash, '/');
    if (!notRoot) {
        strcat(dirnoslash, "/");
    }

    if (stat(dirnoslash, &filestat) == 0)
        return 1;

    return 0;
}

int32_t CreateSubfolder(const char *fullpath) {
    if (!fullpath)
        return 0;

    int32_t result = 0;

    char dirnoslash[strlen(fullpath) + 1];
    strcpy(dirnoslash, fullpath);

    int32_t pos = strlen(dirnoslash) - 1;
    while (dirnoslash[pos] == '/') {
        dirnoslash[pos] = '\0';
        pos--;
    }

    if (CheckFile(dirnoslash)) {
        return 1;
    } else {
        char parentpath[strlen(dirnoslash) + 2];
        strcpy(parentpath, dirnoslash);
        char *ptr = strrchr(parentpath, '/');

        if (!ptr) {
            //!Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat;
            if (stat(parentpath, &filestat) == 0)
                return 1;

            return 0;
        }

        ptr++;
        ptr[0] = '\0';

        result = CreateSubfolder(parentpath);
    }

    if (!result)
        return 0;

    if (mkdir(dirnoslash, 0777) == -1) {
        return 0;
    }

    return 1;
}

int32_t saveBufferToFile(const char *path, void *buffer, uint32_t size) {
    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY);
    if (fd < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open %s. %d", path, fd);
        return -1;
    }
    auto sizeToWrite = size;
    auto *ptr        = buffer;
    int32_t curResult;
    int64_t totalSize = 0;
    while (sizeToWrite > 0) {
        curResult = write(fd, ptr, sizeToWrite);
        if (curResult < 0) {
            close(fd);
            return curResult;
        }
        if (curResult == 0) {
            break;
        }
        ptr = (void *) (((uint32_t) ptr) + curResult);
        totalSize += curResult;
        sizeToWrite -= curResult;
    }
    close(fd);
    return totalSize;
}