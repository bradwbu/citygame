#include "utilitieslib/UtilsNew/regfile.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LOCK_NAME "lock"

char registryPath_[1024];
char lockFile_[1024];

int regfileIsInit(void)
{
    if (strlen(lockFile_) != 0)
    {
        return 1;
    }
    //perror("ERROR: Regfile not initialized.");
    return 0;
}

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>
#include <assert.h>

#define LOCK_FLAGS (_O_EXCL | _O_CREAT | _O_RDWR | _O_TEMPORARY), (_S_IREAD | _S_IWRITE)

int tryLock_(int maxTries)
{
    if (!regfileIsInit()) return -1;

    int tries = 0;
    int lfd = open(lockFile_, LOCK_FLAGS);
    while (lfd == -1 && tries < maxTries)
    {
        perror(__FUNCTION__);
        _sleep(8);
        lfd = open(lockFile_, LOCK_FLAGS);
        tries += 1;
    }
    if (tries == maxTries)
    {
        perror(__FUNCTION__);
        perror("FATAL: Could not acquire shadow registry lockfile.");
        exit(EXIT_FAILURE);
    }
    return lfd;
}

int releaseLock_(int lfd)
{
    return close(lfd);
}

int listFiles_(const char* path, char* files)
{
    HANDLE findHandle;
    WIN32_FIND_DATA findData;
    char* iter = files;
    int fileCount = 0;

    findHandle = FindFirstFile(path, &findData);

    if (findHandle == INVALID_HANDLE_VALUE)
        return fileCount;

    strcpy(iter, findData.cFileName);
    iter += strlen(iter) + 1;
    fileCount += 1;

    while (FindNextFile(findHandle, &findHandle))
    {
        strcpy(iter, findData.cFileName);
        iter += strlen(iter) + 1;
        fileCount += 1;
    }

    FindClose(findHandle);
    return fileCount;
}

#endif

// Shameless Copypasta from SO

int mkpath_(const char* path)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, path);
    char pathBuffer[512];
    strcpy(pathBuffer, path);
    for (char* p = strchr(pathBuffer + 1, REGFILE_SEPERATOR_CHAR); p; p = strchr(p + 1, REGFILE_SEPERATOR_CHAR))
    {
        *p = '\0';
        if (mkdir(pathBuffer) == -1)
        {
            if (errno != EEXIST)
            {
                perror(__FUNCTION__);
                perror(strerror(errno));
                *p = REGFILE_SEPERATOR_CHAR;
                return -1;
            }
        }
        *p = REGFILE_SEPERATOR_CHAR;
    }
    printf("FN: [%s] Done.\n", __FUNCTION__);
    return 0;
}

int regfileInit(const char* directory)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, directory);
    strcpy(registryPath_, directory);
    regfileNormalizeKey(registryPath_);
    if (directory[strlen(directory) - 1] != REGFILE_SEPERATOR_CHAR)
        strcat(registryPath_, REGFILE_SEPERATOR_STR);

    int status = mkpath_(registryPath_);
    if (status != 0)
    {
        perror(__FUNCTION__);
        perror("Failed to initialize regfile");
        return -1;
    }

    strcpy(lockFile_, registryPath_);
    strcat(lockFile_, LOCK_NAME);

    printf("FN: [%s] Done.\n", __FUNCTION__);
    return 0;
}

void regfileNormalizeKey(char* mutableKey)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, mutableKey);
    char* iter = mutableKey;
    while (*iter != 0)
    {
        *iter = tolower(*iter);
        #ifdef _WIN32
        if (*iter == '/')
            *iter = '\\';
        #else
        if (*iter == '\\')
            *iter = '/';
        #endif
        ++iter;
    }
    printf("FN: [%s] Done.\n", __FUNCTION__);
}

size_t regfileLoadKeyValue(const char* key, void* buffer, size_t len)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, key);
    int lfd = tryLock_(50);

    char* keyPath[1024];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);
    mkpath_(keyPath);

    FILE* keyfile = fopen(keyPath, "wb+");

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(__FUNCTION__);
        perror(strerror(errcode));
        releaseLock_(lfd);
        return 0;
    }

    size_t bytesRead = fread(buffer, 1, len, keyfile);

    fclose(keyfile);
    releaseLock_(lfd);

    printf("FN: [%s] Done.\n", __FUNCTION__);
    return bytesRead;
}

int regfileStoreKeyValue(const char* key, void* value, size_t len)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, key);
    int lfd = tryLock_(50);

    char* keyPath[1024];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);
    mkpath_(keyPath);

    FILE* keyfile = fopen(keyPath, "wb+");

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(__FUNCTION__);
        perror(strerror(errcode));
        releaseLock_(lfd);
        return 0;
    }

    size_t bytesWritten = fwrite(value, 1, len, keyfile);

    if (bytesWritten != len)
    {
        perror(__FUNCTION__);
        perror("Error writing value to key file.");
    }

    fclose(keyfile);
    releaseLock_(lfd);

    printf("FN: [%s] Done.\n", __FUNCTION__);
    return bytesWritten;
}

int regfileRemoveKey(const char* key)
{
    printf("FN: [%s] ARG: [%s]\n", __FUNCTION__, key);
    char* keyPath[1024];
    strcpy(keyPath, registryPath_);
    strcat(keyPath, key);
    regfileNormalizeKey(keyPath);
    remove(keyPath);
    printf("FN: [%s] Done.\n", __FUNCTION__);
}

int regfileList(const char* key, char* files)
{
    return listFiles_(key, files);
}