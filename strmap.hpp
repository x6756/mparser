#ifndef _STRMAP_H_
#define _STRMAP_H_
#include <string>

class MStrMap {
public:
    MStrMap( int extrabytes=sizeof(double));
    void CreateFromChain( int extrabytes, char *strchain, void *data);
    ~MStrMap();
    void ShrinkMem();
    void Clear();
    void set_capacity(int new_capacity);
    int IndexOf(char *str, void **data);
    int Replace( char *str,void *data);
    char* GetString(int index, int *len, void **data);
    void FillFromChain(char *strchain, void *data);
    int IndexOf(char *str, int len, void **data);
    void add_str(const char *str, long long len, void *data);

    void add_var(const std::string &str, double data);
    void add_var(const char *str, double data);
    void add_var_to_upper(char *str, int len, double data);
    int Replace( char *str, int len, void *data);
private:
    void Trim(int NewCount);
    void TrimClear(int NewCount);
    int   FCount, FCapacity;
    int   FExtraLen, FRecordLen;
    char *FList;
};

#endif //_STRMAP_H_
