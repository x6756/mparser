#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include "strmap.hpp"

MStrMap::MStrMap(int extrabytes) {
    FList = nullptr;
    FCount = 0;
    FCapacity = 0;
    FExtraLen = extrabytes;
    FRecordLen = sizeof(char*) + sizeof(int) + extrabytes;
}

void MStrMap::FillFromChain(char *strchain, void *data) {
    while(*strchain) {
        long long len = strlen(strchain);
        add_str(strchain, len, data);
        strchain += len+1;
        data = (char*)data + FExtraLen;
    }
}

void MStrMap::CreateFromChain(int extrabytes, char *strchain, void *data) {
    FExtraLen = extrabytes;
    FRecordLen = sizeof(char*) + sizeof(int) + extrabytes;
    FillFromChain( strchain, data);
    ShrinkMem();
}

MStrMap::~MStrMap() {
    Clear();
}

/*
void MStrMap::add_string(const std::string &str, void *data) {
    add_str_to_upper((char*)str.c_str(), str.size(), data);
}

void MStrMap::add_string(const char *str, void *data) {
    add_str_to_upper((char*)str, strlen(str), data);
}

void MStrMap::add_str_to_upper(char *str, int len, void *data){

    add_str(str,len,data);
}*/

void MStrMap::add_var(const std::string &str, double data) {
    add_var_to_upper((char*)str.c_str(), str.size(), data);
}

void MStrMap::add_var(const char *str, double data) {
    add_var_to_upper((char*)str, strlen(str), data);
}

void MStrMap::add_var_to_upper(char *str, int len, double data){
    for(size_t i = 0;i <= strlen(str);++i) {
        str[i] = toupper(str[i]);
    }
    add_str(str,len,&data);
}

void MStrMap::add_str(const char *str, long long len, void *data) {
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

void MStrMap::ShrinkMem() {
    set_capacity(FCount);
}

void MStrMap::Trim(int NewCount) {
    FCount = NewCount;
}

void MStrMap::TrimClear(int NewCount) {
    if(NewCount < FCount) {
        char *Rec = FList + NewCount * FRecordLen;
        for(int i = NewCount; i < FCount; i++) {
            free( *(char**)Rec);
            Rec += FRecordLen;
        }
        FCount = NewCount;
    }
}

void MStrMap::set_capacity(int new_capacity) {
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

int MStrMap::IndexOf(char *str, void **data) {
    return IndexOf( str, strlen(str), data);
}

int MStrMap::IndexOf(char *str, int len, void **data) {
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

int MStrMap::Replace(char *str, void *data) {
    return Replace(str, strlen(str), data);
}

int MStrMap::Replace(char *str, int len, void *data) {
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

char* MStrMap::GetString(int index, int *len, void **data) {
    char *Rec = FList + index * FRecordLen;
    *len =  *(int*)(Rec + sizeof(char*));
    if (data!=nullptr && FExtraLen>0) {
        *data = (Rec + sizeof(char*) + sizeof(int));
    }
    return *(char**)Rec;
}

void MStrMap::Clear() {
    TrimClear(0);
    if (FList) {
        free(FList);
        FList = nullptr;
    }
}
