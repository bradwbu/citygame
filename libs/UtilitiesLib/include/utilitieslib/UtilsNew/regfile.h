#ifndef REGFILE_H
#define REGFILE_H

#include <stdint.h>

#ifdef _cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#define REGFILE_DEFAULT_PATH ".\\registry_keys\\"
#define REGFILE_SEPERATOR_CHAR '\\'
#define REGFILE_SEPERATOR_STR "\\"
#else
#define REGFILE_DEFAULT_PATH "./registry_keys/"
#define REGFILE_SEPERATOR_CHAR '/'
#define REGFILE_R_STR "/"
#endif


    int regfileIsInit(void);
    int regfileInit(const char* filepath);
    size_t regfileLoadKeyValue(const char* key, void* buffer, size_t len);
    int regfileStoreKeyValue(const char* key, void* value, size_t len);
    void regfileNormalizeKey(char* mutable_key);
    int regfileRemoveKey(const char* key);
    int regfileList(const char* key, char* files);

#ifdef _cplusplus
}
#endif

#endif // !REGFILE_H
