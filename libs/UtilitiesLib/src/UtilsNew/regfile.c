#include "regfile.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* g_reg_path = NULL;
char* g_lock_file = NULL;

int regfile_is_init(void)
{
    if (g_reg_path != NULL && g_lock_file != NULL)
    {
        return 1;
    }
    perror("ERROR: Regfile not initialized.");
    return 0;
}

#ifdef _WIN32
#define LOCK_NAME "\\lock"
#else
#define LOCK_NAME "/lock"
#endif

#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>

#define LOCK_FLAGS (_O_EXCL | _O_CREAT | _O_RDWR | _O_TEMPORARY), (_S_IREAD | _S_IWRITE)

int try_lock(int max_tries)
{
    if (!regfile_is_init()) return -1;

    int tries = 0;
    int lfd = _open(g_lock_file, LOCK_FLAGS);
    while (lfd == -1 && tries < max_tries)
    {
        _sleep(8);
        lfd = _open(g_lock_file, LOCK_FLAGS);
        tries += 1;
    }
    if (tries == max_tries)
    {
        perror("FATAL: Could not acquire shadow registry lockfile.");
        exit(EXIT_FAILURE);
    }
    return lfd;
}

int release_lock(int lfd)
{
    return _close(lfd);
}

#endif

int regfile_init(const char* directory)
{
    g_reg_path = calloc(strlen(directory)+1, 1);
    strcpy(g_reg_path, directory);

    g_lock_file = calloc(strlen(directory) + strlen(LOCK_NAME) + 1, 1);
    strcpy(g_lock_file, g_reg_path);
    strcat(g_lock_file, LOCK_NAME);

    return 0;
}

void regfile_deinit()
{
    if (!regfile_is_init())
        return;
    free(g_reg_path);
    free(g_lock_file);
}

void regfile_normalize_key(char* mutable_key)
{
    char* iter = mutable_key;
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
    }
}

void* regfile_load_key_value(const char* key, size_t len)
{
    int lfd = try_lock(50);

    char* key_path = calloc(strlen(key) + strlen(g_reg_path) + 1, 1);
    strcpy(key_path, g_reg_path);
    strcat(key_path, key);

    FILE* keyfile = fopen(key_path, "rb");

    free(key_path);

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(strerror(errcode));
        release_lock(lfd);
        fclose(keyfile);
        return NULL;
    }

    void* buffer = calloc(len, 1);
    if (buffer == NULL)
    {
        int errcode = errno;
        perror(strerror(errcode));
        release_lock(lfd);
        fclose(keyfile);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, len, keyfile);

    fclose(keyfile);
    release_lock(lfd);

    if (bytes_read != len)
    {
        perror("Error reding key file.");
        free(buffer);
        return NULL;
    }

    return buffer;
}

int regfile_store_key_value(const char* key, void* value, size_t len)
{
    int lfd = try_lock(50);

    char* key_path = calloc(strlen(key) + strlen(g_reg_path) + 1, 1);
    strcpy(key_path, g_reg_path);
    strcat(key_path, key);

    FILE* keyfile = fopen(key_path, "wb");

    free(key_path);

    if (keyfile == NULL)
    {
        int errcode = errno;
        perror(strerror(errcode));
        release_lock(lfd);
        fclose(keyfile);
        return 0;
    }

    size_t bytes_written = fwrite(value, 1, len, keyfile);
    if (bytes_written != len)
    {
        perror("Error writing value to key file.");
    }

    fclose(keyfile);
    release_lock(lfd);
    return bytes_written;
}
