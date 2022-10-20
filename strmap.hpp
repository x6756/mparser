#ifndef _STRMAP_H_
#define _STRMAP_H_
#include <string>

class m_str_map {
public:
    m_str_map( int extrabytes=sizeof(double));
    void create_from_chain( int extrabytes, char *strchain, void *data);
    ~m_str_map();
    void shrink_mem();
    void clear();
    void set_capacity(int new_capacity);
    int index_of(char *str, void **data);
    int replace( char *str,void *data);
    char* get_string(int index, int *len, void **data);
    void fill_from_chain(char *strchain, void *data);
    int index_of(char *str, int len, void **data);
    void add_str(const char *str, long long len, void *data);

    void add_var(const std::string &str, double data);
    void add_var(const char *str, double data);
    void add_var_to_upper(char *str, int len, double data);
    int replace( char *str, int len, void *data);
private:
    void trim(int NewCount);
    void trim_clear(int NewCount);
    int   FCount, FCapacity;
    int   FExtraLen, FRecordLen;
    char *FList;
};

#endif //_STRMAP_H_
