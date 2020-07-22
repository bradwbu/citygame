#ifndef REGFILE_H
#define REGFILE_H

#ifdef _cplusplus
extern "C"
{
#endif

    int regfile_is_init(void);
    int regfile_init(const char* filepath);
    void* regfile_load_key_value(const char* key, size_t len);
    int regfile_store_key_value(const char* key, void* value, size_t len);
    void regfile_normalize_key(char* mutable_key);

#ifdef _cplusplus
}
#endif

#endif // !REGFILE_H
