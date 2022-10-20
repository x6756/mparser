#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include "strmap.hpp"

m_str_map::m_str_map(int extrabytes) {
    FList = nullptr;
    FCount = 0;
    FCapacity = 0;
    FExtraLen = extrabytes;
    FRecordLen = sizeof(char*) + sizeof(int) + extrabytes;
}

void m_str_map::fill_from_chain(char *strchain, void *data) {
    while(*strchain) {
        long long len = strlen(strchain);
        add_str(strchain, len, data);
        strchain += len+1;
        data = (char*)data + FExtraLen;
    }
}

void m_str_map::create_from_chain(int extrabytes, char *strchain, void *data) {
    FExtraLen = extrabytes;
    FRecordLen = sizeof(char*) + sizeof(int) + extrabytes;
    fill_from_chain( strchain, data);
    shrink_mem();
}

m_str_map::~m_str_map() {
    clear();
}

void m_str_map::add_var(const std::string &str, double data) {
    add_var_to_upper((char*)str.c_str(), str.size(), data);
}

void m_str_map::add_var(const char *str, double data) {
    add_var_to_upper((char*)str, strlen(str), data);
}

void m_str_map::add_var_to_upper(char *str, int len, double data){
    for(size_t i = 0;i <= strlen(str);++i) {
        str[i] = toupper(str[i]);
    }
    add_str(str,len,&data);
}

void m_str_map::add_str(const char *str, long long len, void *data) {
    char *Rec;
    if(FCount >= FCapacity) {
        int delta = (FCapacity > 64) ? FCapacity / 4 : 16;
        set_capacity( FCapacity + delta);
    }
    Rec = FList + FCount * FRecordLen;
    *(char**)Rec = (char *)malloc(len + FExtraLen + sizeof(int));
    strncpy(*(char**)Rec,str,len);
    *(int*)(Rec + sizeof(char*)) = len;
    if(data) {
        void *recdata = (Rec + sizeof(char*) + sizeof(int));
        memcpy( recdata, data, FExtraLen);
    }
    FCount++;
}

void m_str_map::shrink_mem() {
    set_capacity(FCount);
}

void m_str_map::trim(int NewCount) {
    FCount = NewCount;
}

void m_str_map::trim_clear(int NewCount) {
    if(NewCount < FCount) {
        char *Rec = FList + NewCount * FRecordLen;
        for(int i = NewCount; i < FCount; i++) {
            free( *(char**)Rec);
            Rec += FRecordLen;
        }
        FCount = NewCount;
    }
}

void m_str_map::set_capacity(int new_capacity) {
    FCapacity = new_capacity;
    if(FCount >FCapacity) {
        FCount = FCapacity;
    }

    char *tmp = (char*) realloc(FList, FCapacity*FRecordLen);
    if(tmp == nullptr) {
        free(FList);
        throw("set_capacity failed");
    }
    FList = tmp;
}

int m_str_map::index_of(char *str, void **data) {
    return index_of( str, strlen(str), data);
}

int m_str_map::index_of(char *str, int len, void **data) {
    char *Rec = FList;
    for(int i=0; i<FCount; i++) {
        int recLen = *(int*)(Rec + sizeof(char*));
        if (recLen==len && strncmp( str, *(char**)Rec, recLen)==0) {
            *data = (Rec + sizeof(char*) + sizeof(int));
            return i;
        }
        Rec += FRecordLen;
    }
    *data = nullptr;
    return -1;
}

int m_str_map::replace(char *str, void *data) {
    return replace(str, strlen(str), data);
}

int m_str_map::replace(char *str, int len, void *data) {
    char *Rec = FList;
    for (int i=0; i<FCount; i++) {
        int recLen = *(int*)(Rec + sizeof(char*));
        if (recLen==len && strncmp( str, *(char**)Rec, recLen)==0) {
            void *recdata = (Rec + sizeof(char*) + sizeof(int));
            memcpy( recdata, data, FExtraLen);
            return i;
        }
        Rec += FRecordLen;
    }
    return -1;
}

char* m_str_map::get_string(int index, int *len, void **data) {
    char *Rec = FList + index * FRecordLen;
    *len =  *(int*)(Rec + sizeof(char*));
    if (data!=nullptr && FExtraLen>0) {
        *data = (Rec + sizeof(char*) + sizeof(int));
    }
    return *(char**)Rec;
}

void m_str_map::clear() {
    trim_clear(0);
    if (FList) {
        free(FList);
        FList = nullptr;
    }
}
